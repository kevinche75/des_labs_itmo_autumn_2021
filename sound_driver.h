//
// Created by Dmitrii Beliakov on 19.12.2021.
//

#include "main.h"

#define BUZZER_VOLUME_MAX	10
#define BUZZER_VOLUME_MUTE	0

void sound_driver_init(void);
void sound_driver_set_frequency(uint16_t freq);
void sound_driver_volume_on();
void sound_driver_volume_mute();
void sound_driver_play(int cur_melody,int cur_pause, int tempo);
int wholeNote(int tempo);