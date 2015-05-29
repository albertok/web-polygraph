
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

/* this file should only be compiled as a part of msl_test.c */
#ifndef COMPILING_MSL_TEST
#error make msl_test instead
#else

#include <net/bpf.h>
#include <net/route.h>
#include <assert.h>

#define TCPSYNFLAG(tcphdr) (tcphdr->th_flags & TH_SYN)
#define TCPFINFLAG(tcphdr) (tcphdr->th_flags & TH_FIN)
#define TCPPUSHFLAG(tcphdr) (tcphdr->th_flags & TH_PUSH)
#define TCPACKFLAG(tcphdr) (tcphdr->th_flags & TH_ACK)
#define TCPRSTFLAG(tcphdr) (tcphdr->th_flags & TH_RST)
#define TCPSOURCEPORT(tcphdr) (tcphdr->th_sport)
#define TCPDESTPORT(tcphdr) (tcphdr->th_dport)

static int raw_open(char *interface);
static void msl_probe(void *, int);
static int deleteRoute(struct sockaddr_in);
static int addRoute(struct sockaddr_in);
static int chooseBPFint(void);
static void *find_our_packet(int *len);

#define SEQ_ADD_ROUTE 100
#define SEQ_DELETE_ROUTE 200
#define SIZEOF_BPF_HDR  (sizeof(struct bpf_hdr) <= 20 ? 18 : \
    sizeof(struct bpf_hdr))

#define MAX_BPF_BUFLEN 8192

static int bpf_sock = -1;
static unsigned int bpf_buflen = MAX_BPF_BUFLEN;

static void *bpfWalkStart = NULL;
static void *bpfWalkCur = NULL;

void *
bpfWalkInit(void *start, int *len)
{
    struct bpf_hdr *b;
    char *pkt;
    assert(NULL == bpfWalkCur);
    bpfWalkStart = bpfWalkCur = b = start;
    pkt = (void *) b + b->bh_hdrlen;
    *len = b->bh_caplen;
    return pkt;
}

void
bpfWalkEnd(void)
{
    bpfWalkStart = bpfWalkCur = NULL;
}

/* sorry, this function is really ugly */
void *
bpfWalkNext(int *len)
{
    struct bpf_hdr *b = bpfWalkCur;
    char *pkt;
    assert(NULL != bpfWalkCur);
    if (b->bh_hdrlen == 0) {
	bpfWalkStart = bpfWalkCur = NULL;
	return NULL;
    }
    bpfWalkCur += BPF_WORDALIGN(b->bh_hdrlen + b->bh_caplen);
    if (bpfWalkCur >= (bpfWalkStart + bpf_buflen)) {
	bpfWalkStart = bpfWalkCur = NULL;
	return NULL;
    }
    b = bpfWalkCur;
    if (b->bh_hdrlen == 0) {
	bpfWalkStart = bpfWalkCur = NULL;
	return NULL;
    }
    pkt = (void *) b + b->bh_hdrlen;
    *len = b->bh_caplen;
    return pkt;
}

int
addRoute(struct sockaddr_in dst)
{
    struct {
	struct rt_msghdr rtm;
	struct sockaddr_in host;
	struct sockaddr_in localhost;
    } rtmsg;
    int route;
    int seq = SEQ_ADD_ROUTE;
    int msglen;

    if ((route = socket(PF_ROUTE, SOCK_RAW, 0)) < 0) {
	perror("socket: PF_ROUTE");
	return -1;
    }
    msglen = sizeof(rtmsg);
    memset(&rtmsg, 0, msglen);

    rtmsg.host.sin_family = AF_INET;
    rtmsg.host.sin_addr.s_addr = dst.sin_addr.s_addr;
    rtmsg.host.sin_len = sizeof(struct sockaddr_in);;

    rtmsg.localhost.sin_family = AF_INET;
    rtmsg.localhost.sin_addr.s_addr = inet_addr("127.0.0.1");
    rtmsg.localhost.sin_len = sizeof(struct sockaddr_in);

    rtmsg.rtm.rtm_msglen = msglen;
    rtmsg.rtm.rtm_version = RTM_VERSION;
    rtmsg.rtm.rtm_type = RTM_ADD;
    rtmsg.rtm.rtm_flags = RTF_UP | RTF_STATIC | RTA_GATEWAY | RTF_HOST | RTF_BLACKHOLE;
    rtmsg.rtm.rtm_addrs = RTA_DST | RTA_GATEWAY;
    rtmsg.rtm.rtm_pid = getpid();
    rtmsg.rtm.rtm_seq = seq;

    if (write(route, &rtmsg, msglen) < 0) {
	perror("write: route");
	return -1;
    }
    close(route);
    return 0;
}

int
deleteRoute(struct sockaddr_in dst)
{
    int route;
    struct {
	struct rt_msghdr rtm;
	struct sockaddr_in host;
    } rtmsg;
    int seq = SEQ_DELETE_ROUTE;
    int msglen;

    if ((route = socket(PF_ROUTE, SOCK_RAW, 0)) < 0) {
	perror("socket: PF_ROUTE");
	return -1;
    }
    msglen = sizeof(rtmsg);
    memset(&rtmsg, 0, msglen);

    rtmsg.host.sin_family = AF_INET;
    rtmsg.host.sin_addr.s_addr = dst.sin_addr.s_addr;
    rtmsg.host.sin_len = sizeof(struct sockaddr_in);;

    rtmsg.rtm.rtm_msglen = msglen;
    rtmsg.rtm.rtm_version = RTM_VERSION;
    rtmsg.rtm.rtm_type = RTM_DELETE;
    rtmsg.rtm.rtm_addrs = RTA_DST;
    rtmsg.rtm.rtm_pid = getpid();
    rtmsg.rtm.rtm_seq = seq;

    if (write(route, (char *) &rtmsg, msglen) < 0) {
	perror("write: route");
	return -1;
    }
    close(route);
    return 0;
}


void
msl_probe(void *frame, int framelen)
{
    char buf[MAX_BPF_BUFLEN];
    struct timeval start;
    struct timeval now;
    struct timeval last;
    int nfds = bpf_sock + 1;
    int state = 0;

    gettimeofday(&start, NULL);
    last = start;
    fprintf(stderr, "Probing");
    while (0 == state) {
	struct timeval timeout;
	fd_set readfds;
	gettimeofday(&now, NULL);
	if (now.tv_sec - last.tv_sec > 0) {
	    int x;
	    fprintf(stderr, ".");
	    x = write(bpf_sock, frame, framelen);
	    if (x < 0) {
		perror("write: bpf");
		return;
	    }
	    last = now;
	}
	FD_ZERO(&readfds);
	FD_SET(bpf_sock, &readfds);
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	if (select(nfds, &readfds, NULL, NULL, &timeout)) {
	    char *p;
	    int l;
	    memset(buf, '\0', bpf_buflen);
	    if (read(bpf_sock, buf, bpf_buflen) < 0) {
		perror("read: bpf");
		return;
	    }
	    for (p = bpfWalkInit(buf, &l); p; p = bpfWalkNext(&l)) {
		if (0 == packetIsEirSynAck(p)) {
		    state++;
		    break;
		}
	    }
	}
	bpfWalkEnd();
    }
    fprintf(stderr, "\n");
    gettimeofday(&now, NULL);
    printf("TCP TIME_WAIT of %d\n",
	(int) (now.tv_sec - start.tv_sec));
}

int
raw_open(char *interface)
{
    int bpf;
    struct ifreq *ifreq = NULL;
    struct timeval rto;
    unsigned char yes = 1;
    bpf = chooseBPFint();
    ifreq = (struct ifreq *) malloc(sizeof(struct ifreq));
    strcpy(ifreq->ifr_name, interface);
    if (ioctl(bpf, BIOCSETIF, ifreq)) {
	perror("BIOCSETIF");
	return -1;
    }
    free(ifreq);
    if (ioctl(bpf, BIOCGBLEN, &bpf_buflen)) {
	perror("BIOCGBLEN");
	return -1;
    }
    assert(bpf_buflen <= MAX_BPF_BUFLEN);
    fprintf(stderr, "BPF buffer size is %d\n", bpf_buflen);
    rto.tv_sec = 1;
    rto.tv_usec = 0;
#if 0
    if (ioctl(bpf, BIOCSRTIMEOUT, &rto)) {
	perror("BIOCSRTIMEOUT");
	return -1;
    }
#endif
    if (ioctl(bpf, BIOCIMMEDIATE, &yes)) {
	perror("BIOCIMMEDIATE");
	return -1;
    }
    if (ioctl(bpf, BIOCFLUSH, NULL)) {
	perror("BIOCFLUSH");
	return -1;
    }
    return (bpf_sock = bpf);
}

int
chooseBPFint(void)
{
    int bpf, i;
    char p[32];

    for (i = 0; i < 16; i++) {
	snprintf(p, 32, "/dev/bpf%d", i);
	bpf = open(p, O_RDWR);
	if (bpf >= 0) {
	    fprintf(stderr, "Using %s\n", p);
	    return bpf;
	}
	perror(p);
    }
    return -1;

}

static void
raw_close(void)
{
    assert(bpf_sock > -1);
    close(bpf_sock);
}

static void *
find_our_packet(int *len)
{
    char buf[MAX_BPF_BUFLEN];
    static void *synbuf = NULL;
    int state = 0;
    int l;
    int count = 20;
    fprintf(stderr, "Reading our packets");
    while (count-- && 2 != state) {
	void *p = buf;
	memset(buf, '\0', MAX_BPF_BUFLEN);
	l = read(bpf_sock, buf, bpf_buflen);
	if (l < 0) {
	    perror("find_our_packet: read");
	    return NULL;
	}
	for (p = bpfWalkInit(buf, &l); p; p = bpfWalkNext(&l)) {
	    fprintf(stderr, ".");
	    switch (state) {
	    case 0:
		/* looking for a SYN */
		if (0 == packetIsOurSyn(p)) {
		    state++;
		    synbuf = malloc(l);
		    memcpy(synbuf, p, l);
		    *len = l;
		}
		break;
	    case 1:
		/* looking for a FIN+ACK */
		if (0 == packetIsOurFinAck(p))
		    state++;
		break;
	    default:
		assert(0);
		break;
	    }
	    if (2 == state)
		break;
	}
	bpfWalkEnd();
    }
    fprintf(stderr, "\n");
    return synbuf;
}

#endif /* COMPILING_MSL_TEST */
