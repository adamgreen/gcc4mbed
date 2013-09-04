// Sample code taken from http://mbed.org/handbook/USBHostSerial
#include "mbed.h"
#include "USBHostSerial.h"
 
DigitalOut led(LED1);
 
void serial_task(void const*) {
    USBHostSerial serial;
    
    while(1) {
    
        // try to connect a serial device
        while(!serial.connect())
            Thread::wait(500);
        
        // in a loop, print all characters received
        // if the device is disconnected, we try to connect it again
        while (1) {
        
            // if device disconnected, try to connect it again
            if (!serial.connected())
                break;
 
            // print characters received
            while (serial.available()) {
                printf("%c", serial.getc());
            }
            
            Thread::wait(50);
        }
    }
}
 
int main() {
    Thread serialTask(serial_task, NULL, osPriorityNormal, 256 * 6);
    while(1) {
        led=!led;
        Thread::wait(500);
    }
}
