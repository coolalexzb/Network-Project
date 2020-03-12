
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include "PacketRecvHandler.h"
#include "helper.h"

/* a buffer to read data */
char *buf;
extern const int BUF_LEN;

extern const int PACKET_HEADER_POS;
extern const int PACKET_CHECKSUM_POS;
extern const int PACKET_PACKETNUM_POS;
extern const int PACKET_FILEPATHLEN_POS;
extern const int PACKET_FILEPATH_POS;
extern const int PACKET_DATALEN_POS;
extern const int PACKET_DATA_POS;

int ACK_PACKET_LENGTH = 8;

bool ackFinished = false;
bool headerFlag = true;

bool checksum(unsigned short ckSum, int recvLen)
{
    int index = 0;
    unsigned short sum  = 0;
    while(index >= PACKET_HEADER_POS && index < recvLen) {
        if(index >= PACKET_CHECKSUM_POS && index < PACKET_DATALEN_POS) {
            continue;
        }
        sum += (unsigned short)buf[index];
        index++;
    }

    return ckSum == sum;
}

unsigned short generateCkSum(char * buf) {
    return 0;
}

void generateAck(int ackNum, char* buf, bool flag) {
    *(unsigned short *) (buf) = (unsigned short) htons(ackNum);
    unsigned short ckSum = generateCkSum(buf);
    *(unsigned short *) (buf + PACKET_CHECKSUM_POS) = (unsigned short) htons(ckSum);
}

/* simple server, takes one parameter, the server port number */
int main(int argc, char **argv) {

    /* server socket address variables */
    struct sockaddr_in sin, addr;
    unsigned short server_port = atoi(argv[2]);
    printf("%d \n", server_port);
    /* socket address variables for a connected client */
    socklen_t addr_len = sizeof(struct sockaddr_in);

    /* socket and option variables */
    int sock, max;
    int optval = 1;

    /* maximum number of pending connection requests */
    int BACKLOG = 5;

    /* variables for select */
    fd_set read_set, write_set;
    struct timeval time_out;
    int select_retval;

    /* number of bytes sent/received */
    int count;

    buf = (char *)malloc(BUF_LEN);

    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        abort();
    }

//    /* set option so we can reuse the port number quickly after a restart */
//    if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval)) <0)
//    {
//        perror ("setting UDP socket option");
//        abort ();
//    }

    /* fill in the address of the server socket */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(server_port);

    /* bind server socket to the address */
    if (bind(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)      // bind server ip port to socket
    {
        perror("binding socket to address");
        abort();
    }
    /* make the socket non-blocking so send and recv will
                    return immediately if the socket is not ready.
                    this is important to ensure the server does not get
                    stuck when trying to send data to a socket that
                    has too much data to seand already.*/
    if (fcntl (sock, F_SETFL, O_NONBLOCK) < 0)
    {
        perror ("making socket non-blocking");
        abort ();
    }

    PacketRecvHandler packetHandler((char*)"./test");
    printf("pre step ok!\n");
    while(1)
    {
        FD_ZERO (&read_set); /* clear everything */               // file descriptor receive
        FD_ZERO (&write_set); /* clear everything */              // file descriptor send out

        FD_SET (sock, &read_set); /* put the listening socket in */

        time_out.tv_usec = 100000; /* 1-tenth of a second timeout */
        time_out.tv_sec = 0;

        /* invoke select, make sure to pass sock+1 !!! */
        select_retval = select(sock + 1, &read_set, &write_set, NULL, &time_out);    // cnt of variable socket
        if (select_retval < 0)
        {
            perror ("select failed");
            abort ();
        }

        if (select_retval == 0)                                   // no var within timeout
        {
            /* no descriptor ready, timeout happened */
            continue;
        }

        if (select_retval > 0) /* at least one file descriptor is ready */
        {
            if (FD_ISSET(sock, &read_set))                  /* check the server socket */
            {
                printf("recv\n");
                count = (int) recvfrom(sock, buf, BUF_LEN, 0, (sockaddr *) &addr, &addr_len);
                printf("count: %d", count);
                short ckSum = (short) ntohs(*(short *)(buf));

                int ackSeqNum = packetHandler.recvPacket(buf, count, headerFlag);
                if(packetHandler.isOver()) {
                    ackFinished = true;
                }
                generateAck(ackSeqNum, buf, ackFinished);
                printf("buf:  %s", buf);
                sendto(sock, buf, ACK_PACKET_LENGTH, 0, (sockaddr *) &sin, addr_len);
                if(headerFlag) {
                    headerFlag = false;
                }

//                if(checksum(ckSum, count)) {
//                    int ackSeqNum = packetHandler->recvPacket(buf, count, headerFlag);
//                    if(packetHandler->isOver()) {
//                        ackFinished = true;
//                    }
//                    generateAck(ackSeqNum, buf, ackFinished);
//                    sendto(sock, buf, ACK_PACKET_LENGTH, 0, (sockaddr *) &sin, addr_len);
//                    if(headerFlag) {
//                        headerFlag = false;
//                    }
//                }else {
//                    printf("recv failed\n");
//                    generateAck(ACK_NO_VALUE_FLAG, buf, ackFinished);
//                    sendto(sock, buf, ACK_PACKET_LENGTH, 0, (sockaddr *) &sin, addr_len);
//                }
            }
            printf("11111111\n");
        }
    }
}
