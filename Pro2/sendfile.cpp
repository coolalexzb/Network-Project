
#include "PacketSendHandler.h"
#include "helper.h"

char *buf;
char *recBuff;
extern const int BUF_LEN;

extern const int PACKET_HEADER_POS;
extern const int PACKET_CHECKSUM_POS;
extern const int PACKET_PACKETNUM_POS;
extern const int PACKET_FILEPATHLEN_POS;
extern const int PACKET_FILEPATH_POS;
extern const int PACKET_DATALEN_POS;
extern const int PACKET_DATA_POS;

int main(int argc, char **argv) {

    char *recv_host = strtok(argv[2], ":");
    unsigned short recv_port = atoi(strtok(NULL, ":"));
    char* filePath = argv[4];
	printf("--------------- Sending Info ----------------\n");
    printf("recv_host: %s\n", recv_host);
    printf("recv_port: %d\n", recv_port);
	printf("filePath: %s\n", filePath);
	printf("--------------- Sending Start ---------------\n");

    /* server socket address variables */
    struct sockaddr_in sin, sout;

    /* socket address variables for a connected client */
    socklen_t addr_len = sizeof(struct sockaddr_in);

    /* socket and option variables */
    int sock;

    /* variables for select */
    fd_set read_set, write_set;
    struct timeval time_out;
    int select_retval;

    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        abort();
    }

    /* fill in the address of the server socket */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(0);

    // init receiver address info
    memset(&sout, 0, sizeof(sout));
    sout.sin_family = AF_INET;
    sout.sin_addr.s_addr = inet_addr(recv_host);
    sout.sin_port = htons(recv_port);

    buf = (char *) malloc(BUF_LEN);
    recBuff = (char *) malloc(BUF_LEN);

    PacketSendHandler handle(filePath);

    /* bind server socket to the address */
    if (bind(sock, (struct sockaddr *) &sin, sizeof(sin)) < 0) {		// bind server ip port to socket
        perror("binding socket to address");
        abort();
    }

    timeval tv;
    gettimeofday(&tv, nullptr);
    time_t startTime = tv.tv_usec / 1000 + tv.tv_sec * 1000;
    
	while (1) {
        
		// check timeout packet
        gettimeofday(&tv, nullptr);
        time_t currTime = tv.tv_usec / 1000 + tv.tv_sec * 1000;
        packetPtr resendPacket = handle.getUnAckPacket(currTime);

		// resending
		if (resendPacket != nullptr) {
			gettimeofday(&tv, nullptr);
			resendPacket->time = tv.tv_usec / 1000 + tv.tv_sec * 1000;
			sendto(sock, resendPacket->data, resendPacket->len, 0, (struct sockaddr *) &sout, sizeof(sockaddr));
		}
        
		// sending
		if (!handle.isWindowFull() && handle.isSendingOver() == false) {
			packetPtr newPacket = handle.newPacket();
			if (newPacket != nullptr) {
				gettimeofday(&tv, nullptr);
				newPacket->time = tv.tv_usec / 1000 + tv.tv_sec * 1000;
				sendto(sock, newPacket->data, newPacket->len, 0, (struct sockaddr *) &sout, sizeof(sockaddr));
			}
		}

        FD_ZERO (&read_set);				// file descriptor receive
        FD_ZERO (&write_set);				// file descriptor send out

        FD_SET (sock, &read_set);			/* put the listening socket in */

        time_out.tv_usec = PACKET_TIMEOUT_TIME;
        time_out.tv_sec = 0;

        select_retval = select(sock + 1, &read_set, &write_set, NULL, &time_out);		// cnt of variable socket
        if (select_retval < 0) {
            perror("select failed");
            abort();
        }

        if (select_retval == 0) {		// no var within timeout
			/* no descriptor ready, timeout happened */
			continue;
        }

        if (select_retval > 0) {		/* at least one file descriptor is ready */
            if (FD_ISSET(sock, &read_set)) {		/* check the server socket */

                int recvLen = recvfrom(sock, recBuff, BUF_LEN, 0, (struct sockaddr *) &sout, &addr_len);
                short ckSum = (short) ntohs(*(short *)(recBuff + PACKET_CHECKSUM_POS));

                if (checksum(ckSum, recBuff, recvLen)) {
                    short ackFlag = (short)ntohs(*(short *) (recBuff + PACKET_HEADER_POS));
                    // receive a valid or invalid packet
                    handle.recv_ack(ackFlag);
                }

                if (handle.isAllOver()) {
                    cout << "[ Completed ] Sending finished!" << endl;
					int count = 0;
					while (count < 20) {
						count++;
						char* fin = (char *)malloc(4);
						strcpy(fin, "fin");
						sendto(sock, fin, 4, 0, (struct sockaddr *) &sout, sizeof(sockaddr));
					}
                    break;
                }
            }
        }
    }

    gettimeofday(&tv, nullptr);
    time_t endTime = tv.tv_usec / 1000 + tv.tv_sec * 1000;
    printf("[ Performance ] Time used: %ld (ms)\n", endTime - startTime);

    exit(0);
}