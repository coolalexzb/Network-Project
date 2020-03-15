
#ifndef PRO2_PACKETRECVHANDLER_H
#define PRO2_PACKETRECVHANDLER_H

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
	short seq;
	bool isRecv;
};

class PacketRecvHandler {
public:

	PacketRecvHandler();
	~PacketRecvHandler();

	bool isOver();
	short getNextSeq();
	short recvPacket(char *packet, int length);
	short updateSeq(short seqNum);
	int getPacketSize();

private:

    Packet *slideWindow;						// sliding window

	bool startSending;							// whether start sending
	
	short seqOldest;							// oldest seq Number of packet not sent successfully
	short seqNext;								// latest packet received
	short packetSize;
	short packetCnt;
    char* filePath;

	int file;
	void init();
	void wrote(Packet packet);
};

#endif
