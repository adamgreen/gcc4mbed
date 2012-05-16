/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include "netCfg.h"
#if NET_LWIP_STACK

//#include "arch/sys_arch.h"

/* <sys/time.h> is included in cc.h! */
#define LWIP_TIMEVAL_PRIVATE 0

// Set LWIP_DEBUG to 1 to enable extra debugging option in lwIP.
#define LWIP_DEBUG 1

#if LWIP_DEBUG

#define LWIP_DBG_MIN_LEVEL LWIP_DBG_LEVEL_ALL
#define TAPIF_DEBUG LWIP_DBG_OFF
#define TUNIF_DEBUG LWIP_DBG_OFF
#define UNIXIF_DEBUG LWIP_DBG_OFF
#define DELIF_DEBUG LWIP_DBG_OFF
#define SIO_FIFO_DEBUG LWIP_DBG_OFF
#define TCPDUMP_DEBUG LWIP_DBG_OFF

#define PPP_DEBUG        LWIP_DBG_OFF
#define MEM_DEBUG        LWIP_DBG_OFF
#define MEMP_DEBUG       LWIP_DBG_OFF
#define PBUF_DEBUG       LWIP_DBG_OFF
#define API_LIB_DEBUG    LWIP_DBG_OFF
#define API_MSG_DEBUG    LWIP_DBG_OFF
#define TCPIP_DEBUG      LWIP_DBG_OFF
#define NETIF_DEBUG      LWIP_DBG_OFF
#define SOCKETS_DEBUG    LWIP_DBG_OFF
#define DEMO_DEBUG       LWIP_DBG_OFF
#define IP_DEBUG         LWIP_DBG_OFF
#define IP_REASS_DEBUG   LWIP_DBG_OFF
#define RAW_DEBUG        LWIP_DBG_OFF
#define ICMP_DEBUG       LWIP_DBG_OFF
#define IGMP_DEBUG       LWIP_DBG_OFF
#define UDP_DEBUG        LWIP_DBG_OFF
#define TCP_DEBUG        LWIP_DBG_OFF
#define TCP_INPUT_DEBUG  LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG LWIP_DBG_OFF
#define TCP_RTO_DEBUG    LWIP_DBG_OFF
#define TCP_CWND_DEBUG   LWIP_DBG_OFF
#define TCP_WND_DEBUG    LWIP_DBG_OFF
#define TCP_FR_DEBUG     LWIP_DBG_OFF
#define TCP_QLEN_DEBUG   LWIP_DBG_OFF
#define TCP_RST_DEBUG    LWIP_DBG_OFF
#define ETHARP_DEBUG     LWIP_DBG_OFF
#define DNS_DEBUG        LWIP_DBG_OFF

#endif /* LWIP_DEBUG */

/*
extern unsigned char debug_flags;
#define LWIP_DBG_TYPES_ON debug_flags
*/
#define NO_SYS                     1
#define LWIP_SOCKET                (NO_SYS==0)
#define LWIP_NETCONN               (NO_SYS==0)


#define IP_FRAG_USES_STATIC_BUF         0



/* Number of HTTP client connections to support at once. */
#define HTTP_CLIENTS            2    // UNDONE: No reason to support more than this for a little embedded device.

/* Number of buffers to be kept in the HTTP server pool.  Each buffer will
   contain TCP_SND_BUF, the maximum number of bytes that can be written to a
   TCP/IP connection.  If memory wasn't a limit then we would want to have
   2 of these for each client connection.  One with data in it to be written
   out to the connection and another keeping already written data around until 
  it has been acknowledged by the client.  */
#define HTTP_BUFFER_POOL        HTTP_CLIENTS   // UNDONE: This should be atleast 2x if using a DMA driver.

/* The number of TCP segments to queue up for each tcp_write() call.
   TCP_SND_BUF will be based on this value. */
#define TCP_SEGMENTS_PER_SEND   4

/* The number of TCP segments in the advertised TCP/IP window.  The HTTP client
   can queue up this many inbound segments before waiting for an ACK.
   8 * 692 = 5536 which based on LAN ping times and 100Mbit/second bandwidth
   should be enough to receive ACK before filling the window. */
#define TCP_SEGMENTS_IN_WND     8



/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           4

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE                (80 * TCP_SEGMENTS_PER_SEND * HTTP_BUFFER_POOL)  // UNDONE: Shrink this down.  Should only need room for headers ~(80 bytes/header pbuf * 4 segments/send * 2 sends_in_flight)

// UNDONE: I think this should be in a different header.
#define MEM_POSITION __attribute((section("AHBSRAM1"),aligned))

/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        2   // UNDONE: I only see 2 get used in my testing so decreased it to this value.
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB        HTTP_CLIENTS   // UNDONE: I only want to handle 2 HTTP connections at a time.
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 1   // UNDONE: Only listening for HTTP requests on port 80 in my application.
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG        (TCP_SEGMENTS_IN_WND * HTTP_CLIENTS + TCP_SEGMENTS_PER_SEND * HTTP_BUFFER_POOL)  // UNDONE: Handle maximum inbound/outbound TCP segments.
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    12  // UNDONE: During a basic test run, I see 7 being the maximum in-use.

/* The following four are used only with the sequential API and can be
   set to 0 if the application only will use the raw API. */
/* MEMP_NUM_NETBUF: the number of struct netbufs. */
#define MEMP_NUM_NETBUF         0
/* MEMP_NUM_NETCONN: the number of struct netconns. */
#define MEMP_NUM_NETCONN        0
/* MEMP_NUM_TCPIP_MSG_*: the number of struct tcpip_msg, which is used
   for sequential API communication and incoming packets. Used in
   src/api/tcpip.c. */
#define MEMP_NUM_TCPIP_MSG_API   0
#define MEMP_NUM_TCPIP_MSG_INPKT 0

/** SYS_LIGHTWEIGHT_PROT
 * define SYS_LIGHTWEIGHT_PROT in lwipopts.h if you want inter-task protection
 * for certain critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT           0 //No sys here

/* ---------- TCP options ---------- */
#define LWIP_TCP                1

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         1

/* The maximum MTU supported by our Ethernet driver. */
#define ETHERNET_MTU            0x2EA   // UNDONE: This is the value in ethdrv.cpp.  Just less than 1/2 of Ethernet's 1500 MTU

/* TCP Maximum segment size. */
#define TCP_MSS                 (ETHERNET_MTU - 40 - PBUF_LINK_HLEN)    // UNDONE: 40 is the size of the TCP/IP headers.

/**
 * TCP_WND: The size of a TCP window.  This must be at least 
 * (2 * TCP_MSS) for things to work well
 */
#define TCP_WND                 (TCP_SEGMENTS_IN_WND * TCP_MSS)    // UNDONE: Advertised receive window: 8 * 692 = 5536 which based on LAN ping times and 100Mbit/second bandwidth should be enough to receive ACK before filling the window.

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             (TCP_SEGMENTS_PER_SEND * TCP_MSS)   // UNDONE: The send window is set by the HTTP client.  This just controls how much data the HTTP server can queue up on each poll instance.

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN        (2 * TCP_SND_BUF/TCP_MSS)           // UNDONE: (1 for header pbuf and another for rom pbuf to application data)



/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           (MEMP_NUM_TCP_PCB * (TCP_SND_BUF/TCP_MSS))  // UNDONE: We require one of these for each segment of data we send down for a full window (8) for the 2 sockets we are supporting.

/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE          10      // (MEMP_NUM_TCP_PCB * (TCP_WND/TCP_MSS))  // UNDONE: Similar to MEMP_NUM_PBUF except for inbound packets.






/* ---------- ARP options ---------- */
#define LWIP_ARP                (NET_ETH | NET_ZG2100)
#define ARP_TABLE_SIZE          4
#define ARP_QUEUEING            0
#define ETHARP_TRUST_IP_MAC     1

/* ---------- IP options ---------- */
/* Define IP_FORWARD to 1 if you wish to have the ability to forward
   IP packets across network interfaces. If you are going to run lwIP
   on a device with only one network interface, define this to 0. */
#define IP_FORWARD              0


/* IP reassembly and segmentation.These are orthogonal even
 * if they both deal with IP fragments */
 /*
#define IP_REASSEMBLY     1
#define IP_REASS_MAX_PBUFS      10
#define MEMP_NUM_REASSDATA      10
#define IP_FRAG           1
*/
/* ---------- ICMP options ---------- */

/* ---------- IGMP options ---------- */
#define LWIP_IGMP               (NET_ETH | NET_ZG2100)

/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
   interfaces. */
#define LWIP_DHCP               (NET_ETH | NET_ZG2100)
#define LWIP_NETIF_HOSTNAME     1

/* 1 if you want to do an ARP check on the offered address
   (recommended if using DHCP). */
#define DHCP_DOES_ARP_CHECK     (LWIP_DHCP)

/* ---------- AUTOIP options ------- */
#define LWIP_AUTOIP             0

/* ---------- SNMP options ---------- */
/** @todo SNMP is experimental for now
    @note UDP must be available for SNMP transport */
#ifndef LWIP_SNMP
#define LWIP_SNMP               0
#endif


#ifndef SNMP_PRIVATE_MIB
#define SNMP_PRIVATE_MIB        0
#endif


/* ---------- UDP options ---------- */
#define LWIP_UDP                1

/* ---------- DNS options ---------- */
#define LWIP_DNS                1

/* ---------- RAW options ---------- */
#define LWIP_RAW                0

/* ---------- Statistics options ---------- */
/**
 * LWIP_STATS==1: Enable statistics collection in lwip_stats.
 */
#define LWIP_STATS                      1

/**
 * LWIP_STATS_DISPLAY==1: Compile in the statistics output functions.
 */
#define LWIP_STATS_DISPLAY              1

/**
 * LINK_STATS==1: Enable link stats.
 */
#define LINK_STATS                      0

/**
 * ETHARP_STATS==1: Enable etharp stats.
 */
#define ETHARP_STATS                    0

/**
 * IP_STATS==1: Enable IP stats.
 */
#define IP_STATS                        0

/**
 * IPFRAG_STATS==1: Enable IP fragmentation stats. Default is
 * on if using either frag or reass.
 */
#define IPFRAG_STATS                    0

/**
 * ICMP_STATS==1: Enable ICMP stats.
 */
#define ICMP_STATS                      0

/**
 * IGMP_STATS==1: Enable IGMP stats.
 */
#define IGMP_STATS                      0

/**
 * UDP_STATS==1: Enable UDP stats. Default is on if
 * UDP enabled, otherwise off.
 */
#define UDP_STATS                       0

/**
 * TCP_STATS==1: Enable TCP stats. Default is on if TCP
 * enabled, otherwise off.
 */
#define TCP_STATS                       1

/**
 * MEM_STATS==1: Enable mem.c stats.
 */
#define MEM_STATS                       1

/**
 * MEMP_STATS==1: Enable memp.c pool stats.
 */
#define MEMP_STATS                      1

/**
 * SYS_STATS==1: Enable system stats (sem and mbox counts, etc).
 */
#define SYS_STATS                       0


/* ---------- PPP options ---------- */

#define PPP_SUPPORT      NET_PPP      /* Set > 0 for PPP */

#if PPP_SUPPORT > 0

#define NUM_PPP 1           /* Max PPP sessions. */


/* Select modules to enable.  Ideally these would be set in the makefile but
 * we're limited by the command line length so you need to modify the settings
 * in this file.
 */
#define PAP_SUPPORT      1      /* Set > 0 for PAP. */
#define CHAP_SUPPORT     1      /* Set > 0 for CHAP. */
#define MSCHAP_SUPPORT   0      /* Set > 0 for MSCHAP (NOT FUNCTIONAL!) */
#define CBCP_SUPPORT     0      /* Set > 0 for CBCP (NOT FUNCTIONAL!) */
#define CCP_SUPPORT      0      /* Set > 0 for CCP (NOT FUNCTIONAL!) */
#define VJ_SUPPORT       1      /* Set > 0 for VJ header compression. */
#define MD5_SUPPORT      1      /* Set > 0 for MD5 (see also CHAP) */


/*
 * Timeouts.
 */
#define FSM_DEFTIMEOUT        6    /* Timeout time in seconds */
#define FSM_DEFMAXTERMREQS    2    /* Maximum Terminate-Request transmissions */
#define FSM_DEFMAXCONFREQS    10    /* Maximum Configure-Request transmissions */
#define FSM_DEFMAXNAKLOOPS    5    /* Maximum number of nak loops */

#define UPAP_DEFTIMEOUT        6    /* Timeout (seconds) for retransmitting req */
#define UPAP_DEFREQTIME        30    /* Time to wait for auth-req from peer */

#define CHAP_DEFTIMEOUT        6    /* Timeout time in seconds */
#define CHAP_DEFTRANSMITS    10    /* max # times to send challenge */


/* Interval in seconds between keepalive echo requests, 0 to disable. */
#define LCP_ECHOINTERVAL 0


/* Number of unanswered echo requests before failure. */
#define LCP_MAXECHOFAILS 3

/* Max Xmit idle time (in jiffies) before resend flag char. */
#define PPP_MAXIDLEFLAG 0//Send it every time//100

/*
 * Packet sizes
 *
 * Note - lcp shouldn't be allowed to negotiate stuff outside these
 *    limits.  See lcp.h in the pppd directory.
 * (XXX - these constants should simply be shared by lcp.c instead
 *    of living in lcp.h)
 */
#define PPP_MTU     1500     /* Default MTU (size of Info field) */
#if 0
#define PPP_MAXMTU  65535 - (PPP_HDRLEN + PPP_FCSLEN)
#else

#define PPP_MAXMTU  1500 /* Largest MTU we allow */
#endif

#define PPP_MINMTU  64
#define PPP_MRU     1500     /* default MRU = max length of info field */
#define PPP_MAXMRU  1500     /* Largest MRU we allow */
#define PPP_DEFMRU    296        /* Try for this */
#define PPP_MINMRU    128        /* No MRUs below this */


#define MAXNAMELEN      64     /* max length of hostname or name for auth */
#define MAXSECRETLEN    64     /* max length of password or secret */

#endif /* PPP_SUPPORT > 0 */

//C++ Compat
#define try vTry

#endif


#endif /* __LWIPOPTS_H__ */
