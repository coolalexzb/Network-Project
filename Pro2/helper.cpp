
#include "helper.h"

// calculate checksum
unsigned short generateCkSum(char* buf, int packetLen) {
	int index = 0;
	unsigned short sum = 0;
	while (index >= PACKET_HEADER_POS && index < packetLen) {
		if (index >= PACKET_CHECKSUM_POS && index < PACKET_DATALEN_POS) {
			index++;
			continue;
		}
		sum += (unsigned short)buf[index++];
	}
	return sum;
}

// examine checksum
bool checksum(unsigned short ckSum, char* buf, int packetLen) {
	unsigned short sum = generateCkSum(buf, packetLen);
	return ckSum == sum;
}

// packet examination
void packetExam(char* buf, int cnt) {
	short seq = (short)ntohs(*(short *)buf + PACKET_HEADER_POS);
	printf("---------------PACKET %hd-----------------\n", seq);
	printf("sendingLength:\t%hd\n", cnt);
	printf("checksum:\t%hd\n", (short)ntohs(*(short *)(buf + PACKET_CHECKSUM_POS)));
	if (seq == 0) {
		printf("totalPacketNum:\t%hd\n", (short)ntohs(*(short *)(buf + PACKET_PACKETNUM_POS)));
		printf("filePathLen:\t%hd\n", (short)ntohs(*(short *)(buf + PACKET_FILEPATHLEN_POS)));
		printf("filePath:\t%s\n", buf + PACKET_FILEPATH_POS);
	}
	else {
		printf("dataLen:\t%hd\n", (short)ntohs(*(short *)(buf + PACKET_DATALEN_POS)));
		printf("data:\n%s\n", buf + PACKET_DATA_POS);
	}
	printf("-----------------------------------------\n");
}
