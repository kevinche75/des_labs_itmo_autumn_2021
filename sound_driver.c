/*
 * sound_driver.c
 *
 *  Created on: Dec 23, 2021
 *      Author: kevinche
 */

#include "sound_driver.h"
#include "tim.h"
#include "main.h"
#include <stdio.h>

void sound_driver_init(void) {
    HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Init(&htim2);
};


void sound_driver_set_frequency(uint32_t freq) {
	char message[150] = "";
	uint32_t psc = (((uint32_t)HAL_RCC_GetPCLK1Freq()) / (10 * freq)) - 1;
	uint16_t arr = TIM2->ARR;
	if (psc > 65635){
		uint32_t psc = (((uint32_t)HAL_RCC_GetPCLK1Freq()) / (8 * 10 * freq)) - 1;
		arr *= 8;
	}
    TIM2->PSC = (uint16_t)psc;
    TIM2->ARR = arr;
};

void sound_driver_volume_on() {
    TIM2->CCR1 = 10;
};

void sound_driver_volume_mute() {
    TIM2->CCR1 = 0;
};
