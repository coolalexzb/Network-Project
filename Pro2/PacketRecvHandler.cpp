
#include "PacketRecvHandler.h"

PacketRecvHandler::PacketRecvHandler() {
    init();
}

bool PacketRecvHandler::isOver() {
    return packetSize == packetCnt;
}

short PacketRecvHandler::getNextSeq() {

}

short PacketRecvHandler::recvPacket(char * packet, int length) {
    if (!startSending) {
		
		// header
        packetSize = (short)ntohs(*(short *)(packet + PACKET_PACKETNUM_POS));
        slideWindow[0].isRecv = true;
        
		filePath = packet + PACKET_FILEPATH_POS;
        file = open(filePath, O_RDWR | O_CREAT, 0777);
        if (file < 0) {
            printf("File open failed!\n");
        }
        
		startSending = true;
		return 0;
    }
	else {
		short seqNum = (short)ntohs(*(short *)(packet + PACKET_HEADER_POS));
        printf("seqNum: %d\n", seqNum);
        printf("seqOldest: %d\n", seqOldest);
        printf("seqNext: %d\n", seqNext);
        if (slideWindow[seqNum % WINDOW_SIZE].isRecv) {
            //printf("enter into seqNum: %d\n", seqNum);
            return updateSeq(seqNum);
        }
		else {
            short dateLen = (short)ntohs(*(short *)(packet + PACKET_DATALEN_POS));
            memcpy(slideWindow[seqNum % WINDOW_SIZE].data, packet + PACKET_DATA_POS, dateLen);
            slideWindow[seqNum % WINDOW_SIZE].isRecv = true;
            slideWindow[seqNum % WINDOW_SIZE].seq = seqNum;
            packetCnt++;
            
			wrote(slideWindow[seqNum % WINDOW_SIZE]);
			if (seqNum >= seqNext) seqNext = seqNum + 1;
            return updateSeq(seqNum);
        }
    }
}

short PacketRecvHandler::updateSeq(short seqNum) {
    for (short i = seqOldest; i <= seqNext; i++) {
		short index = i % WINDOW_SIZE;
        if (slideWindow[index].isRecv) {
            if(seqOldest < seqNext) {
                seqOldest++;
            }
            slideWindow[index].isRecv = false;
			//memset(slideWindow[seqNum % WINDOW_SIZE].data, 0, PACKET_DATA_LENGTH);
            memset(slideWindow[index % WINDOW_SIZE].data, 0, PACKET_DATA_LENGTH);
        }
		else {
            break;
        }
    }

    return seqOldest - 1;
}

void PacketRecvHandler::init() {
    filePath = (char *)"./output";
    slideWindow = new Packet[WINDOW_SIZE];
    for (int i = 0; i < WINDOW_SIZE; i++) {
        slideWindow[i].data = new char[PACKET_DATA_LENGTH];
        slideWindow[i].isRecv = false;
    }
    
	packetCnt = 0;
	seqOldest = 0;
    seqNext = 1;

	startSending = false;
}

int PacketRecvHandler::getPacketSize(){
    return packetSize;
}

void PacketRecvHandler::wrote(Packet packet) {
    lseek(file, (packetCnt - 1) * PACKET_DATA_LENGTH, SEEK_SET);
    //printf("data: %s\n", packet.data);
    if (isOver()) {
        write(file, packet.data, sizeof(packet.data));
    }
	else {
        write(file, packet.data, sizeof(packet.data));
    }

}

PacketRecvHandler::~PacketRecvHandler() {
    close(file);
}