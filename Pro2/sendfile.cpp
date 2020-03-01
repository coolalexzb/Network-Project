//
// Created by 郑博 on 2/25/20.
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
#include <sys/time.h>
#include <fcntl.h>


/* simple client, takes two parameters, the server domain name,
   and the server port number */

char *buf;
int BUF_LEN = 65535;

int main(int argc, char** argv) {


    char* recv_host = strtok(argv[2], ":");
    unsigned short recv_port = atoi(strtok(NULL, ":"));
    char* subdir = strtok(argv[4], "/");
    char* filename = strtok( NULL, "/");

    /* server socket address variables */
    struct sockaddr_in sin, sout, addr;

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

    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        abort();
    }

    /* fill in the address of the server socket */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(recv_port);

    // init receiver address info
    memset(&sout, 0, sizeof(sout));
    sout.sin_family = AF_INET;
    sout.sin_addr.s_addr = inet_addr(recv_host);
    sout.sin_port = htons(recv_port);

    buf = (char *)malloc(BUF_LEN);


    /* bind server socket to the address */
    if (bind(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)      // bind server ip port to socket
    {
        perror("binding socket to address");
        abort();
    }

    /* put the server socket in listen mode */
    if (listen (sock, BACKLOG) < 0)                           // to listen status
    {
        perror ("listen on socket failed");
        abort();
    }

    /* make the socket non-blocking so send and recv will
     return immediately if the socket is not ready.
     this is important to ensure the server does not get
     stuck when trying to send data to a socket that
     has too much data to send already.*/
    if (fcntl (sock, F_SETFL, O_NONBLOCK) < 0)
    {
        perror ("making socket non-blocking");
        abort ();
    }

    while(1) {


        // TODO 1st send packet


        // recv ACK


        FD_ZERO (&read_set); /* clear everything */               // file descriptor receive
        FD_ZERO (&write_set); /* clear everything */              // file descriptor send out

        FD_SET (sock, &read_set); /* put the listening socket in */

        time_out.tv_usec = 100000; /* 1-tenth of a second timeout */
        time_out.tv_sec = 0;

        /* invoke select, make sure to pass max+1 !!! */
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
                //TODO when recv ack packet

            }

        }

    }

    return 0;
}