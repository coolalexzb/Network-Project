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
    int size = 10000;
    int count;
    int num;
    unsigned short send_size = (unsigned short)strtol(argv[3],NULL,10);
    int cnt = atoi(argv[4]);
    printf("send_size: %d\n", send_size);
    printf("cnt: %d\n", cnt);
    /* allocate a memory buffer in the heap */
    /* putting a buffer on the stack like:

           char buffer[500];

       leaves the potential for
       buffer overflow vulnerability */
    buffer = (char *) malloc(size);
    if (!buffer)
    {
        perror("failed to allocated buffer");
        abort();
    }



    /* create a socket */
    if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror ("opening TCP socket");
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
        perror("connect to server failed");
        abort();
    }

    /* everything looks good, since we are expecting a
       message from the server in this example, let's try receiving a
       message from the socket. this call will block until some data
       has been received */
    count = recv(sock, buffer, size, 0);
    if (count < 0)
    {
        perror("receive failure");
        abort();
    }

    /* in this simple example, the message is a string,
       we expect the last byte of the string to be 0, i.e. end of string */
    if (buffer[count-1] != 0)
    {
        /* In general, TCP recv can return any number of bytes, not
       necessarily forming a complete message, so you need to
       parse the input to see if a complete message has been received.
           if not, more calls to recv is needed to get a complete message.
        */
        printf("Message incomplete, something is still being transmitted\n");
    }
    else
    {
        printf("Here is what we got: %s", buffer);
    }

    for(int i = 0; i < cnt; i++) {
        char data[send_size - 10];
        memset(data, 'a', sizeof(data));
        printf("Enter the value of the number to send: ");
        fgets(buffer, size, stdin);
        num = atol(buffer);
        if (strncmp(buffer, "bye", 3) == 0) {
            /* free the resources, generally important! */
            close(sock);
            free(buffer);
            return 0;
        }

        struct timeval start;
        gettimeofday(&start, NULL);
        time_t tv_sec = start.tv_sec;
        suseconds_t tv_usec = start.tv_usec;
        unsigned short sendLen = sizeof(data) + 10;
        char *sendbuffer;
        sendbuffer = (char *) malloc(sendLen);

        *(unsigned short *) (sendbuffer) = (unsigned short) htons(sendLen);
        *(int *) (sendbuffer + 2) = (int) htonl((int)tv_sec);
        *(int *) (sendbuffer + 6) = (int) htonl((int)tv_usec);
        memcpy(sendbuffer + 10, data, sizeof(data));

        printf("===========\n");
        //printf("senddata:  %s\n", sendbuffer + 10);
        printf("senddata:   %ld\n", strlen(sendbuffer + 10));
        printf("data:   %ld\n", sizeof(data));
        printf("sendLen: %d\n", sendLen);
        send(sock, sendbuffer, sendLen, 0);

        char *recbuffer;
        recbuffer = (char *) malloc(sendLen);
        unsigned short recLen = 0;


        unsigned short offset = 0;
        while(sendLen >= size) {
            recLen = recv(sock, recbuffer + offset, size, 0);
            sendLen -= recLen;
            offset += recLen;
            printf("sendLen: %d   recLen:  %d\n", sendLen, recLen);
        }
        if(sendLen > 0) {
            recLen = recv(sock, recbuffer + offset, sendLen, 0);
            sendLen -= recLen;
            offset += recLen;
            printf("sendLen: %d   recLen:  %d\n", sendLen, recLen);
        }

        // time latency
        struct timeval end;
        gettimeofday(&end, NULL);
        float time_use=(end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) * 0.001;
        //parse
        unsigned short size_t = (short) ntohs(*(short *)(recbuffer));
        int t1 = (int) ntohl(*(int *)(recbuffer + 2));
        int t2 = (int) ntohl(*(int *)(recbuffer + 6));
        printf("size:   %d\n", size_t);
        printf("t1:   %d\n", t1);
        printf("t2:   %d\n", t2);
        //printf("recdata:   %s\n", recbuffer + 10);
        printf("recdata:   %ld\n", strlen(recbuffer + 10));
        printf("time_use:  %f\n", time_use);
    }



    return 0;
}
