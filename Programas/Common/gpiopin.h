#ifndef _GPIO_PIN_H_
#define _GPIO_PIN_H_

/* Definicion de pines de central de domotica */
#define GPIO_IO1 40 /* <-- Nro de PIN */
#define GPIO_IO2 38
#define GPIO_IO3 36
#define GPIO_IO4 32
#define GPIO_IO5 28
#define GPIO_IO6 26
#define GPIO_IO7 24
#define GPIO_IO8 22

#define GPIO_OUT1 5
#define GPIO_OUT2 7

#define GPIO_EXP1_1 13
#define GPIO_EXP1_2 11
#define GPIO_EXP1_3 19
#define GPIO_EXP1_4 15
#define GPIO_EXP1_5 23
#define GPIO_EXP1_6 21
#define GPIO_EXP1_7 29
#define GPIO_EXP1_8 27

#define GPIO_STATUS_LED 16
#define GPIO_MODE_LED 18

#define GPIO_TX 8
#define GPIO_RX 10

/* Conversión de pines entre diagrama de KiCAD y Librería wiringPi (index = pin, value = GPIO) */
int gpio_pin[] = {-1,-1,-1,2,-1,3,-1,4,14,-1,15,17,28,27,-1,22,23,-1,24,10,-1,9,25,11,8,-1,7,0,1,5,-1,6,12,13,-1,19,16,26,20,-1,21};




#endif /* _GPIO_PIN_H_ */