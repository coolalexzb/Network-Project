//
// Created by 郑博 on 3/5/20.
//

#ifndef PRO2_HELPER_H
#define PRO2_HELPER_H

// packet positions
const int PACKET_HEADER_POS = 0;
const int PACKET_CHECKSUM_POS = 2;
const int PACKET_PACKETNUM_POS = 4;
const int PACKET_FILEPATHLEN_POS = 6;
const int PACKET_FILEPATH_POS = 8;
const int PACKET_DATALEN_POS = 4;
const int PACKET_DATA_POS = 6;

extern int DATA_PACKET_DATA_LENGTH;
extern int DATA_PACKET_CONTENT;
extern int WINDOW_SIZE;
extern const int PACKET_HEADER_LENGTH;
extern const int PACKET_DATA_LENGTH;
extern const int PACKET_TIMEOUT_TIME;
extern int ACK_PACKET_LENGTH;
extern int DATA_PACKET_SEQ;
extern int DATA_PACKET_CKSUM;
extern int DATA_PACKET_LEN;
extern int ACK_PACKET_CKSUM_LENGTH;

// ACK packet
extern const int ACK_FLAG_POS;
extern const int ACK_CHECKSUM_POS;

// send packet
extern const int SEND_SEQ_POS;
extern const int SEND_CHECKSUM_POS;
extern const int SEND_LENGTH_POS;

// flag
extern const int ACK_FINISH_FLAG;
extern const int ACK_INVALID_FLAG;

#endif //PRO2_HELPER_H
