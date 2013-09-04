// Sample code taken from http://mbed.org/handbook/USBHostMSD
#include "mbed.h"
#include "USBHostMSD.h"
 
DigitalOut led(LED1);
 
void msd_task(void const *)
{
    
    USBHostMSD msd("usb");
    int i = 0;
    
    while(1)
    {
        // try to connect a MSD device
        while(!msd.connect()) 
        {
            Thread::wait(500);
        }
        
        // in a loop, append a file
        // if the device is disconnected, we try to connect it again
        while(1) 
        {
            // append a file
            FILE * fp = fopen("/usb/test1.txt", "a");
        
            if (fp != NULL)
            {
                fprintf(fp, "Hello fun SD Card World: %d!\r\n", i++);
                printf("Goodbye World!\r\n");
                fclose(fp);
            }
            else
            {
                printf("FILE == NULL\r\n");
            }
            
            Thread::wait(500);
        
            // if device disconnected, try to connect again
            if (!msd.connected())
                break;
        }
    }
}
 
 
int main()
{
    Thread msdTask(msd_task, NULL, osPriorityNormal, 1024 * 4);
    while(1)
    {
        led=!led;
        Thread::wait(500);
    }
}