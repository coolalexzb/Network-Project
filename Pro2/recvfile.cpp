
#include "PacketRecvHandler.h"
#include "helper.h"

char *buf;
char *sendbuf;
extern const int BUF_LEN;

extern const int PACKET_HEADER_POS;
extern const int PACKET_CHECKSUM_POS;
extern const int PACKET_PACKETNUM_POS;
extern const int PACKET_FILEPATHLEN_POS;
extern const int PACKET_FILEPATH_POS;
extern const int PACKET_DATALEN_POS;
extern const int PACKET_DATA_POS;
extern const int ACK_NO_VALUE_FLAG;
extern const int ACK_PACKET_LENGTH;

void generateAck(int ackNum, char* buffer) {
    *(short *) (buffer) = (short) htons(ackNum);
    unsigned short ckSum = generateCkSum(buffer, ACK_PACKET_LENGTH);
    *(unsigned short *) (buffer + PACKET_CHECKSUM_POS) = (unsigned short) htons(ckSum);
}

/* simple server, takes one parameter, the server port number */
int main(int argc, char **argv) {

    /* server socket address variables */
    struct sockaddr_in sin, addr;
    unsigned short server_port = atoi(argv[2]);
    printf("server port: %d \n", server_port);
    
	/* socket address variables for a connected client */
    socklen_t addr_len = sizeof(struct sockaddr_in);

    /* socket and option variables */
    int sock;

    /* variables for select */
    fd_set read_set, write_set;
    struct timeval time_out;
    int select_retval;

    buf = (char *)malloc(BUF_LEN);
    sendbuf = (char *)malloc(BUF_LEN);

    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        abort();
    }

    /* fill in the address of the server socket */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(server_port);

    /* bind server socket to the address */
    if (bind(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0) {		// bind server ip port to socket
        perror("binding socket to address");
        abort();
    }

	printf("Ready for recv!\n");
    PacketRecvHandler packetHandler;

    while (1) {
        FD_ZERO (&read_set);				// file descriptor receive
        FD_ZERO (&write_set);				// file descriptor send out

        FD_SET (sock, &read_set);			/* put the listening socket in */

        time_out.tv_usec = PACKET_TIMEOUT_TIME;
        time_out.tv_sec = 0;

        select_retval = select(sock + 1, &read_set, &write_set, NULL, &time_out);		// cnt of variable socket
        if (select_retval < 0) {
            perror ("select failed");
            abort ();
        }

        if (select_retval == 0) {		// no var within timeout
            /* no descriptor ready, timeout happened */
            continue;
        }

        if (select_retval > 0) {		/* at least one file descriptor is ready */
            if (FD_ISSET(sock, &read_set)) {		/* check the server socket */

				int recvLen = (int) recvfrom(sock, buf, BUF_LEN, 0, (sockaddr *) &addr, &addr_len);
				
				short ckSum = (short) ntohs(*(short *)(buf + PACKET_CHECKSUM_POS));
				if (checksum(ckSum, buf, recvLen)) {
                    short ackSeqNum = packetHandler.recvPacket(buf, recvLen);
                    generateAck(ackSeqNum, sendbuf);
                    int num = sendto(sock, sendbuf, ACK_PACKET_LENGTH, 0, (sockaddr *) &addr, sizeof(addr));
                } 
				else {
                    printf("[recv corrupt packet]\n");
                    generateAck(ACK_NO_VALUE_FLAG, sendbuf);
                    sendto(sock, sendbuf, ACK_PACKET_LENGTH, 0, (sockaddr *) &addr, sizeof(addr));
                }
            }
        }
    }

	exit(0);
}
