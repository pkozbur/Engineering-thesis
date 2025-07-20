/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hagl.h"
#include "font6x9.h"
#include "rgb565.h"
#include <stdio.h>
#include "ds18b20.h"
#include <wchar.h>
#include "mfrc522.h"
#include <string.h>
#include "lps25hb.h"
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

float voltage;
float temp;
float templps25TempAmount;
float templps25PressureAmount;
char str1[100];
uint8_t cardstr[20];
uint8_t status = 0;
char buffer[50];


typedef struct {
    uint8_t id[5];
    bool hasEntered;
} Person;

int people_in_building = 0;

Person registered_people[] = {
    {{0xA3, 0xBD, 0x45, 0x90, 0xCB}, false},  // Person 1
    {{0xC1, 0xFC, 0x71, 0x1C, 0x50}, false}   // Person 2
};

int find_person_by_card(uint8_t *cardstr) {
    for (int i = 0; i < sizeof(registered_people) / sizeof(Person); i++) {
        int match = 1;
        for (int j = 0; j < 5; j++) {
            if (cardstr[j] != registered_people[i].id[j]) {
                match = 0;
                break;
            }
        }
        if (match) {
            return i;
        }
    }
    return -1;
}

void process_card_scan(uint8_t *cardstr) {
    int person_index = find_person_by_card(cardstr);

    // If the card is registered
    if (person_index != -1) {
        Person *person = &registered_people[person_index];  // Get the person details

        // Check if the person has entered or exited
        if (person->hasEntered) {
            // Person exits
            if (people_in_building > 0) {
                people_in_building--;
                printf("Person exited. People in building: %d\n", people_in_building);
            }
            person->hasEntered = false;  // Toggle to exit status
        } else {
            // Person enters
            people_in_building++;
            printf("Person entered. People in building: %d\n", people_in_building);
            person->hasEntered = true;   // Toggle to enter status
        }
    } else {
        printf("Unregistered card scanned!\n");
    }
}

int __io_putchar(int ch)
{
  if (ch == '\n') {
    __io_putchar('\r');
  }

  HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);

  return 1;
}

void ConvertFloatToWchar(float value, wchar_t* wbuffer, size_t buffer_size) {
    swprintf(wbuffer, buffer_size, L"%.1f °C", value);
}
void ConvertFloatToWchar2(float value, wchar_t* wbuffer, size_t buffer_size) {
    swprintf(wbuffer, buffer_size, L"Temperatura %.1f °C", value);
}
void ConvertFloatToWchar3(float value, wchar_t* wbuffer, size_t buffer_size) {
    swprintf(wbuffer, buffer_size, L"Ciśnienie %.1f hPa", value);
}

void send_data(float temp, uint8_t *cardstr) {
    uint8_t tx_buffer[24];

    memcpy(tx_buffer, &temp, sizeof(float));
    memcpy(tx_buffer + 4, cardstr, 20);

    HAL_UART_Transmit(&huart1, tx_buffer, sizeof(tx_buffer), HAL_MAX_DELAY);
}


void send_telemetry_to_esp32(float temperature, float pressure, float light_level, int people_count) {
    char message[100];
    sprintf(message, "TELE:%.2f,%.2f,%.2f,%d\n", temperature, pressure, light_level, people_count);
    HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
}

void send_rfid_to_esp32(float temperature, uint8_t *card_info) {
    char message[100];
    sprintf(message, "RFID:%.2f,%02X%02X%02X%02X%02X\n", temperature, card_info[0], card_info[1], card_info[2], card_info[3], card_info[4]);

    if (HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), HAL_MAX_DELAY) != HAL_OK) {
        printf("UART Transmit failed for RFID data\n");
    } else {
        printf("RFID data sent: %s\n", message);  // Debug print to console
    }
}

int overflow_count = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim7) {
        overflow_count++;

        if (overflow_count >= 8) {  // After 8 overflows (~60 seconds)
            overflow_count = 0;
            send_telemetry_to_esp32(templps25TempAmount, templps25PressureAmount, voltage, people_in_building);// Send data to ESP32
        }
    }
}


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */




/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_SPI2_Init();
  MX_RTC_Init();
  MX_SPI3_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
  MFRC522_Init(&hspi3);
  HAL_ADC_Start(&hadc1);
  HAL_TIM_Base_Start_IT(&htim7);




  if (ds18b20_init() != HAL_OK) {
    Error_Handler();
  }

  uint8_t ds1[DS18B20_ROM_CODE_SIZE];

  if (ds18b20_read_address(ds1) != HAL_OK) {
    Error_Handler();
  }
  lcd_init();

  for (int i = 0; i < 8; i++) {
  	    hagl_draw_rounded_rectangle(2+i, 2+i, 158-i, 126-i, 8-i, rgb565(100, 0, i*16));
  }

  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  lps25hb_init();
	  ds18b20_start_measure(NULL);
	  HAL_Delay(750);
	  if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK){
	  	uint32_t LDR_Value = HAL_ADC_GetValue(&hadc1);
	  	voltage = 100 - (LDR_Value * 0.0008f / 2.5f * 100);
	  	sprintf(buffer, "Poziom natężenia oświetlenia: %f\r\n", voltage);
	  	HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	  }
	    temp = ds18b20_get_temp(NULL);
	    templps25TempAmount = lps25hb_read_temp();
	    templps25PressureAmount = lps25hb_read_pressure();
	    wchar_t wbuffer[20];
	    wchar_t wbufferlps25hpTemp[20];
	    wchar_t wbufferlps25hpPress[21];
	    ConvertFloatToWchar(temp, wbuffer, sizeof(wbuffer) / sizeof(wchar_t));
	    ConvertFloatToWchar2(templps25TempAmount, wbufferlps25hpTemp, sizeof(wbufferlps25hpTemp) / sizeof(wchar_t));
	    ConvertFloatToWchar3(templps25PressureAmount, wbufferlps25hpPress, sizeof(wbufferlps25hpPress) / sizeof(wchar_t));
	    if (temp >= 80.0f){
	      printf(" DS18 sensor error...\n");
	    }
	    else{
	    	hagl_put_text(wbuffer, 10, 15, YELLOW, font6x9);
	    	hagl_put_text(L"Na zewnątrz", 10, 90, YELLOW, font6x9);
	    	hagl_put_text(wbufferlps25hpTemp, 10, 100, YELLOW, font6x9);
	    	hagl_put_text(wbufferlps25hpPress, 10, 110, YELLOW, font6x9);
	    	lcd_copy();
	    }
	    status = MFRC522_Request(PICC_REQIDL, &(cardstr[0]));
	   	if (status == MI_OK) {
	   	  	status = MFRC522_Anticoll(&(cardstr[0]));
	   	  	if (status == MI_OK) {
	   	  		sprintf(str1, "Card:%x,%x,%x,%x,%x\r\n", cardstr[0], cardstr[1], cardstr[2], cardstr[3], cardstr[4]);
	   	  		process_card_scan(cardstr);
	   	  		HAL_UART_Transmit(&huart2, str1, strlen(str1), 500);
	   	  				if(cardstr[0] == 0xc1){
	   	  					//hagl_put_text(L"Witaj Patryk", 45, 55, YELLOW, font6x9);
	   	  					if(registered_people[1].hasEntered == true){
	   	  						hagl_put_text(L"Logowanie", 45, 55, YELLOW, font6x9);
	   	  					}
	   	  					else{
	   	  						hagl_put_text(L"Wylogowanie", 45, 55, YELLOW, font6x9);

	   	  					}
	   	  					lcd_copy();
	   	  					send_rfid_to_esp32(temp, cardstr);
	   	  					HAL_Delay(2000);
	   	  					hagl_clear_screen();
	   	  					for (int i = 0; i < 8; i++) {
	   	  						hagl_draw_rounded_rectangle(2+i, 2+i, 158-i, 126-i, 8-i, rgb565(100, 0, i*16));
	   	  					}
	   	  					lcd_copy();
	   	  				}
	   	  				if(cardstr[0] == 0xa3){
	   	  					hagl_put_text(L"Witaj Mariusz", 45, 55, YELLOW, font6x9);
	   	  					lcd_copy();
	   	  					send_rfid_to_esp32(temp, cardstr);
	   	  					HAL_Delay(1000);
	   	  					hagl_clear_screen();
	   	  					for (int i = 0; i < 8; i++) {
	   	  						hagl_draw_rounded_rectangle(2+i, 2+i, 158-i, 126-i, 8-i, rgb565(100, 0, i*16));
	   	  					}
	   	  					lcd_copy();
	   	  				}
	   	  			}
	   	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
