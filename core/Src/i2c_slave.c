#include "i2c_slave.h"

extern I2C_HandleTypeDef hi2c1;   // defined in main.c by CubeMX

static uint8_t i2c_tx_buffer[5];  // private to this file — nothing outside can touch it directly

void I2C_Slave_Init(void)
{
  HAL_I2C_Slave_Transmit_IT(&hi2c1, i2c_tx_buffer, sizeof(i2c_tx_buffer));
  printf("I2C slave armed\r\n");
}

void I2C_Slave_UpdateBuffer(uint8_t humidity_int, uint8_t humidity_dec,
                             uint8_t temp_int, uint8_t temp_dec, uint8_t checksum)
{
  i2c_tx_buffer[0] = humidity_int;
  i2c_tx_buffer[1] = humidity_dec;
  i2c_tx_buffer[2] = temp_int;
  i2c_tx_buffer[3] = temp_dec;
  i2c_tx_buffer[4] = checksum;
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C1)
  {
    printf("I2C slave TX complete\r\n");
    HAL_I2C_Slave_Transmit_IT(&hi2c1, i2c_tx_buffer, sizeof(i2c_tx_buffer));
  }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C1)
  {
    printf("I2C slave ERROR, code=%lu\r\n", HAL_I2C_GetError(hi2c));
    HAL_I2C_Slave_Transmit_IT(&hi2c1, i2c_tx_buffer, sizeof(i2c_tx_buffer));
  }
}
