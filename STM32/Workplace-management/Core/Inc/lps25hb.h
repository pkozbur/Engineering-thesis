#pragma once
#include "stm32l4xx.h"


HAL_StatusTypeDef lps25hb_init(void);
float lps25hb_read_temp(void);
float lps25hb_read_pressure(void);
