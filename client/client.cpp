#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include "err.h"
#include <netinet/in.h>

#define BUFFER_SIZE 1000

int main(int argc, char *argv[])
{
    int sock;
    struct addrinfo addr_hints;
    struct addrinfo *addr_result;

    int i, flags, sflags;
    char buffer[BUFFER_SIZE];
    size_t len;
    ssize_t snd_len, rcv_len;
    struct sockaddr_in my_address;
    struct sockaddr_in srvr_address;
    socklen_t rcva_len;

    if (argc < 3)
    {
        fatal("Usage: %s host port message ...\n", argv[0]);
    }

    // 'converting' host/port in string to struct addrinfo
    (void)memset(&addr_hints, 0, sizeof(struct addrinfo));
    addr_hints.ai_family = AF_INET; // IPv4
    addr_hints.ai_socktype = SOCK_DGRAM;
    addr_hints.ai_protocol = IPPROTO_UDP;
    addr_hints.ai_flags = 0;
    addr_hints.ai_addrlen = 0;
    addr_hints.ai_addr = NULL;
    addr_hints.ai_canonname = NULL;
    addr_hints.ai_next = NULL;
    if (getaddrinfo(argv[1], NULL, &addr_hints, &addr_result) != 0)
    {
        syserr("getaddrinfo");
    }

    my_address.sin_family = AF_INET; // IPv4
    my_address.sin_addr.s_addr =
        ((struct sockaddr_in *)(addr_result->ai_addr))->sin_addr.s_addr; // address IP
    my_address.sin_port = htons((uint16_t)atoi(argv[2]));                // port from the command line

    freeaddrinfo(addr_result);

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        syserr("socket");

    /*wklejam*/
    printf("a\n");
    const char *msg = "\0\0\0\0\0\0\0\0\1\0\0\0\0player2";
    printf("b\n");
    len = 20;
    (void)printf("sending to socket: %s\n", msg);
    sflags = 0;
    rcva_len = (socklen_t)sizeof(my_address);
    printf("d\n");
    snd_len = sendto(sock, msg, len, sflags,
                     (struct sockaddr *)&my_address, rcva_len);
    printf("e\n");
    if (snd_len != (ssize_t)len)
    {
        syserr("partial / failed write");
    }
    /*wkleilem*/

    if (close(sock) == -1)
    {                    //very rare errors can occur here, but then
        syserr("close"); //it's healthy to do the check
    }

    return 0;
}
