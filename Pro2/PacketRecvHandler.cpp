
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
        packetSize = (short) ntohs(*(short *)(packet + PACKET_PACKETNUM_POS));
        return -1;
    }else {
        short seqNum = (short) ntohs(*(short *)(packet + PACKET_HEADER_POS));
        if(slideWindow[seqNum].isRecv) {
            short retSeqNum = updateSeq(seqNum);
            return retSeqNum;
        }else {
            short dateLen = (short) ntohs(*(short *)(packet + PACKET_DATALEN_POS));
            memcpy(slideWindow[seqNum].data, packet + PACKET_DATA_POS, (int) (length - dateLen));
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
    for(short i = seqOldest; i <= seqNum; i++) {
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
        slideWindow[i].data = new char[PACKET_DATA_LENGTH + 1];
        slideWindow[i].isRecv = false;
    }
    packetCnt = 0;
	seqOldest = 0;
}

int PacketRecvHandler::getPacketSize(){
    return packetSize;
}

void PacketRecvHandler::wrote(char* packet) {
    lseek(file, (packetCnt - 1)* PACKET_DATA_LENGTH, SEEK_SET);
    if(isOver()) {
        write(file, packet, sizeof(packet));
    }else {
        write(file, packet, PACKET_DATA_LENGTH);
    }

}

PacketRecvHandler::~PacketRecvHandler() {
    close(file);
}