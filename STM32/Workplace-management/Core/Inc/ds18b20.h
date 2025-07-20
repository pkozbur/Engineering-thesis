#pragma once

#include "stm32l4xx.h"

#define DS18B20_ROM_CODE_SIZE		8

HAL_StatusTypeDef ds18b20_init(void);
HAL_StatusTypeDef ds18b20_read_address(uint8_t* rom_code);
HAL_StatusTypeDef ds18b20_start_measure(const uint8_t* rom_code);
float ds18b20_get_temp(const uint8_t* rom_code);
