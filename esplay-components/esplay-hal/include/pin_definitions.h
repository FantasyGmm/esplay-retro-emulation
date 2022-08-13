#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

#include <driver/adc.h>
#include "sdkconfig.h"

// TFT
#define DISP_SPI_MOSI 12
#define DISP_SPI_CLK 48
#define DISP_SPI_CS 8
#define DISP_SPI_DC 47
#define DISP_BCKL 39

// KEYPAD
#ifdef CONFIG_ESPLAY20_HW
#define A 32
#define B 33
#define START 36
#define SELECT 0
#define IO_Y ADC1_CHANNEL_7
#define IO_X ADC1_CHANNEL_6
#define MENU 13
#endif

#ifdef CONFIG_ESPLAY_MICRO_HW
#define L_BTN   40
#define R_BTN   41
#define MENU    42
#define I2C_SDA 10
#define I2C_SCL 11
#define I2C_ADDR 0x20
#endif

// STATUS LED
#define LED1 2

// AUDIO
#define I2S_BCK 38
#define I2S_WS 13
#define I2S_DOUT 9
#define AMP_SHDN 18

// POWER
#define USB_PLUG_PIN 6
#define CHRG_STATE_PIN 5
#define ADC_PIN ADC1_CHANNEL_3

#endif // PIN_DEFINITIONS_H
