// Sample code taken from http://mbed.org/handbook/USBHostHub
#include "mbed.h"
#include "USBHostKeyboard.h"
#include "USBHostMouse.h"
 
DigitalOut led(LED1);
 
void onKey(uint8_t key) {
    printf("Key: %c\r\n", key);
}
 
void onMouseEvent(uint8_t buttons, int8_t x, int8_t y, int8_t z) {
    printf("buttons: %d, x: %d, y: %d, z: %d\r\n", buttons, x, y, z);
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
 
void mouse_task(void const *) {
    
    USBHostMouse mouse;
    
    while(1) {
        // try to connect a USB mouse
        while(!mouse.connect())
            Thread::wait(500);
    
        // when connected, attach handler called on mouse event
        mouse.attachEvent(onMouseEvent);
        
        // wait until the mouse is disconnected
        while(mouse.connected())
            Thread::wait(500);
    }
}
 
int main() {
    Thread keyboardTask(keyboard_task, NULL, osPriorityNormal, 256 * 4);
    Thread mouseTask(mouse_task, NULL, osPriorityNormal, 256 * 4);
    while(1) {
        led=!led;
        Thread::wait(500);
    }
}
