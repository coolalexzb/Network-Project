
#include "PacketRecvHandler.h"

PacketRecvHandler::PacketRecvHandler() {
    init();
}

bool PacketRecvHandler::isOver() {
    return packetSize == packetCnt;
}

void PacketRecvHandler::mkDir(char *dir) {
    for (char *pos = dir; *pos; pos++) {
        if (*pos == '/') {
            *pos = '\0';
            mkdir(dir, S_IRWXU);
            *pos = '/';
        }
    }
    mkdir(dir, S_IRWXU);
}

short PacketRecvHandler::recvPacket(char * packet, int length) {
    if (!startSending) {
		char* sendingPath = packet + PACKET_FILEPATH_POS;
		int sendingPathLen = strlen(sendingPath);
		
		char* filePath = (char *)malloc(sendingPathLen + 6);
		strcpy(filePath, sendingPath);
		strcpy(filePath + sendingPathLen, ".recv");

        char *last = strrchr(filePath, '/');
		int dirLen = strlen(filePath) - strlen(last);

        char* subdir;
        subdir = (char *)malloc(dirLen + 1);
        strncpy(subdir, filePath, dirLen);
        mkDir(subdir);

		// header
        packetSize = (short)ntohs(*(short *)(packet + PACKET_PACKETNUM_POS));
        is_write = new bool[packetSize + 1];
        slideWindow[0].isRecv = true;
        
        file = open(filePath, O_WRONLY | O_CREAT, 0777);
        printf("filePath: %s\n", filePath);
        if (file < 0) {
            printf("File open failed!\n");
        }
        
		startSending = true;
		printf("[recv data] packect#%04hd  Header  ACCEPTED(In-ord)\n", 0);
		return 0;
    }
	else {
		short seqNum = (short)ntohs(*(short *)(packet + PACKET_HEADER_POS));
		if (seqNum == 0) {
			printf("[recv data] packect#%04hd  Header  IGNORED\n", seqNum);
			return 0;
		}

		short dataLen = (short)ntohs(*(short *)(packet + PACKET_DATALEN_POS));
		if (seqNum < seqOldest) {
			printf("[recv data] packect#%04hd  Start:%08ld bytes  Length:%hd  ACCEPTED(Out-ord)\n", seqNum, (seqNum - 1) * PACKET_DATA_LENGTH, dataLen);
		    return seqOldest - 1;
		}
        if (slideWindow[seqNum % WINDOW_SIZE].isRecv) {
			printf("[recv data] packect#%04hd  Start:%08ld bytes  Length:%hd  IGNORED\n", seqNum, (seqNum - 1) * PACKET_DATA_LENGTH, dataLen);
			return seqOldest - 1;
        }
		
		short index = seqNum % WINDOW_SIZE;
		memcpy(slideWindow[index].data, packet + PACKET_DATA_POS, dataLen);
		slideWindow[index].isRecv = true;
		slideWindow[index].seq = seqNum;
            
		wrote(slideWindow[index], seqNum);
		if (seqNum >= seqNext) seqNext = seqNum + 1;
		printf("[recv data] packect#%04hd  Start:%08ld bytes  Length:%hd  ACCEPTED(In-ord)\n", seqNum, (seqNum - 1) * PACKET_DATA_LENGTH, dataLen);
		return updateSeq(seqNum);
    }
}

short PacketRecvHandler::updateSeq(short seqNum) {
    for (short i = seqOldest; i <= seqNext; i++) {
		short index = i % WINDOW_SIZE;
        if (slideWindow[index].isRecv) {
            seqOldest++;
            slideWindow[index].isRecv = false;
            memset(slideWindow[index].data, 0, PACKET_DATA_LENGTH);
			continue;
		}
		break;
    }
    return seqOldest - 1;
}

void PacketRecvHandler::init() {
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

void PacketRecvHandler::wrote(Packet packet, short seqNum) {
    if (is_write[seqNum - 1]) return;

    lseek(file, (seqNum - 1) * PACKET_DATA_LENGTH, SEEK_SET);
    write(file, packet.data, strlen(packet.data));
    is_write[seqNum - 1] = true;
	packetCnt++;
}

PacketRecvHandler::~PacketRecvHandler() {
    close(file);
}