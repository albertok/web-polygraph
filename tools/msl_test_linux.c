
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

/* this file should only be compiled as a part of msl_test.c */
#ifndef COMPILING_MSL_TEST
#error make msl_test instead
#else


/*
 * Linux-dependent stuff for msl_test
 */

#include <net/route.h>
#include <netpacket/packet.h>
#include <assert.h>

#define TCPSYNFLAG(tcphdr) (tcphdr->syn)
#define TCPFINFLAG(tcphdr) (tcphdr->fin)
#define TCPPUSHFLAG(tcphdr) (tcphdr->psh)
#define TCPACKFLAG(tcphdr) (tcphdr->ack)
#define TCPRSTFLAG(tcphdr) (tcphdr->rst)
#define TCPSOURCEPORT(tcphdr) (tcphdr->source)
#define TCPDESTPORT(tcphdr) (tcphdr->dest)

static int raw_open(char *interface);
static void raw_close(void);
static void msl_probe(void *, int);
static int deleteRoute(struct sockaddr_in);
static int addRoute(struct sockaddr_in);
static void *find_our_packet(int *len);

const int RECV_BUF_SIZE = 8192;

static int raw_sock = -1;
static struct sockaddr_ll From;


int
addRoute(struct sockaddr_in dst)
{
    int route;
    struct rtentry rt;
    struct sockaddr *rt_dst = NULL;
    if ((route = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	fprintf(stderr, "socket failed with: %s\n", strerror(errno));
	return (-1);
    }
    rt_dst = (struct sockaddr *) &dst;
    rt.rt_dst = *rt_dst;
    rt.rt_dev = (char *) malloc(sizeof(char) * 3);
    strcpy(rt.rt_dev, "lo");
    rt.rt_metric = 0;
    rt.rt_flags = RTF_UP | RTF_STATIC | RTF_HOST;
    if (ioctl(route, SIOCADDRT, &rt) < 0) {
	fprintf(stderr, "ioctl failed with: %s\n", strerror(errno));
	close(route);
	return (-1);
    }
    close(route);
    return (1);
}

int
deleteRoute(struct sockaddr_in dst)
{
    int route;
    struct rtentry rt;
    struct sockaddr *rt_dst = NULL;
    if ((route = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	fprintf(stderr, "socket failed with: %s\n", strerror(errno));
	return (-1);
    }
    rt_dst = (struct sockaddr *) &dst;
    rt.rt_dst = *rt_dst;
    rt.rt_dev = (char *) malloc(sizeof(char) * 3);
    strcpy(rt.rt_dev, "lo");
    rt.rt_metric = 0;
    rt.rt_flags = RTF_UP | RTF_STATIC | RTF_HOST;
    if (ioctl(route, SIOCDELRT, &rt) < 0) {
	close(route);
	return (-1);
    }
    close(route);
    return (1);
}


void
msl_probe(void *frame, int framelen)
{
    char buf[RECV_BUF_SIZE];
    struct timeval timeout;
    struct timeval start;
    struct timeval last;
    struct timeval now;
    int nfds = raw_sock + 1;

    gettimeofday(&start, NULL);
    last = start;
    fprintf(stderr, "Probing");
    for (;;) {
	fd_set readfds;
	gettimeofday(&now, NULL);
	if (now.tv_sec - last.tv_sec > 0) {
	    FD_ZERO(&readfds);
	    FD_SET(raw_sock, &readfds);
	    fprintf(stderr, ".");
	    if (sendto(raw_sock, frame, framelen, 0, (struct sockaddr *) &From, sizeof(From)) < 0) {
		perror("sendto: raw_sock");
		return;
	    }
	    last = now;
	}
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	if (select(nfds, &readfds, NULL, NULL, &timeout)) {
	    if (recv(raw_sock, buf, RECV_BUF_SIZE, 0) < 0) {
		perror("recv: raw_sock");
		return;
	    }
	    if (0 == packetIsEirSynAck(buf))
		break;
	}
    }
    fprintf(stderr, "\n");
    printf("TCP TIME_WAIT of %d seconds\n",
	(int) (now.tv_sec - start.tv_sec));
}

static int
raw_open(char *ifname)
{
    int s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    struct sockaddr_ll sa;
    if (s < 0) {
	perror("SOCK_RAW");
	return s;
    }
    memset(&sa, '\0', sizeof(sa));
    sa.sll_family = AF_PACKET;
    strncpy((char *) sa.sll_addr, ifname, sizeof(sa.sll_addr));
    if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
	perror(ifname);
	close(s);
	return -1;
    }
    return (raw_sock = s);
}


static void
raw_close(void)
{
    assert(raw_sock > -1);
    close(raw_sock);
}


static void *
find_our_packet(int *len)
{
    static char buf[RECV_BUF_SIZE];
    static void *synbuf = NULL;
    int state = 0;
    int l;
    int count = 20;
    socklen_t fl;
    fprintf(stderr, "Reading our packets");
    while (count-- && 2 != state) {
	memset(buf, '\0', RECV_BUF_SIZE);
	memset(&From, '\0', fl = sizeof(From));
	if ((l = recvfrom(raw_sock, buf, RECV_BUF_SIZE, 0, (struct sockaddr *) &From, &fl)) < 0) {
	    perror("find_our_packet: read");
	    return NULL;
	}
	fprintf(stderr, ".");
	switch (state) {
	case 0:
	    /* looking for a SYN */
	    if (0 == packetIsOurSyn(buf)) {
		state++;
		synbuf = malloc(l);
		memcpy(synbuf, buf, l);
		*len = l;
	    }
	    break;
	case 1:
	    /* looking for a FIN+ACK */
	    if (0 == packetIsOurFinAck(buf))
		state++;
	    break;
	default:
	    assert(0);
	    break;
	}
    }
    fprintf(stderr, "\n");
    return synbuf;
}

#endif /* COMPILING_MSL_TEST */

