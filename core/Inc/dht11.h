#ifndef DHT11_H
#define DHT11_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"

#define DHT11_PORT  GPIOB
#define DHT11_PIN   GPIO_PIN_0

typedef struct {
    uint8_t humidity_int;
    uint8_t humidity_dec;
    uint8_t temp_int;
    uint8_t temp_dec;
    uint8_t checksum;
} DHT11_Data;

void DHT11_Init(void);
void DHT11_SetPinOutput(GPIO_TypeDef *GPIOx, uint16_t pin);
void DHT11_SetPinInput(GPIO_TypeDef *GPIOx, uint16_t pin);
uint8_t DHT11_Read(DHT11_Data *data);

#endif // DHT11_H
