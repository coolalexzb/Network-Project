
#ifndef PRO2_HELPER_H
#define PRO2_HELPER_H

// slide window
const int WINDOW_SIZE = 16;
const int PACKET_DATA_LENGTH = 10;
const int PACKET_TIMEOUT_TIME = 10;

// packet positions
const int PACKET_HEADER_POS = 0;
const int PACKET_CHECKSUM_POS = 2;
const int PACKET_PACKETNUM_POS = 4;
const int PACKET_FILEPATHLEN_POS = 6;
const int PACKET_FILEPATH_POS = 8;
const int PACKET_DATALEN_POS = 4;
const int PACKET_DATA_POS = 6;

// buffer
const int BUF_LEN = 65535;

#endif
