/**
 * @file lv_templ.h
 *
 */

#ifndef __ST7789V_H__
#define __ST7789V_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/
#define ST7789V_HOR_RES 320
#define ST7789V_VER_RES 240

/**********************
*      TYPEDEFS
**********************/

/**********************
* GLOBAL PROTOTYPES
**********************/
int st7789v_is_backlight_initialized();
void st7789v_backlight_percentage_set(int value);
void st7789v_init(void);
void st7789v_backlight_deinit();
void st7789v_prepare();
void st7789v_poweroff();

/**********************
*      MACROS
**********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*ILI9342_H*/
