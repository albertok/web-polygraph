
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/uio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/socket.h>

#include <net/if.h>
#include <net/ethernet.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>

static int createTCPsession(struct sockaddr_in);
static void printPacket(void *);
static void parseOptions(int argc, char *argv[]);
static void usage(const char *prg_name);
static void breakupPacket(void *, struct ether_header **, struct ip **, struct tcphdr **);
int packetIsOurSyn(void *buf);
int packetIsOurFinAck(void *buf);
int packetIsEirSynAck(void *buf);

#define READ_SIZE 1024
#define HTTP_REQUEST "GET / HTTP/1.0\n\n"

static char *arg_interface = NULL;
static struct sockaddr_in Dest;
static struct sockaddr_in Source;

/*
 * XXX Ugliness Warning: The .c files included below have #define macros
 * that are used in this file.  TCPSYNFLAG, etc.
 */
#define COMPILING_MSL_TEST
#ifdef __linux__
#include "msl_test_linux.c"
#else
#include "msl_test_bsd.c"
#endif

static void
usage(const char *prg_name)
{
    fprintf(stderr,
	"Usage: %s -i interface-name -s src-host -d dst-host -p dst-port\n"
	"       -i interface-name The name of the interface which to listen.\n"
	"       -s src-host	 The IP address of the local host\n"
	"       -d dst-host       The IP address of the destination.\n"
	"       -p dst-port       Port which to connect to.\n"
	"You must be root to run this program.\n",
	prg_name);
    exit(1);
}

static void
parseOptions(int argc, char *argv[])
{
    extern char *optarg;
    int c;

    memset(&Dest, '\0', sizeof(Dest));
    memset(&Source, '\0', sizeof(Source));
    Dest.sin_family = AF_INET;
    Source.sin_family = AF_INET;
    while ((c = getopt(argc, argv, "i:s:d:p:?")) != -1) {
	switch (c) {
	case 'i':
	    arg_interface = strdup(optarg);
	    break;
	case 's':
	    Source.sin_addr.s_addr = inet_addr(optarg);
	    break;
	case 'd':
	    Dest.sin_addr.s_addr = inet_addr(optarg);
	    break;
	case 'p':
	    Dest.sin_port = htons(atoi(optarg));
	    break;
	case '?':
	default:
	    usage(argv[0]);
	    break;
	}
    }
}


int
main(int argc, char **argv)
{
    void *pkt = NULL;
    int etherlen = 0;

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    if (argc == 9) {
	parseOptions(argc, argv);
    } else {
	usage(argv[0]);
    }

    if (getuid()) {
	printf("You must be root to run this program!\n");
	return (-1);
    }
    (void) deleteRoute(Dest);

    if (raw_open(arg_interface) < 0)
	return 1;

    if (createTCPsession(Dest))
	return 1;

    /*
     * now we want to find our SYN packet so we can
     * replay it again.
     */
    if (NULL == (pkt = find_our_packet(&etherlen))) {
	fprintf(stderr, "Failed to capture our SYN packet\n");
	return 1;
    }
    if (addRoute(Dest) < 0) {
	printf("Unable to create blackhole route\n");
	return 1;
    }
    printf("Sending a %d byte packet: ", etherlen);
    printPacket(pkt);

    msl_probe(pkt, etherlen);

    if (deleteRoute(Dest) < 0)
	printf("Unable to remove blackhole route\n");

    raw_close();
    return 0;
}

static void
breakupPacket(void *buf,
    struct ether_header **e,
    struct ip **i,
    struct tcphdr **t)
{
    int ip_hl;
    *e = (struct ether_header *) buf;
    *i = NULL;
    *t = NULL;
    if (ETHERTYPE_IP != ntohs((*e)->ether_type))
	return;
    *i = (struct ip *) (*e + 1);
    if (IPPROTO_TCP != (*i)->ip_p)
	return;
    ip_hl = (*i)->ip_hl << 2;
    *t = (struct tcphdr *) ((char *) *i + ip_hl);
}

void
printPacket(void *buf)
{
    struct ether_header *e;
    struct ip *i;
    struct tcphdr *t;
    breakupPacket(buf, &e, &i, &t);
    if (NULL == e) {
	printf("Not an Ethernet Packet\n");
	return;
    }
    if (NULL == i) {
	printf("Not an IP Packet\n");
	return;
    }
    if (NULL == t) {
	printf("Not a TCP Packet\n");
	return;
    }
    if (80 != ntohs(TCPSOURCEPORT(t)) && 80 != ntohs(TCPDESTPORT(t)))
	return;
    printf("%s.%d > ", inet_ntoa(i->ip_src), ntohs(TCPSOURCEPORT(t)));
    printf("%s.%d\t", inet_ntoa(i->ip_dst), ntohs(TCPDESTPORT(t)));
    if (TCPSYNFLAG(t))
	printf("SYN ");
    if (TCPFINFLAG(t))
	printf("FIN ");
    if (TCPPUSHFLAG(t))
	printf("PUSH ");
    if (TCPACKFLAG(t))
	printf("ACK ");
    if (TCPRSTFLAG(t))
	printf("RST ");
    printf("\n");
}

int
createTCPsession(struct sockaddr_in S)
{
    int sock;
    socklen_t nl;
    char buf[READ_SIZE];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	return (1);

    fprintf(stderr, "Connecting to %s:%d\n", inet_ntoa(S.sin_addr), ntohs(S.sin_port));
    if (connect(sock, (struct sockaddr *) &S, sizeof(S))) {
	perror("connect");
	return (1);
    }
    nl = sizeof(Source);
    getsockname(sock, (struct sockaddr *) &Source, &nl);
    fprintf(stderr, "Local port is %d\n", ntohs(Source.sin_port));

    fprintf(stderr, "Writing HTTP request\n");
    write(sock, HTTP_REQUEST, strlen(HTTP_REQUEST));

    fprintf(stderr, "Reading HTTP response\n");
    while (read(sock, buf, READ_SIZE));

    fprintf(stderr, "Closing HTTP socket\n");
    close(sock);
    return (0);
}

int
packetIsOurSyn(void *buf)
{
    struct ether_header *e;
    struct ip *i;
    struct tcphdr *t;
    breakupPacket(buf, &e, &i, &t);
    if (NULL == e)
	return 2;
    if (NULL == i)
	return 3;
    if (i->ip_src.s_addr != Source.sin_addr.s_addr)
	return 3;
    if (i->ip_dst.s_addr != Dest.sin_addr.s_addr)
	return 3;
    if (NULL == t)
	return 4;
    if (TCPFINFLAG(t))
	return 4;
    if (TCPPUSHFLAG(t))
	return 4;
    if (TCPACKFLAG(t))
	return 4;
    if (TCPRSTFLAG(t))
	return 4;
    if (!TCPSYNFLAG(t))
	return 4;
    if (TCPSOURCEPORT(t) != Source.sin_port)
	return 9;
    if (TCPDESTPORT(t) != Dest.sin_port)
	return 10;
    return 0;
}

/*
 * This function returns ZERO when the packet in 'buf' is our
 * TCP FIN+ACK from the connection created by createTCPsession()
 */

int
packetIsOurFinAck(void *buf)
{
    struct ether_header *e;
    struct ip *i;
    struct tcphdr *t;
    breakupPacket(buf, &e, &i, &t);
    if (NULL == e)
	return 2;
    if (NULL == i)
	return 3;
    if (i->ip_src.s_addr != Source.sin_addr.s_addr)
	return 3;
    if (i->ip_dst.s_addr != Dest.sin_addr.s_addr)
	return 3;
    if (NULL == t)
	return 4;
    if (TCPSYNFLAG(t))
	return 4;
    if (TCPPUSHFLAG(t))
	return 4;
    if (TCPRSTFLAG(t))
	return 4;
    if (!TCPACKFLAG(t))
	return 4;
    if (!TCPFINFLAG(t))
	return 4;
    if (TCPSOURCEPORT(t) != Source.sin_port)
	return 4;
    if (TCPDESTPORT(t) != Dest.sin_port)
	return 4;
    return 0;
}

int
packetIsEirSynAck(void *buf)
{
    struct ether_header *e;
    struct ip *i;
    struct tcphdr *t;
    breakupPacket(buf, &e, &i, &t);
    if (NULL == e)
	return 2;
    if (NULL == i)
	return 3;
    if (i->ip_src.s_addr != Dest.sin_addr.s_addr)
	return 3;
    if (i->ip_dst.s_addr != Source.sin_addr.s_addr)
	return 3;
    if (NULL == t)
	return 4;
    if (TCPFINFLAG(t))
	return 4;
    if (TCPPUSHFLAG(t))
	return 4;
    if (TCPRSTFLAG(t))
	return 4;
    if (!TCPACKFLAG(t))
	return 4;
    if (!TCPSYNFLAG(t))
	return 4;
    if (TCPSOURCEPORT(t) != Dest.sin_port)
	return 4;
    if (TCPDESTPORT(t) != Source.sin_port)
	return 4;
    return 0;
}
