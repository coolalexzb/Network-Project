
#ifndef PRO2_PACKETRECVHANDLER_H
#define PRO2_PACKETRECVHANDLER_H

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "helper.h"

using namespace std;

extern const int WINDOW_SIZE;
extern const int PACKET_DATA_LENGTH;
extern const int PACKET_HEADER_POS;
extern const int PACKET_CHECKSUM_POS;
extern const int PACKET_PACKETNUM_POS;
extern const int PACKET_FILEPATHLEN_POS;
extern const int PACKET_FILEPATH_POS;
extern const int PACKET_DATALEN_POS;
extern const int PACKET_DATA_POS;

struct Packet {
	char *data;
	int seq;
	bool isRecv;
};

class PacketRecvHandler {
public:

	PacketRecvHandler();
	bool isOver();
	short getNextSeq();
	short recvPacket(char * packet, int length, bool isHeader);
	short updateSeq(short seqNum);
	int getPacketSize();
	~PacketRecvHandler();

private:

    Packet *slideWindow;						// sliding window
	short seqOldest;							// The oldest seq Number of packet not sent successfully
	short seqNext;								// Next packet to be sent
	short packetSize;
	short packetCnt;
    char* filePath;

	int file;
	void init();
	void wrote(Packet packet);

};

#endif
