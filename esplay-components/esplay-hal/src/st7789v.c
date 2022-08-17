
/**
 * @file st7789v.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "st7789v.h"
#include "disp_spi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "lcd_struct.h"
#include "driver/ledc.h"
//#include "driver/rtc_io.h"
#include "pin_definitions.h"

/*********************
 *      DEFINES
 *********************/
#define TFT_CMD_SWRESET 0x01
#define TFT_CMD_SLEEP 0x10
#define TFT_CMD_DISPLAY_OFF 0x28
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void st7789v_send_cmd(uint8_t cmd);
static void st7789v_send_data(void *data, uint16_t length);
static void backlight_init();

/**********************
 *  STATIC VARIABLES
 **********************/
static const int DUTY_MAX = 0x1fff;
static const int LCD_BACKLIGHT_ON_VALUE = 1;
static bool isBackLightIntialized = false;

DRAM_ATTR static const lcd_init_cmd_t st_sleep_cmds[] = {
		{TFT_CMD_SWRESET, {0}, 0x80},
		{TFT_CMD_DISPLAY_OFF, {0}, 0x80},
		{TFT_CMD_SLEEP, {0}, 0x80},
		{0, {0}, 0xff}};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void st7789v_init(void)
{
	gpio_set_direction(DISP_RST,GPIO_MODE_OUTPUT);
	gpio_set_level(DISP_RST,0);
	vTaskDelay(120 / portTICK_RATE_MS);
	gpio_set_level(DISP_RST,1);
	lcd_init_cmd_t st_init_cmds[] = {
			{TFT_CMD_SWRESET, {0}, 0x80},
			//-----------------------ST7789V Frame rate setting-----------------//
			{0x3A, {0X05}, 1},  //65k mode
			{0xC5, {0x1A}, 1},  //VCOM
			{0x36, {0x60}, 1},      //屏幕显示方向设置
			//-------------ST7789V Frame rate setting-----------//
			{0xB2, {0x05, 0x05, 0x00, 0x33, 0x33}, 5},  //Porch Setting
			{0xB7, {0x05}, 1}, //Gate Control //12.2v   -10.43v
			//--------------ST7789V Power setting---------------//
			{0xBB, {0x3F}, 1},  //VCOM
			{0xC0, {0x2c}, 1},						//Power control
			{0xC2, {0x01}, 1},						//VDV and VRH Command Enable
			{0xC3, {0x0F}, 1},						//VRH Set 4.3+( vcom+vcom offset+vdv)
			{0xC4, {0xBE}, 1},					//VDV Set 0v
			{0xC6, {0X01}, 1},                    //Frame Rate Control in Normal Mode 111Hz
			{0xD0, {0xA4,0xA1}, 2},           //Power Control 1
			{0xE8, {0x03}, 1},                    //Power Control 1
			{0xE9, {0x09,0x09,0x08}, 3},  //Equalize time control
			//---------------ST7789V gamma setting-------------//
			{0xE0, {0xD0,0x05,0x09,0x09,0x08,0x14,0x28,0x33,0x3F,0x07,0x13,0x14,0x28,0x30}, 14},//Set Gamma
			{0XE1, {0xD0, 0x05, 0x09, 0x09, 0x08, 0x03, 0x24, 0x32, 0x32, 0x3B, 0x14, 0x13, 0x28, 0x2F, 0x1F}, 14},//Set Gamma
			{0x20, {0}, 0},//反显
			{0x11, {0}, 0},//Exit Sleep // 退出睡眠模式
			{0x29, {0}, 0x80},//Display on // 开显示
			{0, {0}, 0xff},
	};

	//Initialize non-SPI GPIOs
	gpio_set_direction(DISP_BCKL, GPIO_MODE_OUTPUT);

	//Reset the display
//	st7789v_send_cmd(TFT_CMD_SWRESET);
//	vTaskDelay(100 / portTICK_RATE_MS);

	printf("ST7789V initialization.\n");

	//Send all the commands
	uint16_t cmd = 0;
	while (st_init_cmds[cmd].databytes != 0xff)
	{
		st7789v_send_cmd(st_init_cmds[cmd].cmd);
		st7789v_send_data(st_init_cmds[cmd].data, st_init_cmds[cmd].databytes & 0x1F);
		if (st_init_cmds[cmd].databytes & 0x80)
		{
			vTaskDelay(100 / portTICK_RATE_MS);
		}
		cmd++;
	}

	///Enable backlight
	printf("Enable backlight.\n");
	backlight_init();
}

int st7789v_is_backlight_initialized()
{
	return isBackLightIntialized;
}

void st7789v_backlight_percentage_set(int value)
{
	int duty = DUTY_MAX * (value * 0.01f);

	ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty, 500);
	ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
}

void st7789v_poweroff()
{
	esp_err_t err = ESP_OK;

	// fade off backlight
	ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (LCD_BACKLIGHT_ON_VALUE) ? 0 : DUTY_MAX, 100);
	ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_WAIT_DONE);

	// Disable LCD panel
	int cmd = 0;
	while (st_sleep_cmds[cmd].databytes != 0xff)
	{
		st7789v_send_cmd(st_sleep_cmds[cmd].cmd);
		st7789v_send_data(st_sleep_cmds[cmd].data, st_sleep_cmds[cmd].databytes & 0x7f);
		if (st_sleep_cmds[cmd].databytes & 0x80)
		{
			vTaskDelay(100 / portTICK_RATE_MS);
		}
		cmd++;
	}
//	err = rtc_gpio_init(DISP_BCKL);
//	if (err != ESP_OK)
//	{
//		abort();
//	}
	err = gpio_set_direction(DISP_BCKL,GPIO_MODE_OUTPUT);
//	err = rtc_gpio_set_direction(DISP_BCKL, RTC_GPIO_MODE_OUTPUT_ONLY);
	if (err != ESP_OK)
	{
		abort();
	}
	err = gpio_set_level(DISP_BCKL,LCD_BACKLIGHT_ON_VALUE ? 0 : 1);
//	err = rtc_gpio_set_level(DISP_BCKL, LCD_BACKLIGHT_ON_VALUE ? 0 : 1);
	if (err != ESP_OK)
	{
		abort();
	}
}

//void st7789v_prepare()
//{
//	// Return use of backlight pin
//}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void st7789v_send_cmd(uint8_t cmd)
{
	disp_spi_send(&cmd, 1, CMD_ON);
}

static void st7789v_send_data(void *data, uint16_t length)
{
	disp_spi_send(data, length, DATA_ON);
}

void st7789v_backlight_deinit()
{
	ledc_fade_func_uninstall();
	esp_err_t err = ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
	if (err != ESP_OK)
	{
		printf("%s: ledc_stop failed.\n", __func__);
	}
}

static void backlight_init()
{
	//configure timer0
	ledc_timer_config_t ledc_timer;
	memset(&ledc_timer, 0, sizeof(ledc_timer));

	ledc_timer.duty_resolution = LEDC_TIMER_13_BIT; //set timer counter bit number
	ledc_timer.freq_hz = 5000;						//set frequency of pwm
	ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;	//timer mode,
	ledc_timer.timer_num = LEDC_TIMER_0;			//timer index

	ledc_timer_config(&ledc_timer);

	//set the configuration
	ledc_channel_config_t ledc_channel;
	memset(&ledc_channel, 0, sizeof(ledc_channel));

	//set LEDC channel 0
	ledc_channel.channel = LEDC_CHANNEL_0;
	//set the duty for initialization.(duty range is 0 ~ ((2**duty_resolution)-1)
	ledc_channel.duty = (LCD_BACKLIGHT_ON_VALUE) ? 0 : DUTY_MAX;
	//GPIO number
	ledc_channel.gpio_num = DISP_BCKL;
	//GPIO INTR TYPE, as an example, we enable fade_end interrupt here.
	ledc_channel.intr_type = LEDC_INTR_FADE_END;
	//set LEDC mode, from ledc_mode_t
	ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
	//set LEDC timer source, if different channel use one timer,
	//the frequency and duty_resolution of these channels should be the same
	ledc_channel.timer_sel = LEDC_TIMER_0;

	ledc_channel_config(&ledc_channel);

	//initialize fade service.
	ledc_fade_func_install(0);

	// duty range is 0 ~ ((2**duty_resolution)-1)
	ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (LCD_BACKLIGHT_ON_VALUE) ? DUTY_MAX : 0, 500);
	ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);

	isBackLightIntialized = true;

	printf("Backlight initialization done.\n");
}