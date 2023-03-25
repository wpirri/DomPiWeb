#ifndef _GPIO_PIN_H_
#define _GPIO_PIN_H_

/* Definicion de pines de central de domotica */
#define GPIO_A01 40 /* <-- Nro de PIN */
#define GPIO_A02 38
#define GPIO_A03 36
#define GPIO_A04 32
#define GPIO_A05 28
#define GPIO_A06 26
#define GPIO_A07 24
#define GPIO_A08 22

#define GPIO_B01 5
#define GPIO_B02 7
#define GPIO_B03 0
#define GPIO_B04 0
#define GPIO_B05 0
#define GPIO_B06 0
#define GPIO_B07 0
#define GPIO_B08 0

#define GPIO_C01 13
#define GPIO_C02 11
#define GPIO_C03 19
#define GPIO_C04 15
#define GPIO_C05 23
#define GPIO_C06 21
#define GPIO_C07 29
#define GPIO_C08 27

#define GPIO_STATUS_LED 16
#define GPIO_MODE_LED 18

#define GPIO_TX 8
#define GPIO_RX 10

/* Conversión de pines entre diagrama de KiCAD y Librería wiringPi (index = pin, value = GPIO) */
int gpio_pin[] = {-1,-1,-1,2,-1,3,-1,4,14,-1,15,17,28,27,-1,22,23,-1,24,10,-1,9,25,11,8,-1,7,0,1,5,-1,6,12,13,-1,19,16,26,20,-1,21};




#endif /* _GPIO_PIN_H_ */