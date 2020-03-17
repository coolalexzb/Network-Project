
#ifndef PRO2_HELPER_H
#define PRO2_HELPER_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <vector>
#include <cstring>
#include <iostream>
#include <sys/stat.h>

// slide window
const int WINDOW_SIZE = 8;
const int PACKET_DATA_LENGTH = 1018;
const int PACKET_TIMEOUT_TIME = 10;
const int ACK_PACKET_LENGTH = 8;

// packet positions
const int PACKET_HEADER_POS = 0;
const int PACKET_CHECKSUM_POS = 2;
const int PACKET_PACKETNUM_POS = 4;
const int PACKET_FILEPATHLEN_POS = 6;
const int PACKET_FILEPATH_POS = 8;
const int PACKET_DATALEN_POS = 4;
const int PACKET_DATA_POS = 6;
const int ACK_NO_VALUE_FLAG = -1;

// buffer
const int BUF_LEN = 65535;

// checksum functions
unsigned short generateCkSum(char* buf, int packetLen);
bool checksum(unsigned short ckSum, char* buf, int packetLen);

// packet examination
void packetExam(char* buf, int cnt);

#endif
