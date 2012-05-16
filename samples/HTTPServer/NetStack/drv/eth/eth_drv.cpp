
/*
Copyright (c) 2010 Donatien Garnier (donatiengar [at] gmail [dot] com)
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "netCfg.h"
#if NET_ETH

#include "mbed.h"

Ethernet *pEth = NULL;
#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/opt.h"

#include "lwip/def.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "netif/etharp.h"
#include "string.h"

//#include "eth_drv.h"

#define IFNAME0 'E'
#define IFNAME1 'X'

#define min(x,y) (((x)<(y))?(x):(y))

struct netif* eth_netif;

static err_t eth_output(struct netif *netif, struct pbuf *p) {
  #if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
  #endif

  do {
    pEth->write((const char *)p->payload, p->len);
  } while((p = p->next)!=NULL);

  pEth->send();

  #if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
  #endif
  
  LINK_STATS_INC(link.xmit);
  return ERR_OK;
}

/*
void show(char *buf, int size) {
    printf("Destination:  %02hx:%02hx:%02hx:%02hx:%02hx:%02hx\n",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    printf("Source: %02hx:%02hx:%02hx:%02hx:%02hx:%02hx\n",
            buf[6], buf[7], buf[8], buf[9], buf[10], buf[11]);
  
    printf("Type %hd\n", htons((short)buf[12]));
    
   // hexview(buf, size);
}
*/

int eth_poll() {
  int PacketsReceived = 0;
  struct eth_hdr *ethhdr;
  struct pbuf *frame, *p;
  int len, read;

  while((len = pEth->receive()) != 0) {
      PacketsReceived++;
      frame = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
      if(frame == NULL) {
          return PacketsReceived;
      }
      p = frame;
      /* no packet could be read, silently ignore this */
      if (p == NULL) return PacketsReceived;
      do {
         read = pEth->read((char *)p->payload, p->len);
         p = p->next;
      } while(p != NULL && read != 0);
      
      /* Ignore this packet if it is too small to contain a valid Ethernet header. */
      if (len < (sizeof(struct eth_hdr) + ETH_PAD_SIZE))
      {
        pbuf_free(frame);
        frame = NULL;
        continue;
      }
      
      #if ETH_PAD_SIZE
          pbuf_header(p, ETH_PAD_SIZE);
      #endif

      ethhdr = (struct eth_hdr *)(frame->payload);
      
      switch (htons(ethhdr->type)) {
      /* IP or ARP packet? */
      case ETHTYPE_IP:
      case ETHTYPE_ARP:
      #if PPPOE_SUPPORT
      /* PPPoE packet? */
      case ETHTYPE_PPPOEDISC:
      case ETHTYPE_PPPOE:
      #endif /* PPPOE_SUPPORT */
      /* full packet send to tcpip_thread to process */
        //if (netif->input(p, gnetif)!=ERR_OK)
        if (ethernet_input(frame, eth_netif)!=ERR_OK)
        { LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
          pbuf_free(frame);
          frame = NULL;
        }
        break;

      default:
        pbuf_free(frame);
        frame = NULL;
        break;
      }
  }

  return PacketsReceived;
}

err_t eth_init(struct netif *netif) {
  LWIP_ASSERT("netif != NULL", (netif != NULL));
  
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 0x2EA);
  
  /* maximum transfer unit */
  netif->mtu = 0x2EA;
  
  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_IGMP;

  netif->state = NULL;
  eth_netif = netif;

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;

  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output          = etharp_output;
  netif->linkoutput      = eth_output;

  if (!pEth) pEth = new Ethernet(); // only create Ethernet object if required

  // UNDONE: Remove.  Just for debug
  pEth->set_link(Ethernet::HalfDuplex100);
  
  return ERR_OK;
}

void eth_free()
{
  if(pEth)
    delete pEth;
  pEth = NULL;
}

void eth_address(char* mac) {
    pEth->address(mac);
}

Ethernet* eth_interface() {
    return pEth;
}    

#ifdef __cplusplus
};
#endif

#endif
