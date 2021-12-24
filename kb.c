#include "main.h"
#include "pca9538.h"
#include "kb.h"
#include "sdk_uart.h"
#include "usart.h"
#include "utils.h"

static const uint8_t rows[] = { 0xFE, 0xFD, 0xFB, 0xF7 };

uint8_t read_row(uint8_t n_row) {
    uint8_t n_key = 0x00;
    HAL_StatusTypeDef ret = HAL_OK;
    uint8_t mask = rows[n_row];
    uint8_t kbd_in;

    ret = PCA9538_Write_Register(KBRD_WR_ADDR, CONFIG, mask);
    if (ret != HAL_OK) {
        send_message((uint8_t*) "Can't inverse ports\r\n");
    }

    ret = PCA9538_Write_Register(KBRD_WR_ADDR, OUTPUT_PORT, mask);
    if (ret != HAL_OK) {
        send_message((uint8_t*) "Can't write 1 on row\r\n");
    }

    uint8_t buf = 0;
    ret = PCA9538_Read_Inputs(KBRD_RD_ADDR, buf);
    if (ret != HAL_OK) {
        send_message((uint8_t*) "Can't read column\r\n");
    }
    kbd_in = buf & 0x70;
    n_key = 0x0F ^ (kbd_in >> 4);
    return Nkey;
}

int check_button() {
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);
}
