//
// Created by 郑博 on 3/1/20.
//

#include "PacketRecvHandler.h"

PacketRecvHandler::PacketRecvHandler(char* filePath) {
    init(filePath);
}

bool PacketRecvHandler::isOver() {
    return packetSize == packetCnt;
}

short PacketRecvHandler::getNextSeq() {

}

short PacketRecvHandler::recvPacket(char * packet, int length, bool isHeader) {
    if(isHeader) {
        packetSize = (short) ntohs(*(short *)(packet + 4));
        return -1;
    }else {
        short seqNum = (short) ntohs(*(short *)(packet));
        if(slideWindow[seqNum].isRecv) {
            short retSeqNum = updateSeq(seqNum);
            return retSeqNum;
        }else {
            short dateLen = (short) ntohs(*(short *)(packet + 4));
            memcpy(slideWindow[seqNum].data, packet + DATA_PACKET_CONTENT, (int) (length - dateLen));
            slideWindow[seqNum].isRecv = true;
            slideWindow[seqNum].seq = seqNum;
            wrote(packet);
            packetCnt++;
            short retSeqNum = updateSeq(seqNum);
            return retSeqNum;
        }
    }
}

short PacketRecvHandler::updateSeq(short seqNum) {
    for(short i = seqOldest + 1; i <= seqNum; i++) {
        if(slideWindow[i].isRecv) {
            seqOldest += 1;
        }else {
            break;
        }
    }
    return seqOldest;
}

void PacketRecvHandler::init(char* filePath) {
    file = open(filePath, O_RDWR | O_CREAT, 0777);
    if (file < 0) {
        printf("File open failed!\n");
        return;
    }
    slideWindow = new packet[WINDOW_SIZE];
    for(int i = 0; i < WINDOW_SIZE; i++) {
        slideWindow[i].data = new char[DATA_PACKET_DATA_LENGTH + 1];
        slideWindow[i].isRecv = false;
    }
    packetCnt = 0;
}

int PacketRecvHandler::getPacketSize(){
    return packetSize;
}

void PacketRecvHandler:: wrote(char* packet) {
    lseek(file, (packetCnt - 1)* DATA_PACKET_DATA_LENGTH, SEEK_SET);
    if(isOver()) {
        write(file, packet, sizeof(packet));
    }else {
        write(file, packet, DATA_PACKET_DATA_LENGTH);
    }

}

PacketRecvHandler::~PacketRecvHandler() {
    close(file);
}