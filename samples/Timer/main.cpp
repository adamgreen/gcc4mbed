#include "mbed.h"
#include "rtos.h"

DigitalOut LEDs[4] = {
    DigitalOut(LED1), DigitalOut(LED2), DigitalOut(LED3), DigitalOut(LED4)
};

void print_char() {
    static int counter = 0;
    printf("%d\r\n", ++counter);
    fflush(stdout);
}

void blink(void const *n) {
    static int counter = 0;
    const int led_id = int(n);
    LEDs[led_id] = !LEDs[led_id];
    if (++counter == 75) {
        print_char();
        counter = 0;
    }
}

int main(void) {
    RtosTimer led_1_timer(blink, osTimerPeriodic, (void *)0);
    RtosTimer led_2_timer(blink, osTimerPeriodic, (void *)1);
    RtosTimer led_3_timer(blink, osTimerPeriodic, (void *)2);
    RtosTimer led_4_timer(blink, osTimerPeriodic, (void *)3);

    led_1_timer.start(200);
    led_2_timer.start(100);
    led_3_timer.start(50);
    led_4_timer.start(25);

    Thread::wait(osWaitForever);
}
