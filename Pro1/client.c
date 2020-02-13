//
// Created by 郑博 on 2/7/20.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/* simple client, takes two parameters, the server domain name,
   and the server port number */

int main(int argc, char** argv) {

    /* our client socket */
    int sock, server_sock;

    /* variables for identifying the server */
    unsigned int server_addr;
    struct sockaddr_in sin;
    struct addrinfo *getaddrinfo_result, hints;

    /* convert server domain name to IP address */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* indicates we want IPv4 */

    if (getaddrinfo(argv[1], NULL, &hints, &getaddrinfo_result) == 0) {
        server_addr = (unsigned int) ((struct sockaddr_in *) (getaddrinfo_result->ai_addr))->sin_addr.s_addr;
        freeaddrinfo(getaddrinfo_result);
    }

    /* server port number */
    unsigned short server_port = atoi (argv[2]);

    char *buffer;
    int size = 65535;
    unsigned short send_size = (unsigned short)strtol(argv[3],NULL,10);
    int cnt = atoi(argv[4]);
    /* allocate a memory buffer in the heap */
    /* putting a buffer on the stack like:

           char buffer[500];

       leaves the potential for
       buffer overflow vulnerability */
    buffer = (char *) malloc(size);
    if (!buffer)
    {
        //perror("failed to allocated buffer");
        abort();
    }

    /* create a socket */
    if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        //perror ("opening TCP socket");
        abort ();
    }
    /* fill in the server's address */
    memset (&sin, 0, sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = server_addr;
    sin.sin_port = htons(server_port);

    /* connect to the server */
    if (connect(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
        //perror("connect to server failed");
        abort();
    }

    int i = 0;
    float time_total = 0;
    for(i = 0; i < cnt; i++) {
        char data[send_size - 10];
        memset(data, 'a', sizeof(data));
        data[sizeof(data) - 1] = '\0';

        unsigned short sendLen = sizeof(data) + 10;
        char *sendbuffer;
        sendbuffer = (char *) malloc(sendLen);

        struct timeval start;
        gettimeofday(&start, NULL);
        time_t tv_sec = start.tv_sec;
        suseconds_t tv_usec = start.tv_usec;
        *(unsigned short *) (sendbuffer) = (unsigned short) htons(sendLen);
        *(int *) (sendbuffer + 2) = (int) htonl((int)tv_sec);
        *(int *) (sendbuffer + 6) = (int) htonl((int)tv_usec);
        memcpy(sendbuffer + 10, data, sizeof(data));

        send(sock, sendbuffer, sendLen, 0);

        char *recbuffer;
        recbuffer = (char *) malloc(sendLen);
        unsigned short curSize = 0;
        unsigned short offset = 0;
        unsigned short recLen = 0;
        while(curSize != sendLen) {
            recLen = recv(sock, recbuffer + offset, sendLen, 0);
            curSize += recLen;
            offset += recLen;
        }

        // time latency
        struct timeval end;
        gettimeofday(&end, NULL);
        float time_use=(end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) * 0.001;
        printf("rtt:  %f\n", time_use);
        if(time_use > 3){
            continue;
        }
        time_total += time_use;
        cnt--;
    }

    float time_avg = time_total / cnt;
    //printf("cnt: %d\n", cnt);
    //printf("time_avg: %f\n", time_avg);

    return 0;
}
