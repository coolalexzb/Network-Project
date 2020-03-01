//
// Created by 郑博 on 3/1/20.
//

#ifndef PRO2_PACKETRECVHANDLER_H
#define PRO2_PACKETRECVHANDLER_H

#include <iostream>
#include <sys/time.h>
#include <vector>
using namespace std;

struct packet {
    char *data;
    int seq;
};

class PacketRecvHandler {
public:
    PacketRecvHandler();
    bool isOver();
    int getNextSeq();
    int recvPacket(char * packet, int length);
    ~PacketRecvHandler();

private:
    packet *slideWindow;                    // sliding window
    int seqOldest;                          // The oldest seq Number of packet not sent successfully
    int seqNext;                            // Next packet to be sent

    char* file;
    void init(char* filePath);
    void wrote();

};

#endif //PRO2_PACKETRECVHANDLER_H
