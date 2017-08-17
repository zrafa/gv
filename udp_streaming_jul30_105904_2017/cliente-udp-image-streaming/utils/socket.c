#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "config.h"

#define BUFSIZE PACK_SIZE

static int debug = 0;
/* 
 * error - wrapper for perror
 */
static void error(char *msg) {
    perror(msg);
    exit(0);
}

static char buf[BUFSIZE];
static int sockfd;
static struct sockaddr_in serveraddr;

int init_socket(char *hostname, int portno)
{
    struct hostent *server;

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    return 0;
}

struct header {
    int total_pack;
} header;

void send_frame(unsigned char * frame, int len)
{
    int serverlen;
    int i;
    int total_pack;
    int n;
    unsigned char buf[4];
    struct header * header_pt = (struct header *)buf;

    /* send the message to the server */
    serverlen = sizeof(serveraddr);
    total_pack = (1 + (len - 1) / PACK_SIZE);
    header_pt->total_pack = htonl(total_pack);
    if (debug) {
        printf("len: %d\n", len);
        printf("total_pack: %d\n", total_pack);
        printf("sizeof total_pack: %d\n", sizeof(total_pack));
        printf("buf[0]: %2.2X\n", buf[0]);
        printf("buf[1]: %2.2X\n", buf[1]);
        printf("buf[2]: %2.2X\n", buf[2]);
        printf("buf[3]: %2.2X\n", buf[3]);
    }

    n = sendto(sockfd, buf, 4, 0, (struct sockaddr *)&serveraddr, serverlen);
    if (n == -1) {
        fprintf(stderr,"ERROR, sendto\n");
        exit(0);
    }
  
    for (i = 0; i < total_pack; i++) {
        n = sendto(sockfd, &frame[i * PACK_SIZE], PACK_SIZE, 0, (struct sockaddr *)&serveraddr, serverlen);
        if (debug)
            printf("Packet size: %d Packet sended: %d frame[%d]\n", n, i, i * PACK_SIZE);
    }
}
