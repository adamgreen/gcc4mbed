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
/* Header file for core lwIP ethernet functionality. */
#ifndef NETWORK_H_
#define NETWORK_H_

#include <mbed.h>
#include "lwip/dhcp.h"



// Structure which stores information about the network stack.
struct SNetwork
{
    // Timers used to determine when various network maintenance tasks should
    // be executed.
    Timer           ARP;
    Timer           DHCPCoarse;
    Timer           DHCPFine;
    Timer           TCP;
    Timer           DNS;
    // Object used to track the DHCP state.
    dhcp            DHCP;
    // The structure used to represent the LPC17xx polled Ethernet interface.
    netif           EthernetInterface;
};


// Function prototypes.
int      SNetwork_Init(SNetwork*   pNetwork,
                       const char* pIPAddress,
                       const char* pSubnetMask,
                       const char* pGatewayAddress,
                       const char* pHostName);
int     SNetwork_Poll(SNetwork* pNetwork);
void    SNetwork_PrintAddress(ip_addr_t* pAddress);



#endif /* NETWORK_H_ */
