#ifndef _GPIO_PIN_H_
#define _GPIO_PIN_H_

/* Definicion de pines de central de domotica */
#define GPIO_IO1 27
#define GPIO_IO2 28
#define GPIO_IO3 3
#define GPIO_IO4 5
#define GPIO_IO5 7
#define GPIO_IO6 29
#define GPIO_IO7 31
#define GPIO_IO8 26
#define GPIO_EX1 36
#define GPIO_EX2 11
#define GPIO_EX3 12
#define GPIO_EX4 35
#define GPIO_EX5 38
#define GPIO_EX6 40
#define GPIO_EX7 15
#define GPIO_EX8 16

#define GPIO_POWER_5V 18
#define GPIO_STATUS_LED 24

#define GPIO_TX_MODEM_RX 8
#define GPIO_RX_MODEM_TX 10
#define GPIO_MODEM_POWER_SET 21
#define GPIO_MODEM_POWER_GET 19
#define GPIO_MODEM_RESET 23
#define GPIO_MODEM_RING 32
#define GPIO_MODEM_PWRKEY 33

/* Conversión de pines entre diagrama de KiCAD y Librería wiringPi */
int gpio_pin[] = {-1,-1,-1,2,-1,3,-1,4,14,-1,15,17,28,27,-1,22,23,-1,24,10,-1,9,25,11,8,-1,7,0,1,5,-1,6,12,13,-1,19,16,26,20,-1,21};




#endif /* _GPIO_PIN_H_ */