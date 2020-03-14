
#include "PacketRecvHandler.h"

PacketRecvHandler::PacketRecvHandler() {
    init();
}

bool PacketRecvHandler::isOver() {
    return packetSize == packetCnt;
}

short PacketRecvHandler::getNextSeq() {

}

short PacketRecvHandler::recvPacket(char * packet, int length, bool isHeader) {
    if(isHeader) {
        packetSize = (short) ntohs(*(short *)(packet + PACKET_PACKETNUM_POS));
        slideWindow[0].isRecv = true;
        filePath = packet + 4 + sizeof(short) * 2;
        file = open(filePath, O_RDWR | O_CREAT, 0777);
        if (file < 0) {
            printf("File open failed!\n");
        }
        return 0;
    }else {
        short seqNum = (short) ntohs(*(short *)(packet + PACKET_HEADER_POS));
        printf("seqNum: %d\n", seqNum);
        if(slideWindow[seqNum].isRecv && seqNum > seqOldest && seqNum < seqNext) {
            short retSeqNum = updateSeq(seqNum);
            printf("retSeqNum1: %d\n", retSeqNum);
            return retSeqNum;
        }else {
            short dateLen = (short) ntohs(*(short *)(packet + PACKET_DATALEN_POS));
            memcpy(slideWindow[seqNum].data, packet + PACKET_DATA_POS, dateLen);
            slideWindow[seqNum].isRecv = true;
            slideWindow[seqNum].seq = seqNum;
            packetCnt++;
            wrote(slideWindow[seqNum]);
            short retSeqNum = updateSeq(seqNum);
            printf("retSeqNum2: %d\n", retSeqNum);
            seqNext = seqNum >= seqNext ? seqNum + 1: seqNext;
            return retSeqNum;
        }
    }
}

short PacketRecvHandler::updateSeq(short seqNum) {
    for(short i = seqOldest; i <= seqNext; i++) {
        if(slideWindow[i].isRecv) {
            seqOldest += 1;
            slideWindow[i].isRecv = false;
        }else {
            break;
        }
    }

    return seqOldest;
}

void PacketRecvHandler::init() {
    filePath = (char*)"./output";
    slideWindow = new Packet[WINDOW_SIZE];
    for(int i = 0; i < WINDOW_SIZE; i++) {
        slideWindow[i].data = new char[PACKET_DATA_LENGTH + 1];
        slideWindow[i].isRecv = false;
    }
    packetCnt = 0;
	seqOldest = 0;
    seqNext = 1;
}

int PacketRecvHandler::getPacketSize(){
    return packetSize;
}

void PacketRecvHandler::wrote(Packet packet) {
    lseek(file, (packetCnt - 1) * PACKET_DATA_LENGTH, SEEK_SET);
    printf("data: %s\n", packet.data);
    if(isOver()) {
        write(file, packet.data, sizeof(packet.data));
    }else {
        write(file, packet.data, sizeof(packet.data));
    }

}

PacketRecvHandler::~PacketRecvHandler() {
    close(file);
}