//
// Created by 郑博 on 2/4/20.
//

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

/**************************************************/
/* a few simple linked list functions             */
/**************************************************/

/* a buffer to read data */
char *buf;
int BUF_LEN = 65535;
char* HeaderResp[] = {"HTTP/1.1 200 OK \r\n", "HTTP/1.1 400 Bad Request \r\n", "HTTP/1.1 404 Not Found \r\n", "HTTP/1.1 500 Internal Server Error \r\n", "HTTP/1.1 501 Not Implemented \r\n", "Content-Type: text/html\r\n\r\n"};

/* A linked list node data structure to maintain application
   information related to a connected socket */
struct node {
    int socket;
    struct sockaddr_in client_addr;
    int pending_data; /* flag to indicate whether there is more data to send */
    /* you will need to introduce some variables here to record
       all the information regarding this socket.
       e.g. what data needs to be sent next */
    //char* buffer;
    struct node *next;
};

/* remove the data structure associated with a connected socket
   used when tearing down the connection */
void dump(struct node *head, int socket) {
    struct node *current, *temp;
    current = head;

    while (current->next) {
        if (current->next->socket == socket) {
            temp = current->next;
            current->next = temp->next;
            free(temp);
            return;
        } else {
            current = current->next;
        }
    }
}

/* create the data structure associated with a connected socket */
void add(struct node *head, int socket, struct sockaddr_in addr) {
    struct node *new_node;

    new_node = (struct node *)malloc(sizeof(struct node));
    new_node->socket = socket;
    new_node->client_addr = addr;
    new_node->pending_data = 0;
    new_node->next = head->next;
    head->next = new_node;
}

int getStatus(const char *path) {
    if(strstr(path, "../") != NULL) {
        return 1;
    }else if(open(path, O_RDONLY) < 0) {
        return 2;
    }else if(opendir(path) != NULL) {
        return 4;
    }

    return 0;
}

char* concat(const char *s1, const char *s2) {
    char *res = malloc(strlen(s1) + strlen(s2) + 1); // 1 for '\0'
    strcpy(res, s1);
    strcat(res, s2);
    return res;
}

void sendInfo(char* path, struct node *current) {
    char byte[BUF_LEN + 1];
    byte[BUF_LEN] = '\0';
    int file = open(path, O_RDONLY);
    if (file < 0) {
        printf("path: %s\n", path);
        printf("File open failed!\n");
    }
    lseek(file, 0, SEEK_SET);
    ssize_t cnt = read(file, &byte, BUF_LEN);

    int offset = 1;
    while(cnt > 0) {
        buf = byte;
        send(current->socket, buf, strlen(buf) + 1, 0);
        lseek(file, BUF_LEN * offset, SEEK_SET);
        memset(byte,'\0',sizeof(byte));
        cnt = read(file, &byte, BUF_LEN);
        offset++;
    }
    close(file);
}

char* parse(char* buf, char* root_directory) {
    char* ele = strtok(buf, " /t/r/n");
    if (ele != NULL)
    {
        ele = strtok(NULL, " ");
    }
    char *filename = ele;
    return strcat(root_directory, filename);
}

void sendHeader(int index, struct node *current) {
    char *respHeader = concat(HeaderResp[index], HeaderResp[5]);
    printf("H: %s", respHeader);
    send(current->socket, respHeader, strlen(respHeader) + 1, 0);
    free(respHeader);
    return;
}
/*****************************************/
/* main program                          */
/*****************************************/

/* simple server, takes one parameter, the server port number */
int main(int argc, char **argv) {

    /* socket and option variables */
    int sock, new_sock, max;
    int optval = 1;

    /* server socket address variables */
    struct sockaddr_in sin, addr;
    unsigned short server_port = atoi(argv[1]);


    char* mode = argv[2];
    char* root_directory = argv[3];
    printf("mode %s\n", mode);
    printf("root_directory %s\n", root_directory);

    /* socket address variables for a connected client */
    socklen_t addr_len = sizeof(struct sockaddr_in);

    /* maximum number of pending connection requests */
    int BACKLOG = 5;

    /* variables for select */
    fd_set read_set, write_set;
    struct timeval time_out;
    int select_retval;

    /* a silly message */
    char *message = "Welcome! COMP/ELEC 429 Students!\n";

    /* number of bytes sent/received */
    int count;

    /* numeric value received */
    int num;

    /* linked list for keeping track of connected sockets */
    struct node head;
    struct node *current, *next;



    buf = (char *)malloc(BUF_LEN);

    /* initialize dummy head node of linked list */
    head.socket = -1;
    head.next = 0;

    /* create a server socket to listen for TCP connection requests */
    if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)              // server welcome socket
    {
        perror ("opening TCP socket");
        abort ();
    }

    /* set option so we can reuse the port number quickly after a restart */
    if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval)) <0)
    {
        perror ("setting TCP socket option");
        abort ();
    }

    /* fill in the address of the server socket */
    memset (&sin, 0, sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons (server_port);

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

    /* now we keep waiting for incoming connections,
       check for incoming data to receive,
       check for ready socket to send more data */
    while (1)
    {

        /* set up the file descriptor bit map that select should be watching */
        FD_ZERO (&read_set); /* clear everything */               // file descriptor receive
        FD_ZERO (&write_set); /* clear everything */              // file descriptor send out

        FD_SET (sock, &read_set); /* put the listening socket in */
        max = sock; /* initialize max */

        /* put connected sockets into the read and write sets to monitor them */
        for (current = head.next; current; current = current->next) {     // multiple connections
            FD_SET(current->socket, &read_set);

            if (current->pending_data) {                                 // read multiple times
                /* there is data pending to be sent, monitor the socket
                       in the write set so we know when it is ready to take more
                       data */
                FD_SET(current->socket, &write_set);
            }

            if (current->socket > max) {
                /* update max if necessary */
                max = current->socket;
            }
        }

        time_out.tv_usec = 100000; /* 1-tenth of a second timeout */
        time_out.tv_sec = 0;

        /* invoke select, make sure to pass max+1 !!! */
        select_retval = select(max+1, &read_set, &write_set, NULL, &time_out);    // cnt of variable socket
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
            if (FD_ISSET(sock, &read_set)) /* check the server socket */          // read var?
            {
                /* there is an incoming connection, try to accept it */
                new_sock = accept (sock, (struct sockaddr *) &addr, &addr_len);       // new socket in server to connect r/w

                if (new_sock < 0)
                {
                    perror ("error accepting connection");
                    abort ();
                }

                /* make the socket non-blocking so send and recv will
                         return immediately if the socket is not ready.
                         this is important to ensure the server does not get
                         stuck when trying to send data to a socket that
                         has too much data to send already.
                    */
                if (fcntl (new_sock, F_SETFL, O_NONBLOCK) < 0)
                {
                    perror ("making socket non-blocking");
                    abort ();
                }

                /* the connection is made, everything is ready */
                /* let's see who's connecting to us */
                printf("Accepted connection. Client IP address is: %s\n",
                       inet_ntoa(addr.sin_addr));

                /* remember this client connection in our linked list */
                add(&head, new_sock, addr);

                /* let's send a message to the client just for fun */
                count = send(new_sock, message, strlen(message)+1, 0);                // delete!!!
                if (count < 0)
                {
                    perror("error sending message to client");
                    abort();
                }
            }

            /* check other connected sockets, see if there is
                   anything to read or some socket is ready to send
                   more pending data */
            for (current = head.next; current; current = next) {
                next = current->next;

                /* see if we can now do some previously unsuccessful writes */
                if (FD_ISSET(current->socket, &write_set)) {
                    /* the socket is now ready to take more data */
                    /* the socket data structure should have information
                           describing what data is supposed to be sent next.
                       but here for simplicity, let's say we are just
                           sending whatever is in the buffer buf
                         */
                    count = send(current->socket, buf, BUF_LEN, MSG_DONTWAIT);
                    if (count < 0) {
                        if (errno == EAGAIN) {
                            /* we are trying to dump too much data down the socket,
                               it cannot take more for the time being
                               will have to go back to select and wait til select
                               tells us the socket is ready for writing
                            */
                        } else {
                            /* something else is wrong */
                        }
                    }
                    /* note that it is important to check count for exactly
                           how many bytes were actually sent even when there are
                           no error. send() may send only a portion of the buffer
                           to be sent.
                    */
                }

                if (FD_ISSET(current->socket, &read_set)) {
                    /* we have data from a client */

                    count = recv(current->socket, buf, BUF_LEN, 0);

                    if (count <= 0) {
                        /* something is wrong */
                        if (count == 0) {
                            printf("Client closed connection. Client IP address is: %s\n", inet_ntoa(current->client_addr.sin_addr));
                        } else {
                            perror("error receiving from a client");
                        }

                        /* connection is closed, clean up */
                        close(current->socket);
                        dump(&head, current->socket);
                    } else {
                        if(strcmp(mode, "www") == 0) {
                            printf("buf\n%s\n", buf);
                            char* path = parse(buf, root_directory);
                            int status = getStatus(path);
                            printf("path %s\n", path);

                            switch(status){
                                case 1:
                                    buf = "400";
                                    sendHeader(status, current);
                                    send(current->socket, buf, strlen(buf) + 1, 0);
                                    break;
                                case 2:
                                    buf = "404";
                                    sendHeader(status, current);
                                    send(current->socket, buf, strlen(buf) + 1, 0);
                                    break;
                                case 4:
                                    buf = "501";
                                    sendHeader(status, current);
                                    send(current->socket, buf, strlen(buf) + 1, 0);
                                    break;
                                default:
                                    sendHeader(status, current);
                                    sendInfo(path, current);
                            }

                            close(current->socket);
                            dump(&head, current->socket);
                        } else {
                            //Ping-Pong
                            unsigned short size_t = (unsigned short) ntohs(*(unsigned short *)(buf));
                            unsigned short curSize = count;
                            unsigned short offset = count;
                            unsigned short recLen = 0;
                            while(curSize != size_t) {
                                recLen = recv(current->socket, buf + offset, BUF_LEN, 0);
                                curSize += recLen;
                                offset += recLen;
                            }

                            int t1 = (int) ntohl(*(int *)(buf + 2));
                            int t2 = (int) ntohl(*(int *)(buf + 6));
                            printf("size:   %d\n", size_t);
                            printf("t1:   %d\n", t1);
                            printf("t2:   %d\n", t2);
                            printf("data:   %ld\n", strlen(buf + 10));
                            //printf("data:   %s\n", buf + 10);
                            send(current->socket, buf, size_t, 0);
                        }
                    }
                }
            }
        }
    }
}
