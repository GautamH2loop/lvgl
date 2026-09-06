/**
 * @file lv_port_ltdc_stm32h735g_dk_template.h
 *
 * STM32H735G-DK LTDC + LVGL port template.
 */

/* Copy this file as "lv_port_disp.h" and set this value to "1" to enable content */
#if 0

#ifndef LV_PORT_LTDC_STM32H735G_DK_TEMPLATE_H
#define LV_PORT_LTDC_STM32H735G_DK_TEMPLATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_port_disp_init(void);
void lv_port_disp_task_handler(void);

lv_display_t * lv_port_disp_get_display(void);
lv_indev_t * lv_port_disp_get_touch_indev(void);

/* ISR/driver hooks to be called from STM32 HAL callbacks */
void lv_port_disp_on_dma2d_transfer_complete_isr(void);
void lv_port_disp_on_touch_irq_isr(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_PORT_LTDC_STM32H735G_DK_TEMPLATE_H */

#endif /* Disable/Enable content */
