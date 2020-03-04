//
// Created by 郑博 on 3/1/20.
//
#include "PacketSendHandler.h"

void PacketSendHandler::init() {
	// init slideWindow
	seqSize = WINDOW_SIZE * 2;
	slideWindow = new packet[seqSize];
	for (int i = 0; i < seqSize; i++) {
		slideWindow[i].isAck = false;
		slideWindow[i].data = new char[PACKET_HEADER_LENGTH + sizeof(short) + PACKET_DATA_LENGTH];
		slideWindow[i].len = 0;
	}

	// init variables
	seqFirst = 0;
	seqNext = 0;
	startSending = false;
	finishSending = false;

	// init file
	fin.seekg(0, std::ios::end);
	fileLen= fin.tellg();
	fin.seekg(0, std::ios::beg);
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
	fin = ifstream(filePath, std::ios::binary | std::ios::in);
	
	size_t length = strlen(filePath);
	this->filePath = new char[length];
	memcpy(this->filePath, filePath, length);
	
	init();
}

PacketSendHandler::~PacketSendHandler() {
	fin.close();
	
	for (int i = 0; i < seqSize; i++)
		delete slideWindow[i].data;
	delete[] slideWindow;
}

packetPtr PacketSendHandler::newPacket()
{
	packetPtr thisPacket = &slideWindow[seqNext];

	// for first packet, send filePath and fileLen
	if (!startSending) {
		startSending = true;

		short packetNum = fileLen / PACKET_DATA_LENGTH;
		if (fileLen % PACKET_DATA_LENGTH != 0) packetNum++;
		*(short *)(thisPacket->data + PACKET_HEADER_LENGTH) = (short)htons(packetNum);

		short filePathLen = strlen(filePath);
		*(short *)(thisPacket->data + PACKET_HEADER_LENGTH + sizeof(short)) = (short)htons(filePathLen); 
		memcpy(thisPacket->data + PACKET_HEADER_LENGTH + sizeof(short) * 2, filePath, filePathLen);

		thisPacket->len = PACKET_HEADER_LENGTH + sizeof(short) * 2 + filePathLen;
	}
	else {
		long lenToSend = fileLen - sendingPos;

		// the last packet
		if (lenToSend <= PACKET_DATA_LENGTH) {

			*(short *)(thisPacket->data + PACKET_HEADER_LENGTH) = (short)htons(lenToSend);
			fin.read(thisPacket->data + PACKET_HEADER_LENGTH + sizeof(short), lenToSend);
			thisPacket->len = PACKET_HEADER_LENGTH + sizeof(short) + lenToSend;

			sendingPos = fileLen;
			finishSending = true;
		}
		else {
			*(short *)(thisPacket->data + PACKET_HEADER_LENGTH) = (short)htons(PACKET_DATA_LENGTH);
			fin.read(thisPacket->data + PACKET_HEADER_LENGTH + sizeof(short), PACKET_DATA_LENGTH);
			thisPacket->len = PACKET_HEADER_LENGTH + sizeof(short) + PACKET_DATA_LENGTH;

			sendingPos += PACKET_DATA_LENGTH;
		}
	}

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