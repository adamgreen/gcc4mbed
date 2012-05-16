/* Copyright 2011 Adam Green (http://mbed.org/users/AdamGreen/)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
/* Implementation of core lwIP ethernet functionality. */
#include <mbed.h>
#include "lwip/init.h"
#include "lwip/tcp_impl.h"
#include "lwip/dns.h"
#include "netif/etharp.h"
#include "drv/eth/eth_drv.h"
#include "network.h"
#include "debug.h"


/* Initializes the network stack.

   NOTE: Using DHCP to supply the IP addresses is the only supported scenario
         in this sample.
   
   Parameters:
    pNetwork is a pointer to the network object to be initialized.
    pIPAddress is a pointer to the string which specifies the static IP address
        of this node.  Set to NULL for DHCP.
    pSubnetMask is a pointer to the string which specifies the subnet mask for
        static IP addresses.  Set to NULL for DHCP.
    pGatewayAddress is a pointer to the string which specifies the static 
        gateway IP address.  Set to NULL for DHCP.
    pHostName is the host name to be used for representing this device to the
        DHCP server.
    
   Returns:
    0 on success and a positive error code otherwise.
*/
int SNetwork_Init(SNetwork*   pNetwork,
                  const char* pIPAddress,
                  const char* pSubnetMask,
                  const char* pGatewayAddress,
                  const char* pHostName)
{
    int         Return = 1;
    netif*      pAddResult = NULL;
    ip_addr_t   IpAddress;
    ip_addr_t   NetMask;
    ip_addr_t   GatewayAddress;
    Timer       Timeout;

    // Must specify a network object to be initialized.
    assert ( pNetwork );
    
    // Clear out the network object and fill it in during the initialization
    // process.
    memset(pNetwork, 0, sizeof(*pNetwork));
    
    // Initialize the lwIP network stack.
    lwip_init();
    
    // Set the initial IP addresses for the ethernet interface based on whether
    // DHCP is to be used or not.
    if (pIPAddress)
    {
        // For static IP, all strings must be set.
        assert ( pSubnetMask && pGatewayAddress );
        
        ip4_addr_set_u32(&IpAddress,      ipaddr_addr(pIPAddress));
        ip4_addr_set_u32(&NetMask,        ipaddr_addr(pSubnetMask));
        ip4_addr_set_u32(&GatewayAddress, ipaddr_addr(pGatewayAddress));
    }
    else
    {
        // For DHCP, all strings must be NULL.
        assert ( !pSubnetMask && !pGatewayAddress );

        // Clear the IP addresses and use DHCP instead.
        ip_addr_set_zero(&IpAddress);
        ip_addr_set_zero(&NetMask);
        ip_addr_set_zero(&GatewayAddress);
    }
    
    // Obtain the MAC address for the Ethernet device.
    // NOTE: This should really be done in eth_init()
    pNetwork->EthernetInterface.hwaddr_len = ETHARP_HWADDR_LEN;
    eth_address((char *)pNetwork->EthernetInterface.hwaddr);    
    
    // Connect the LPC17xx ethernet driver into the lwIP stack.
    pAddResult = netif_add(&pNetwork->EthernetInterface, 
                           &IpAddress, 
                           &NetMask, 
                           &GatewayAddress, 
                           NULL, 
                           eth_init, 
                           ethernet_input);
    if (!pAddResult)
    {
        printf("error: Failed to add ethernet interface to lwIP.\r\n");
        goto Error;
    }
    netif_set_default(&pNetwork->EthernetInterface);
    
    // Start the required timers.
    pNetwork->ARP.start();
    pNetwork->DHCPCoarse.start();
    pNetwork->DHCPFine.start();
    pNetwork->TCP.start();
    pNetwork->DNS.start();

    if (pIPAddress)
    {
        // Start the ethernet interface up with the static IP address.
        netif_set_up(&pNetwork->EthernetInterface);
    }
    else
    {
        err_t StartResult;
        
        // Start the DHCP request.
        pNetwork->EthernetInterface.hostname = (char*)pHostName;
        dhcp_set_struct(&pNetwork->EthernetInterface, &pNetwork->DHCP);
        StartResult = dhcp_start(&pNetwork->EthernetInterface);
        if (ERR_OK != StartResult)
        {
            printf("error: Failed to start DHCP service.\r\n");
            goto Error;
        }
    }

    // Wait for network to startup.
    Timeout.start();
    printf("Waiting for network...\r\n");

    // Wait until interface is up
    while (!netif_is_up(&pNetwork->EthernetInterface)) 
    {
        SNetwork_Poll(pNetwork);

        // Stop program if we get a timeout on DHCP attempt.
        if (Timeout.read_ms() > 10000) 
        {
            printf("error: Timeout while waiting for network to initialize.\r\n");
            goto Error;
        }
    }

    // Print out the DHCP provided IP addresses.
    printf("IP     : "); 
    SNetwork_PrintAddress(&pNetwork->EthernetInterface.ip_addr);
    printf("\r\n");

    printf("Gateway: "); 
    SNetwork_PrintAddress(&pNetwork->EthernetInterface.gw);
    printf("\r\n");

    printf("Mask   : "); 
    SNetwork_PrintAddress(&pNetwork->EthernetInterface.netmask);
    printf("\r\n");
    
    Return = 0;
Error:
    return Return;
}


/* Called from within the main application loop to perform network maintenance
   tasks at appropriate time intervals and execute the ethernet driver which 
   will push new network packets into the lwIP stack.  lwIP will then call into
   the callbacks which have been configured by this application.
   
   Parameters:
    pNetwork is a pointer to the intitialized network object.
    
   Returns:
    Number of network tasks run during this polling process.  If 0 then no
        network related tasks were completed during this polling iteration.
*/
int SNetwork_Poll(SNetwork* pNetwork)
{
    int TasksRun = 0;
    
    // Validate parameters.
    assert ( pNetwork );
    
    // Execute timed maintenance tasks.
    if (pNetwork->DHCPFine.read_ms() >= DHCP_FINE_TIMER_MSECS)
    {
        pNetwork->DHCPFine.reset();
        dhcp_fine_tmr();
        TasksRun++;
    }
    if (pNetwork->DHCPCoarse.read() >= DHCP_COARSE_TIMER_SECS)
    {
        pNetwork->DHCPCoarse.reset();
        dhcp_coarse_tmr();
        TasksRun++;
    }
    if (pNetwork->ARP.read_ms() >= ARP_TMR_INTERVAL)
    {
        pNetwork->ARP.reset();
        etharp_tmr();
        TasksRun++;
    }
    if (pNetwork->TCP.read_ms() >= TCP_TMR_INTERVAL)
    {
        pNetwork->TCP.reset();
        tcp_tmr();
        TasksRun++;
    }
    if (pNetwork->DNS.read_ms() >= DNS_TMR_INTERVAL)
    {
        pNetwork->DNS.reset();
        dns_tmr();
        TasksRun++;
    }

    // Poll the ethernet driver to let it pull packets in and push new packets
    // into the lwIP network stack.
    TasksRun += eth_poll();
    
    return TasksRun;
}


/* Prints out the caller supplied IP address.

   Parameters:
    pAddress is a pointer to the ip4 address to be displayed.
    
   Returns:
    Nothing.
*/
void SNetwork_PrintAddress(ip_addr_t* pAddress)
{
    // Validate parameters.
    assert ( pAddress );
    
    printf("%d.%d.%d.%d", 
           ip4_addr1(pAddress),
           ip4_addr2(pAddress),
           ip4_addr3(pAddress),
           ip4_addr4(pAddress));
}
