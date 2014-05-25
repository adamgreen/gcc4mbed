// Sample code taken from http://mbed.org/handbook/USBMouse
#include "mbed.h"
#include "USBMouse.h"
 
USBMouse mouse;
 
int main() 
{
    int16_t x = 0;
    int16_t y = 0;
    int32_t radius = 10;
    int32_t angle = 0;
 
    while (1) 
    {
        x = cos((double)angle*3.14/180.0)*radius;
        y = sin((double)angle*3.14/180.0)*radius;
        
        mouse.move(x, y);
        angle += 3;
        wait(0.001);
    }
}
