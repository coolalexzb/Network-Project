
#ifndef PRO2_PACKETSENDHANDLER_H
#define PRO2_PACKETSENDHANDLER_H

#include "helper.h"

using namespace std;

extern const int WINDOW_SIZE;
extern const int PACKET_DATA_LENGTH;
extern const int PACKET_TIMEOUT_TIME;
extern const int PACKET_HEADER_POS;
extern const int PACKET_CHECKSUM_POS;
extern const int PACKET_PACKETNUM_POS;
extern const int PACKET_FILEPATHLEN_POS;
extern const int PACKET_FILEPATH_POS;
extern const int PACKET_DATALEN_POS;
extern const int PACKET_DATA_POS;

struct packet {
    char* data;
    short len;
    short seq;
    time_t time;
    bool isAck;
};

typedef struct packet *packetPtr;

class PacketSendHandler {
public:
    
	PacketSendHandler(char* filePath);
	~PacketSendHandler();

	packetPtr newPacket();
	packetPtr getUnAckPacket(time_t curTime);
	void recv_ack(short ackSeq);
	bool isWindowFull();
	bool isOver();

private:

	packet *slideWindow;					// sliding window
	
	bool startSending;						// whether start sending
	bool finishSending;						// whether finish sending
	short seqFirst;							// first seq Number of packet not sent successfully
	short seqNext;							// next packet to be sent

	int file;
	char* filePath;
	long fileLen;
	long sendingPos;

	void init();
	bool isInWindow(short ackSeq);
	void updateSeqInfo(short ackSeq);
};

#endif