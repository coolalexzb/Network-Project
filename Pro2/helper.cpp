//
// Created by 郑博 on 3/6/20.
//

#include "helper.h"

int DATA_PACKET_DATA_LENGTH = 10;
int DATA_PACKET_CONTENT = 6;
int WINDOW_SIZE = 10;
const int PACKET_HEADER_LENGTH = 10;
const int PACKET_DATA_LENGTH = 10;
const int PACKET_TIMEOUT_TIME = 10;
int ACK_PACKET_LENGTH = 8;
int DATA_PACKET_SEQ= 0;
int DATA_PACKET_CKSUM = 2;
int DATA_PACKET_LEN = 4;
int ACK_PACKET_CKSUM_LENGTH = 2;

// ACK packet
const int ACK_FLAG_POS = 0;
const int ACK_CHECKSUM_POS = 2;

// send packet
const int SEND_SEQ_POS = 0;
const int SEND_CHECKSUM_POS = 2;
const int SEND_LENGTH_POS = 4;

// flag
const int ACK_FINISH_FLAG = -1;
const int ACK_INVALID_FLAG = -2;