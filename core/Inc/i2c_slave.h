#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H

#include "stm32f4xx_hal.h"

void I2C_Slave_Init(void);
void I2C_Slave_UpdateBuffer(uint8_t humidity_int, uint8_t humidity_dec,
                             uint8_t temp_int, uint8_t temp_dec, uint8_t checksum);

#endif
