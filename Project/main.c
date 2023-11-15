#include "msp.h"
#include "Clock.h"
#include <stdio.h>

#define LED_RED 1
#define LED_GREEN (LED_RED << 1)
#define LED_BLUE (LED_RED << 2)
#define LED_ALL LED_RED | LED_GREEN | LED_BLUE

void led_init()
{
    //Set P2 as GPIO
    P2->SEL0 &= ~0x07;
    P2->SEL1 &= ~0x07;

    //Input or Output
    // CUrrent type is output
    P2->DIR |= 0x07;

    //Turn off LED
    P2->OUT &= ~0x07;
}

void turn_on_led(int color)
{
    P2->OUT &= ~0x07;
    P2->OUT |= color;
}


void turn_off_led()
{
    P2->OUT &= ~0x07;
}

void switch_init()
{
    //Setup switch as GPIO
    P1->SEL0 &= ~0x12;
    P1->SEL1 &= ~0x12;

    //Setup switch as Input
    P1->DIR &= ~0x12;

    //Enable pull-up resistors
    P1->REN |= 0x12;

    //Now pull-up
    P1->OUT |= 0x12;
}

void systick_init() {
    SysTick->LOAD = 0x00FFFFFF;
    SysTick->CTRL = 0x00000005;
}

void systick_wait1ms() {
    SysTick->LOAD = 0xBB80;
    SysTick->VAL = 0;
    while((SysTick->CTRL & 0x00010000) == 0) {};
}

void systick_wait1s() {
    int i;
    int count = 1000;

    for (i = 0; i < count; i++) {
        systick_wait1ms();
    }
}

void pwm_init34(uint16_t period, uint16_t duty3, uint16_t duty4){

    TIMER_A0->CCR[0] = period;

    TIMER_A0->EX0 = 0x0000;

    TIMER_A0->CCTL[3] = 0x0040;
    TIMER_A0->CCR[3] = duty3;
    TIMER_A0->CCTL[4] = 0x0040;
    TIMER_A0->CCR[4] = duty4;

    TIMER_A0->CTL = 0x02F0;

    P2->DIR |= 0xC0;
    P2->SEL0 |= 0xC0;
    P2->SEL1 &= ~0xC0;

}

void sensor_init() {
    P5->SEL0 &= ~0x08;
    P5->SEL1 &= ~0x08;
    P5->DIR |= 0x08;
    P5->OUT &= ~0x08;

    P9->SEL0 &= ~0x04;
    P9->SEL1 &= ~0x04;
    P9->DIR |= 0x04;
    P9->OUT &= ~0x04;

    P7->SEL0 &= ~0xFF;
    P7->SEL1 &= ~0xFF;
    P7->DIR &= ~0xFF;
}

void motor_init(void) {
    P3->SEL0 &= ~0xC0;
    P3->SEL1 &= ~0xC0;
    P3->DIR |= 0xC0;
    P3->OUT &= ~0xC0;

    P5->SEL0 &= ~0x30;
    P5->SEL1 &= ~0x30;
    P5->DIR |= 0x30;
    P5->OUT &= ~0x30;

    P2->SEL0 &= ~0xC0;
    P2->SEL1 &= ~0xC0;
    P2->DIR |= 0xC0;
    P2->OUT &= ~0xC0;

    pwm_init34(7500,0,0);
}

void move_forward(int speed) {
    P5->OUT &= ~0x30;   //PH = 0
    P2->OUT |= 0xC0;    //EN = 1
    P3->OUT |= 0xC0;    //nSleep = 1
    Clock_Delay1us(speed);

    P2->OUT &= ~0xC0;
    Clock_Delay1us(10000-speed);

}

void move(uint16_t leftDuty, uint16_t rightDuty){
    P3->OUT |= 0xC0;
    TIMER_A0->CCR[3] = leftDuty;
    TIMER_A0->CCR[4] = rightDuty;
}

void left_forward(){
    P5->OUT &= ~0x10;
}
void left_backward(){
    P5->OUT |= ~0x10;
}
void right_forward(){
    P5->OUT &= ~0x20;
}
void right_backward(){
    P5->OUT |= 0x20;
}

void stop(void) {
    P2->OUT &= ~0xC0;
}

//void(*TimerA2Task)(void);
//void TimerA2_Init(void(*task)(void), uint16_t period){
//    TimerA2Task = task;
//    TIMER_A2->CTL=0x0280;
//    TIMER_A2->CCTL[0]=0x0010;
//    TIMER_A2->CCR[0]=(period-1);
//    TIMER_A2->EX0=0x0005;
//    NVIC->IP[3]=(NVIC->IP[3]&0xFFFFFF00) | 0x00000040;
//    NVIC->ISER[0]=0x00001000;
//    TIMER_A2->CTL |= 0x0014;
//}
//
//void TA2_0_IRQHandler(void){
//    TIMER_A2->CCTL[0] &= ~0x0001;
//    (*TimerA2Task)();
//}

//void task(){
//    printf("interrupt");
//}

void timer_A3_capture_init(){
    P10->SEL0 |= 0x30;
    P10->SEL1 &= ~0x30;
    P10->DIR &= ~0x30;

    TIMER_A3->CTL &= ~0x0030;
    TIMER_A3->CTL = 0x0200;

    TIMER_A3->CCTL[0] = 0x4910;
    TIMER_A3->CCTL[1] = 0x4910;
    TIMER_A3->EXO &= ~0x0007;

    NVIC->IP[3]=(NVIC->IP[3]&0x0000FFFF)| 0x40400000;
    NVIC->ISER[0]=0x0000C000;
    TIMER_A3->CTL|=0x0024;
}

uint16_t first_left;
uint16_t first_right;

uint16_t period_left;
uint16_t period_right;

void TA3_0_IRQHandler(void){
    TIMER_A3->CCTL[0] &=~0x0001;
    period_right = TIMER_A3->CCR[0]-first_right;
    first_right = TIMER_A3->CCR[0];
}

void TA3_N_IRQHandler(void) {
    TIMER_A3->CCTL[1] &= ~0x0001;
    period_left = TIMER_A3->CCR[1] - first_left;
    first_left = TIMER_A3->CCR[1];
}

uint32_t get_left_rpm() {
    return 2000000 / period_left;
}

uint32_t left_count;
void TA3_N_IRQHandler(void){
    TIMER_A3->CCTL[1] &= ~0x0001;
    left_count++;
}

int main(void)
{

    Clock_Init48MHz();
    led_init();
    switch_init();
    systick_init();
    sensor_init();
    motor_init();

    while(1) {

        //Turn on IR LEDs
        P5->OUT |= 0x08;
        P9->OUT |= 0x04;

        // Make P7.0 ~P7.7 as output
        P7->DIR = 0xFF;
        //Charges a capacitor
        P7->OUT = 0xFF;
        //Wait for fully charged
        Clock_Delay1us(10);

        //Make P7.0 ~P7.7 as input
        P7->DIR = 0x00;
        //Wait for a while
        Clock_Delay1us(1000);

        //Read
        if ((P7->IN & 0xFF) == 0x18) {
            left_forward();
            right_forward();
            move(500,500);
            systick_wait1s();
        } else if ((P7->IN & 0xFF) > 0x18) {
            left_forward();
            right_backward();
            if ((P7->IN & 0xFF) > 0x60){
                move(900,900);
            } else if ((P7->IN & 0xFF) < 0x60){
                move(600,600);
            }
            systick_wait1s();
        } else if ((P7->IN & 0xFF) < 0x18) {
            left_backward();
            right_forward();
            if ((P7->IN & 0xFF) < 0x06){
                move(900,900);
            } else if ((P7->IN & 0xFF) > 0x06){
                move(600,600);
            }
            systick_wait1s();
        }
    }

//    Clock_Init48MHz();
//    timer_A3_capture_init();
//
//    while(1){
//        left_forward();
//        right_forward();
//        move(100,100);
//        if (left_count > 180) {
//            move(0,0);
//
//        }
//    };




}
