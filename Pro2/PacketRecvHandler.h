//
// Created by 郑博 on 3/1/20.
//

#ifndef PRO2_PACKETRECVHANDLER_H
#define PRO2_PACKETRECVHANDLER_H

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>


using namespace std;

int WINDOW_SIZE = 64;
int DATA_PACKET_DATA_LENGTH = 60000;
int DATA_PACKET_CONTENT = 6;

struct packet {
    char *data;
    int seq;
    bool isRecv;
};

class PacketRecvHandler {
public:
    PacketRecvHandler(char* filePath = "./test");
    bool isOver();
    short getNextSeq();
    short recvPacket(char * packet, int length, bool isHeader);
    short updateSeq(short seqNum);
    int getPacketSize();
    ~PacketRecvHandler();

private:
    packet *slideWindow;                    // sliding window
    short seqOldest;                          // The oldest seq Number of packet not sent successfully
    short seqNext;                            // Next packet to be sent
    short packetSize;
    short packetCnt;

    int file;
    void init(char* filePath);
    void wrote(char* packet);

};

#endif //PRO2_PACKETRECVHANDLER_H
