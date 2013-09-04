// Sample code taken from http://mbed.org/handbook/USBHostKeyboard
#include "mbed.h"
#include "USBHostKeyboard.h"
 
DigitalOut led(LED1);
 
void onKey(uint8_t key) {
    printf("Key: %c\r\n", key);
}
 
void keyboard_task(void const *) {
    
    USBHostKeyboard keyboard;
    
    while(1) {
        // try to connect a USB keyboard
        while(!keyboard.connect())
            Thread::wait(500);
    
        // when connected, attach handler called on keyboard event
        keyboard.attach(onKey);
        
        // wait until the keyboard is disconnected
        while(keyboard.connected())
            Thread::wait(500);
    }
}
 
int main() {
    Thread keyboardTask(keyboard_task, NULL, osPriorityNormal, 256 * 4);
    while(1) {
        led=!led;
        Thread::wait(500);
    }
}
