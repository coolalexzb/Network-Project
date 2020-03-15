
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
