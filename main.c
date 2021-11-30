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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CLEAN 0
#define RED GPIO_PIN_15
#define YELLOW  GPIO_PIN_14
#define GREEN GPIO_PIN_13

#define  SET GPIO_PIN_SET
#define  RESET GPIO_PIN_RESET
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
int ignoreBtn = 0;
int timeout = 10000;
int interrupt = 0;
char ch[] = "F\0";

void switchRedYellow(int pin) {
    if (pin!= RED && pin!=YELLOW && pin!= CLEAN)
        return;
    HAL_GPIO_WritePin(GPIOD, RED, pin == RED ? SET:RESET);
    HAL_GPIO_WritePin(GPIOD, YELLOW, pin == YELLOW? SET: RESET);
}


void switchGreen(int set){
    HAL_GPIO_WritePin(GPIOD,GREEN, set ? SET : RESET);
}



void blink(int pin){
    if (pin!= RED && pin!=YELLOW && pin!= GREEN)
        return;
//    for (int i = period; i <= time; i+=period) {
        HAL_GPIO_TogglePin(GPIOD, pin);
//        HAL_Delay(period);
//    }
}

long getCurrentTime(){
    return HAL_GetTick();
}

void help(char* message){
    char buf[20] = "";
    if(HAL_GPIO_ReadPin(GPIOD,RED) == SET)
        strcat(message,"RED\n\r");
    else if(HAL_GPIO_ReadPin(GPIOD,YELLOW) == SET)
        strcat(message,"Yellow\n\r");
    else if(HAL_GPIO_ReadPin(GPIOD,GREEN) == SET)
        strcat(message,"Green\n\r");

    strcat(message, "timeout ");
    sprintf(buf,"%d\n\r", timeout);
    strcat(message, buf);

    strcat(message, "ignoreBtn ");
    strcat(message, ignoreBtn ? "2\n\r" : "1\n\r");

    strcat(message, !interrupt?"Polling\n\r":"Interrupt\n\r");
}

void handleMessage(char *message){
    char response[300] = "";
    if (!strcmp(message, "?\r"))
        help(response);
    else
    if (!strcmp(message, "set mode 1\r")) {
        ignoreBtn = 0;
        strcpy(response,"OK\n\r");
    }
    else
    if (!strcmp(message, "set mode 2\r")) {
        ignoreBtn = 1;
        strcpy(response,"OK\n\r");
    }
    else
    if (!strcmp(message, "set interrupts on\r")) {
        interrupt = 1;
        strcpy(response,"OK\n\r");
    }
    else
    if (!strcmp(message, "set interrupts off\r")) {
        interrupt = 0;
        strcpy(response,"OK\n\r");
    }
    else
    if (!strncmp(message,"set timeout",11)){
        timeout = strtol(&message[11],0,10) * 1000;
        strcpy(response,"OK\n\r");
    }
    else
        strcpy(response,"ERROR\n\r");

    sendMessage(response);
}

void sendMessage(char *message) {
	int size = strlen(message);
	if (message[0] == '\r'){
		sendMessage("\n");
	}
    if (!interrupt)
        HAL_UART_Transmit(&huart6, (uint8_t *) message, size, 100);
    else {
        long time = getCurrentTime();
        HAL_StatusTypeDef isReaded1 = HAL_BUSY;
        int curtime = getCurrentTime();
        int counter = 0;
        while (counter < 100000 && isReaded1 == HAL_BUSY) {
        	isReaded1 = HAL_UART_Transmit_IT(&huart6, (uint8_t *) message, size);
        	curtime = getCurrentTime();
        	counter++;
                }
    }
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
  long time;
  char msg[200] = "";
  int i = 0;
   HAL_StatusTypeDef status = HAL_BUSY;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  switchGreen(1);
	  	        time = getCurrentTime();
	  	        while (getCurrentTime() - time < timeout/2){
	  	            if (!interrupt)
	  	                status = HAL_UART_Receive(&huart6, (uint8_t *) &ch, 1, 200);
	  	            else
	  	                status = HAL_UART_Receive_IT(&huart6, (uint8_t *) &ch, 1);

	  	            if (status == HAL_OK) {

	  	            	sendMessage(ch);
	  	            	msg[i++] = ch[0];
	  	            	msg[i] = '\0';

	                      if (ch[0] == '\r') {
	                          handleMessage(msg);
	                          msg[0] = '\0';
	                          i = 0;
	                      }
	                  }


	  	        }
	  	        time = getCurrentTime();
	  	        long current = time;
	  	        while ((current = getCurrentTime()) - time < timeout/2) {
	                  if (current - time % timeout / 10 >= timeout / 20)
	                      blink(GREEN);
	                  if (!interrupt)
	                      status = HAL_UART_Receive(&huart6, (uint8_t * ) ch, 1, 200);
	                  else
	                      status = HAL_UART_Receive_IT(&huart6, (uint8_t * )ch, 1);

	                  if (status == HAL_OK) {

	                	  sendMessage(ch);
	                	  msg[i++]=ch[0];
	                	  msg[i] = '\0';

	                      if (ch[0] == '\r') {
	                          handleMessage(msg);
	                          msg[0] = '\0';
	                          i = 0;
	                      }
	                  }


	              }

	  	  	  switchGreen(CLEAN);

	  	        switchRedYellow(YELLOW);
	  	        time = getCurrentTime();
	  	        while (getCurrentTime() - time < timeout/2) {
	                  if (!interrupt)
	                      status = HAL_UART_Receive(&huart6, (uint8_t * ) ch, 1, 200);
	                  else
	                      status = HAL_UART_Receive_IT(&huart6, (uint8_t * ) ch, 1);

	                  if (status == HAL_OK) {

	                	  sendMessage(ch);
	                	  msg[i++] = ch[0];
	                	  msg[i] = '\0';

	                      if (ch[0] == '\r') {
	                          handleMessage(msg);
	                          msg[0] = '\0';
	                          i = 0;
	                      }
	                  }

	              }

	  	  	  switchRedYellow(RED);
	  	        time = getCurrentTime();
	  	        int pressed = 0;
	  	        while (getCurrentTime() - time < timeout) {
	                  if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_RESET)
	                      pressed = 1;

	                  if (!ignoreBtn && pressed)
	                      if (getCurrentTime() - time <= timeout * 3 / 4)
	                          break;

	                  if (!interrupt)
	                      status = HAL_UART_Receive(&huart6, (uint8_t * ) ch, 1, 200);
	                  else
	                      status = HAL_UART_Receive_IT(&huart6, (uint8_t * ) ch, 1);

	                  if (status == HAL_OK) {

	                	  sendMessage(ch);
	                	  msg[i++] = ch[0];
	                	  msg[i] = '\0';

	                      if (ch[0] == '\r') {
	                          handleMessage(msg);
	                          msg[0] = '\0';
	                          i = 0;
	                      }
	                  }

	              }
	  	  	  switchRedYellow(CLEAN);
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
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
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
