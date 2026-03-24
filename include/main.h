#ifndef MAIN_H
#define MAIN_H
#include "stm32f446xx.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_ll_adc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_crc.h"
#include "stm32f4xx_ll_dac.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_dma2d.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_fmc.h"
#include "stm32f4xx_ll_fmpi2c.h"
#include "stm32f4xx_ll_fsmc.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_iwdg.h"
#include "stm32f4xx_ll_lptim.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_rng.h"
#include "stm32f4xx_ll_rtc.h"
#include "stm32f4xx_ll_sdmmc.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_usb.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_wwdg.h"

#include "clock_config.h"

#define USERLED_PORT GPIOA
#define USERLED_PIN LL_GPIO_PIN_5
#define USERBTN_PORT GPIOC
#define USERBTN_PIN LL_GPIO_PIN_13
#define L1_PORT GPIOA
#define L1_SIG_PIN                                                             \
  LL_GPIO_PIN_11 // 13 und 14 sind die JTAG Pins -> man kann dann nicht gescheit
                 // uploaden
#define L1_EN_PIN LL_GPIO_PIN_12
#define L2_PORT GPIOB
#define L2_SIG_PIN LL_GPIO_PIN_1
#define L2_EN_PIN LL_GPIO_PIN_2
#define L3_PORT GPIOC
#define L3_SIG_PIN LL_GPIO_PIN_2
#define L3_EN_PIN LL_GPIO_PIN_3

typedef enum {
  STATE_HIGH = 0b11,
  STATE_LOW = 0b10,
  STATE_HIGHZ = 0b00,
} phaseState_t;
struct motorState_s {
  phaseState_t L1;
  phaseState_t L2;
  phaseState_t L3;
};
typedef struct motorState_s motorState_t;

void writePin(GPIO_TypeDef *port, uint32_t pin, int value);
void initMotorStates();
void SetPinsFromState(motorState_t *motorState);

#endif // !MAIN_H
#define MAIN_H
