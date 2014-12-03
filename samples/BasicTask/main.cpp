#include "mbed.h"
#include "rtos.h"

void print_char()
{
    static int count = 0;
    printf("%d\r\n", ++count);
    fflush(stdout);
}

DigitalOut led1(LED1);
DigitalOut led2(LED2);

void led2_thread(void const *argument) {
    while (true) {
        led2 = !led2;
        Thread::wait(1000);
        print_char();
    }
}

int main() {
    Thread thread(led2_thread);

    while (true) {
        led1 = !led1;
        Thread::wait(500);
    }
}
