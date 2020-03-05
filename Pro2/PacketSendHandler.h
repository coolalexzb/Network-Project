//
// Created by 郑博 on 3/1/20.
//
#ifndef PRO2_PACKETSENDHANDLER_H
#define PRO2_PACKETSENDHANDLER_H

#include <sys/time.h>
#include <vector>
#include <cstring>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <iostream>
#include "helper.h"

using namespace std;

//const int WINDOW_SIZE = 10;
//const int PACKET_HEADER_LENGTH = 10;
//const int PACKET_DATA_LENGTH = 10;
//const int PACKET_TIMEOUT_TIME = 10;

struct packet {
    char* data;
    short len;
    short seq;
    time_t time;
    bool isAck;
};

typedef struct packet *packetPtr;

class PacketSendHandler {
public:
    
	PacketSendHandler(char* filePath);
	~PacketSendHandler();

	packetPtr newPacket();
    packetPtr getUnAckPacket(time_t curTime);
    void recv_ack(short ackSeq);
	bool isWindowFull();
	bool isOver();

private:

	packet *slideWindow;					// sliding window
	int seqSize;							// sliding window sequence size
    
	bool startSending;						// whether start sending
	bool finishSending;						// whether finish sending
	short seqFirst;							// The first seq Number of packet not sent successfully
    short seqNext;							// Next packet to be sent

    int file;
	char* filePath;
    long fileLen;
    long sendingPos;

    void init();
	bool isInWindow(short ackSeq);
	void updateSeqInfo(short ackSeq);
};

#endif //PRO2_PACKETSENDHANDLER_H