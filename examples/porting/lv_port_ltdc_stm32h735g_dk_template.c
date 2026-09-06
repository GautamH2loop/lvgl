/**
 * @file lv_port_ltdc_stm32h735g_dk_template.c
 *
 * STM32H735G-DK LTDC + LVGL port template.
 */

/* Copy this file as "lv_port_disp.c" and set this value to "1" to enable content */
#if 0

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_ltdc_stm32h735g_dk_template.h"

#if defined(STM32H735xx) || defined(STM32H7)
#include "main.h"
#include "lvgl/drivers/display/lv_st_ltdc.h"
#include "lvgl/draw/dma2d/lv_draw_dma2d.h"
#endif

/*********************
 *      DEFINES
 *********************/
#ifndef LV_PORT_H735G_DK_HOR_RES
    #define LV_PORT_H735G_DK_HOR_RES 480U
#endif

#ifndef LV_PORT_H735G_DK_VER_RES
    #define LV_PORT_H735G_DK_VER_RES 272U
#endif

#ifndef LV_PORT_H735G_DK_LAYER_IDX
    #define LV_PORT_H735G_DK_LAYER_IDX 0U
#endif

#ifndef LV_PORT_H735G_DK_USE_DOUBLE_FB
    #define LV_PORT_H735G_DK_USE_DOUBLE_FB 1U
#endif

#ifndef LV_PORT_H735G_DK_USE_DIRECT_MODE
    #define LV_PORT_H735G_DK_USE_DIRECT_MODE 1U
#endif

#if LV_PORT_H735G_DK_USE_DIRECT_MODE
    #define LV_PORT_H735G_DK_RENDER_MODE LV_DISPLAY_RENDER_MODE_DIRECT
#else
    #define LV_PORT_H735G_DK_RENDER_MODE LV_DISPLAY_RENDER_MODE_PARTIAL
#endif

#define LV_PORT_H735G_DK_COLOR_FORMAT LV_COLOR_FORMAT_RGB565
#define LV_PORT_H735G_DK_BPP 2U
#define LV_PORT_H735G_DK_FRAMEBUFFER_BYTES (LV_PORT_H735G_DK_HOR_RES * LV_PORT_H735G_DK_VER_RES * LV_PORT_H735G_DK_BPP)
#define LV_PORT_H735G_DK_PARTIAL_LINES 40U
#define LV_PORT_H735G_DK_PARTIAL_BUF_BYTES (LV_PORT_H735G_DK_HOR_RES * LV_PORT_H735G_DK_PARTIAL_LINES * LV_PORT_H735G_DK_BPP)

#ifndef LV_PORT_H735G_DK_ENABLE_TOUCH_STUB
    #define LV_PORT_H735G_DK_ENABLE_TOUCH_STUB 0U
#endif

#ifndef LV_PORT_H735G_DK_TOUCH_IRQ_PIN
    #define LV_PORT_H735G_DK_TOUCH_IRQ_PIN 0U
#endif

#ifndef LV_PORT_H735G_DK_FB_SECTION
    #define LV_PORT_H735G_DK_FB_SECTION
#endif

#ifndef LV_PORT_H735G_DK_PARTIAL_BUF_SECTION
    #define LV_PORT_H735G_DK_PARTIAL_BUF_SECTION
#endif

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_display_t * disp;
    lv_indev_t * indev;
    volatile uint8_t touch_irq_pending;
} lv_port_h735g_dk_ctx_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_display_t * create_display(void);
static lv_indev_t * create_touch_indev(void);
static void touch_read_cb(lv_indev_t * indev, lv_indev_data_t * data);

/**********************
 *  STATIC VARIABLES
 **********************/
extern LTDC_HandleTypeDef hltdc;

static lv_port_h735g_dk_ctx_t g_ctx;

/*
 * Framebuffers are static and bounded to keep the graphics path deterministic.
 * Place these in a linker section mapped to RAM visible by LTDC.
 */
LV_ATTRIBUTE_MEM_ALIGN LV_PORT_H735G_DK_FB_SECTION
static uint8_t g_framebuffer_1[LV_PORT_H735G_DK_FRAMEBUFFER_BYTES];

#if LV_PORT_H735G_DK_USE_DOUBLE_FB
LV_ATTRIBUTE_MEM_ALIGN LV_PORT_H735G_DK_FB_SECTION
static uint8_t g_framebuffer_2[LV_PORT_H735G_DK_FRAMEBUFFER_BYTES];
#endif

#if !LV_PORT_H735G_DK_USE_DIRECT_MODE
LV_ATTRIBUTE_MEM_ALIGN LV_PORT_H735G_DK_PARTIAL_BUF_SECTION
static uint8_t g_partial_buf_1[LV_PORT_H735G_DK_PARTIAL_BUF_BYTES];
#if LV_PORT_H735G_DK_USE_DOUBLE_FB
LV_ATTRIBUTE_MEM_ALIGN LV_PORT_H735G_DK_PARTIAL_BUF_SECTION
static uint8_t g_partial_buf_2[LV_PORT_H735G_DK_PARTIAL_BUF_BYTES];
#endif
#endif

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_port_disp_init(void)
{
    lv_init();

    /* Use STM32 HAL millisecond tick source for LVGL timing. */
    lv_tick_set_cb(HAL_GetTick);

    g_ctx.disp = create_display();
#if LV_PORT_H735G_DK_ENABLE_TOUCH_STUB
    g_ctx.indev = create_touch_indev();
#else
    g_ctx.indev = NULL;
#endif
}

void lv_port_disp_task_handler(void)
{
    lv_timer_handler();

    /* HAL_Delay(2) is preferred over HAL_Delay(1) for stable pacing. */
    HAL_Delay(2U);
}

lv_display_t * lv_port_disp_get_display(void)
{
    return g_ctx.disp;
}

lv_indev_t * lv_port_disp_get_touch_indev(void)
{
    return g_ctx.indev;
}

void lv_port_disp_on_dma2d_transfer_complete_isr(void)
{
#if LV_USE_DRAW_DMA2D && LV_USE_DRAW_DMA2D_INTERRUPT
    lv_draw_dma2d_transfer_complete_interrupt_handler();
#endif
}

void lv_port_disp_on_touch_irq_isr(void)
{
#if LV_PORT_H735G_DK_ENABLE_TOUCH_STUB
    g_ctx.touch_irq_pending = 1U;
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static lv_display_t * create_display(void)
{
    lv_display_t * disp;

    /*
     * Set LTDC layer framebuffer start address explicitly so the LTDC CFBAR
     * matches the LVGL buffer address.
     */
    (void)HAL_LTDC_SetAddress(&hltdc, (uint32_t)(uintptr_t)&g_framebuffer_1[0], LV_PORT_H735G_DK_LAYER_IDX);

#if LV_PORT_H735G_DK_USE_DIRECT_MODE
    disp = lv_st_ltdc_create_direct((void *)&g_framebuffer_1[0],
#if LV_PORT_H735G_DK_USE_DOUBLE_FB
                                    (void *)&g_framebuffer_2[0],
#else
                                    NULL,
#endif
                                    LV_PORT_H735G_DK_LAYER_IDX);
#else
    disp = lv_st_ltdc_create_partial((void *)&g_partial_buf_1[0],
#if LV_PORT_H735G_DK_USE_DOUBLE_FB
                                     (void *)&g_partial_buf_2[0],
#else
                                     NULL,
#endif
                                     LV_PORT_H735G_DK_PARTIAL_BUF_BYTES,
                                     LV_PORT_H735G_DK_LAYER_IDX);
#endif

    if(disp == NULL) {
        return NULL;
    }

    lv_display_set_color_format(disp, LV_PORT_H735G_DK_COLOR_FORMAT);

    return disp;
}

static lv_indev_t * create_touch_indev(void)
{
    lv_indev_t * indev = lv_indev_create();
    if(indev == NULL) {
        return NULL;
    }

    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read_cb);

    return indev;
}

static void touch_read_cb(lv_indev_t * indev, lv_indev_data_t * data)
{
    LV_UNUSED(indev);

#if LV_PORT_H735G_DK_ENABLE_TOUCH_STUB
    if(g_ctx.touch_irq_pending != 0U) {
        g_ctx.touch_irq_pending = 0U;

        /*
         * Board-specific touch controller readout must be implemented here.
         * Keep this path non-blocking and bounded.
         */
        data->state = LV_INDEV_STATE_RELEASED;
        data->point.x = 0;
        data->point.y = 0;
    }
    else {
        data->state = LV_INDEV_STATE_RELEASED;
        data->point.x = 0;
        data->point.y = 0;
    }
#else
    data->state = LV_INDEV_STATE_RELEASED;
    data->point.x = 0;
    data->point.y = 0;
#endif
}

/*
 * Hook STM32 HAL weak callbacks to the port hooks below in your application code:
 *
 * void DMA2D_IRQHandler(void)
 * {
 *     HAL_DMA2D_IRQHandler(&hdma2d);
 *     lv_port_disp_on_dma2d_transfer_complete_isr();
 * }
 *
 * void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
 * {
 *     if(GPIO_Pin == LV_PORT_H735G_DK_TOUCH_IRQ_PIN) {
 *         lv_port_disp_on_touch_irq_isr();
 *     }
 * }
 *
 * Example linker section assignment in your application-level config header:
 *
 * #define LV_PORT_H735G_DK_FB_SECTION __attribute__((section(".ltdc_fb")))
 * #define LV_PORT_H735G_DK_PARTIAL_BUF_SECTION __attribute__((section(".ltdc_buf")))
 */

#else /* Enable this file at the top */

/* This dummy typedef exists purely to silence -Wpedantic. */
typedef int keep_pedantic_happy;

#endif