
#include "PacketSendHandler.h"

void PacketSendHandler::init() {
	// init slideWindow
	slideWindow = new packet[WINDOW_SIZE];
	for (int i = 0; i < WINDOW_SIZE; i++) {
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
	return (seqFirst <= ackSeq) && (ackSeq < seqNext);
}

void PacketSendHandler::updateSeqInfo(short ackSeq) {
	while (seqFirst <= ackSeq) {
		short index = seqFirst++ % WINDOW_SIZE;
		slideWindow[index].isAck = true;
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

	for (int i = 0; i < WINDOW_SIZE; i++)
		delete slideWindow[i].data;
	delete[] slideWindow;
}

packetPtr PacketSendHandler::newPacket()
{
	packetPtr thisPacket = &slideWindow[seqNext % WINDOW_SIZE];
	*(short *)(thisPacket->data + PACKET_HEADER_POS) = (short)htons(seqNext);

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

	short checksum = generateCkSum(thisPacket->data, thisPacket->len);
	*(short *)(thisPacket->data + PACKET_CHECKSUM_POS) = (short)htons(checksum);

	thisPacket->isAck = false;
	thisPacket->seq = seqNext++;

	return thisPacket;
}

packetPtr PacketSendHandler::getUnAckPacket(time_t curTime) {	
	short seqFirst_copy = seqFirst;
	while (seqFirst_copy < seqNext) {
		packetPtr tmp = &slideWindow[seqFirst_copy++ % WINDOW_SIZE];
		if (!tmp->isAck && (curTime - tmp->time) > PACKET_TIMEOUT_TIME) return tmp;
	}
	
	return nullptr;
}

void PacketSendHandler::recv_ack(short ackSeq) {
	if (isInWindow(ackSeq)) {
		packetPtr thisPacket = &slideWindow[ackSeq % WINDOW_SIZE];

		// duplication check
		if (thisPacket->isAck) {
			printf("[recv ack] packect#%hd DUPLICATED\n", ackSeq);
		}
		else {
			if (ackSeq == 0) {
				printf("[recv ack] packect#%05hd ACCEPTED\tHeader packet RECEIVED\n", ackSeq);
			}
			else {
				long sendingOffset = ackSeq * PACKET_DATA_LENGTH;
				if (sendingOffset > fileLen) sendingOffset = fileLen;
				printf("[recv ack] packect#%05hd ACCEPTED\t%07ld bytes RECEIVED\n", ackSeq, sendingOffset);
			}
			updateSeqInfo(ackSeq);
		}
	}
	else {
		printf("[recv ack] packect#%hd IGNORED\n", ackSeq);
	}
}

bool PacketSendHandler::isWindowFull() {
	return seqNext - seqFirst == WINDOW_SIZE;
}

bool PacketSendHandler::isOver() {
	return finishSending;
}