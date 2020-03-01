//
// Created by 郑博 on 3/1/20.
//
#include <iostream>
#include <sys/time.h>
#include <vector>
using namespace std;

#ifndef PRO2_PACKETSENDHANDLER_H
#define PRO2_PACKETSENDHANDLER_H

struct packet {
    char* data;
    int len;
    int seq;
    time_t time;
    bool isAck;
};

class PacketSendHandler {
public:
    PacketSendHandler();
    packet* mvSlideWindow();
    bool isOver();
    vector<packet *> getNAckPacket();
    bool isSlideWindowLeft();
    void updateSeq(int seqNum);
    void recv_ack(int ack);
    ~PacketSendHandler();

private:
    packet *slideWindow;                    // sliding window
    vector<packet *>  packetRe;             // packet not ack
    int seqOldest;                          // The oldest seq Number of packet not sent successfully
    int seqNext;                            // Next packet to be sent

    char* file;
    long fileLen;
    long filePos;

    void init(char* filePath);

};
#endif //PRO2_PACKETSENDHANDLER_H
