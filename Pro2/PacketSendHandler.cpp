
#include "PacketSendHandler.h"

void PacketSendHandler::init() {
	// init slideWindow
	seqSize = WINDOW_SIZE * 2;
	slideWindow = new packet[seqSize];
	for (int i = 0; i < seqSize; i++) {
		slideWindow[i].isAck = false;
		slideWindow[i].data = new char[PACKET_DATA_POS + PACKET_DATA_LENGTH];
		slideWindow[i].len = 0;
	}

	// init variables
	seqFirst = 0;
	seqNext = 0;
	startSending = false;
	finishSending = false;

	// init file
	fileLen = lseek(file, sendingPos, SEEK_END);
	sendingPos = 0;
}

bool PacketSendHandler::isInWindow(short ackSeq) {
	if (seqNext >= seqFirst) {
		return (seqFirst <= ackSeq) && (ackSeq < seqNext);
	}
	else {
		return ((0 <= ackSeq) && (ackSeq < seqNext)) || ((seqFirst <= ackSeq) && (ackSeq < seqSize));
	}
}

void PacketSendHandler::updateSeqInfo(short ackSeq) {
	while (seqFirst != (ackSeq+1) % seqSize) {
		slideWindow[seqFirst++].isAck = true;
		seqFirst %= seqSize;
	}
}

PacketSendHandler::PacketSendHandler(char* filePath) {

    file = open(filePath, O_FSYNC | O_RDWR | O_CREAT, 0777);
    if (file < 0) {
        printf("File open failed!\n");
        return;
    }
	
	size_t length = strlen(filePath);
	this->filePath = new char[length];
	memcpy(this->filePath, filePath, length);
	
	init();
}

PacketSendHandler::~PacketSendHandler() {
    close(file);

	for (int i = 0; i < seqSize; i++)
		delete slideWindow[i].data;
	delete[] slideWindow;
}

packetPtr PacketSendHandler::newPacket()
{
	packetPtr thisPacket = &slideWindow[seqNext];

	if (!startSending) {

		// for first packet, send filePath and fileLen
		short packetNum = fileLen / PACKET_DATA_LENGTH;
		if (fileLen % PACKET_DATA_LENGTH != 0) packetNum++;
		*(short *)(thisPacket->data + PACKET_PACKETNUM_POS) = (short)htons(packetNum);
		
		short filePathLen = strlen(filePath);
		*(short *)(thisPacket->data + PACKET_FILEPATHLEN_POS) = (short)htons(filePathLen);
		memcpy(thisPacket->data + PACKET_FILEPATH_POS, filePath, filePathLen);
		thisPacket->len = PACKET_FILEPATH_POS + filePathLen;

		startSending = true;
	}
	else {
		long lenToSend = fileLen - sendingPos;

		if (lenToSend <= PACKET_DATA_LENGTH) {
			
			// the last packet
			*(short *)(thisPacket->data + PACKET_DATALEN_POS) = (short)htons(lenToSend);
			lseek(file, sendingPos, SEEK_SET);
			read(file, thisPacket->data + PACKET_DATA_POS, lenToSend);
			thisPacket->len = PACKET_DATA_POS + lenToSend;

			sendingPos = fileLen;
			finishSending = true;
		}
		else {

			// common packet
			*(short *)(thisPacket->data + PACKET_DATALEN_POS) = (short)htons(PACKET_DATA_LENGTH);
			lseek(file, sendingPos, SEEK_SET);
			read(file, thisPacket->data + PACKET_DATA_POS, PACKET_DATA_LENGTH);
			thisPacket->len = PACKET_DATA_POS + PACKET_DATA_LENGTH;

			sendingPos += PACKET_DATA_LENGTH;
		}
	}

	short checksum = 128;
	*(short *)(thisPacket->data + PACKET_HEADER_POS) = (short)htons(seqNext);
	*(short *)(thisPacket->data + PACKET_CHECKSUM_POS) = (short)htons(checksum);

	thisPacket->isAck = false;
	thisPacket->seq = seqNext++;
	seqNext %= seqSize;

	return thisPacket;
}

packetPtr PacketSendHandler::getUnAckPacket(time_t curTime) {
	packetPtr tmp;
	
	short seqFirst_copy = seqFirst;
	while (seqFirst_copy != seqNext) {
		tmp = &slideWindow[seqFirst_copy++];
		if (!tmp->isAck && (curTime - tmp->time) > PACKET_TIMEOUT_TIME) return tmp;
		seqFirst_copy %= seqSize;
	}
	
	return nullptr;
}

void PacketSendHandler::recv_ack(short ackSeq) {
	if (isInWindow(ackSeq)) {
		packetPtr thisPacket = &slideWindow[ackSeq];

		// duplication check
		if (thisPacket->isAck) {
			printf("[recv ack] DUPLICATED\n");
		}
		else {
			printf("[recv ack] ACCEPTED\n");
			updateSeqInfo(ackSeq);
		}
	}
	else {
		printf("[recv ack] IGNORED\n");
	}
}

bool PacketSendHandler::isWindowFull() {
	return abs(seqNext - seqFirst) == seqSize;
}

bool PacketSendHandler::isOver() {
	return finishSending;
}