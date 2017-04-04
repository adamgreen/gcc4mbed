#include "mbed.h"
#include "EthernetInterface.h"

int main() {
    EthernetInterface eth;
    eth.set_dhcp(true);
    eth.connect();
    printf("IP Address is %s\n", eth.get_ip_address());
    
    TCPSocket sock(&eth);
    sock.connect("mbed.org", 80);
    
    char http_cmd[] = "GET /media/uploads/mbed_official/hello.txt HTTP/1.0\n\n";
    sock.send(http_cmd, sizeof(http_cmd)-1);
    
    char buffer[300];
    int ret;
    while (true) {
        ret = sock.recv(buffer, sizeof(buffer)-1);
        if (ret <= 0)
            break;
        buffer[ret] = '\0';
        printf("Received %d chars from server:\n%s\n", ret, buffer);
    }
      
    sock.close();
    
    eth.disconnect();
    
    while(1) {}
}
