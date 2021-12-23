/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "kb.h"
#include "sound_driver.h"
#include <stdio.h>
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define BUFFER_SIZE 32

uint32_t notes[] = {262, 294, 330, 349, 392, 440, 494};
int all_notes_playing = 0;
int current_note = -1;

uint8_t buffer[BUFFER_SIZE];
size_t write_pointer = 0;
size_t read_pointer = 0;

int mode = 1;
int short_press_time = 200;
int is_pressed_button = 0;
long start_time_pressed_button = 0;
int octave = 0;
int duration = 1000;
long note_start_time = 0;
char message[150] = "";

void buffer_init() {
    for (size_t i = 0; i < BUFFER_SIZE; ++i)
        buffer[i] = 0;
}

void buffer_add(uint8_t num) {
    buffer[write_pointer] = num;
    write_pointer = (write_pointer + 1) % BUFFER_SIZE;
}

int buffer_read() {
    if (read_pointer == write_pointer) {
        return -1;
    }
    uint8_t num = buffer[read_pointer];
    read_pointer = (read_pointer + 1) % BUFFER_SIZE;
    return num;
}

void keyboard_read(void) {
    static uint8_t const rows[4] = {0x1E, 0x3D, 0x7B, 0xF7};
    static int current_row = 0;
    static int row_result[4] = {0, 0, 0, 0};

    if (ks_state == 0) {
        if (row_result[current_row] != ks_result) {
            uint8_t keyNum = 0;
            if (ks_result & 1) {
                buffer_add(3 * current_row + 3);
            }
            if (ks_result & 2) {
                buffer_add(3 * current_row + 2);
            }
            if (ks_result & 4) {
                buffer_add(3 * current_row + 1);
            }
        }

        row_result[current_row] = ks_result;
        current_row = (current_row + 1) % 4;
        ks_current_row = rows[current_row];
        ks_continue();
    }
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c == &hi2c1 && ks_state) {
        ks_continue();
    }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c == &hi2c1 && ks_state) {
        ks_continue();
    }
}

typedef enum {
    INPUT_PORT = 0x00, //Read byte XXXX XXXX
    OUTPUT_PORT = 0x01, //Read/write byte 1111 1111
    POLARITY_INVERSION = 0x02, //Read/write byte 0000 0000
    CONFIG = 0x03 //Read/write byte 1111 1111
} pca9538_regs_t;



void send_message(char *message) {
    int size = strlen(message);
    if (message[0] == '\r'){
        send_message("\n");
    }
    HAL_UART_Transmit(&huart6, (uint8_t *) message, size, 100);
}

long getCurrentTime(){
    return HAL_GetTick();
}

void check_pressed_button(){
    if (!check_button()) {
        if (!is_pressed_button) {
            start_time_pressed_button = getCurrentTime();
            is_pressed_button = 1;
        }
    } else {
        if (is_pressed_button && getCurrentTime() - start_time_pressed_button > short_press_time) {
            if (mode == 1) {
                mode = 0;
                send_message("Run mode\r\n");
            } else {
                mode = 1;
                send_message("Debug mode\r\n");
            }
            sound_driver_volume_mute();
        }
        is_pressed_button = 0;
    }
}

void handle_note(int pressed_key){
    current_note = pressed_key - 1;
    all_notes_playing = 0;
    note_start_time = getCurrentTime();
    snprintf(message, sizeof(message), "Current note: %d\r\n", pressed_key);
    send_message(message);
}

void handle_octave(int value){
    if (octave == -4 && value < 0 || octave == 4 && value > 0)
        return;
    octave += value;
    snprintf(message, sizeof(message), "Current octave: %d\r\n", octave);
    send_message(message);
}

void handle_duration(int value){
    if (duration == 100 && value < 0 || duration == 10000 && value > 0)
        return;
    duration += value;
    snprintf(message, sizeof(message), "Current duration: %d\r\n", duration);
    send_message(message);
}

void handle_key(int pressed_key) {
    if (pressed_key < 1 || pressed_key > 12)
        return;
    switch (pressed_key) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            handle_note(pressed_key);
            break;
        case 8:
            handle_octave(1);
            break;
        case 9:
            handle_octave(-1);
            break;
        case 10:
            handle_duration(100);
            break;
        case 11:
            handle_duration(-100);
            break;
        case 12:
            all_notes_playing = 1;
            current_note = 0;
            note_start_time = getCurrentTime();
    }
}

void all_notes_next(){
    if (all_notes_playing) {
        if (current_note == 6) {
            current_note == -1;
            all_notes_playing = 0;
            return;
        }
        current_note += 1;
        note_start_time = getCurrentTime();
    }
}

uint32_t calc_frequency(){
	uint32_t freq = 0;
    if (current_note < 0)
        return 1;
    if (octave >= 0){
    	freq = notes[current_note] << octave;
    } else {
    	freq = notes[current_note] >> -octave;
    }
    return freq;
}
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
  MX_USART6_UART_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  buffer_init();
  sound_driver_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  check_pressed_button();

	  keyboard_read();
	  int key_pressed = buffer_read();

	          if (mode) {
	              if (key_pressed > 0 && key_pressed < 13) {
	                  snprintf(message, sizeof(message), "Pressed key: %d\r\n", key_pressed);
	                  send_message(message);
	              }
	              continue;
	          }

	          handle_key(key_pressed);

	          if (current_note == -1){
	              note_start_time = getCurrentTime();
	          } else {
	              if (getCurrentTime() - note_start_time < duration) {
	                  if (getCurrentTime() - note_start_time < duration * 0.9) {
	                      sound_driver_set_frequency(calc_frequency());
	                      sound_driver_volume_on();
	                  } else {
	                      sound_driver_volume_mute();
	                  	  all_notes_next();
	                  }
	              } else {
	                  sound_driver_volume_mute();
	                  all_notes_next();
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
