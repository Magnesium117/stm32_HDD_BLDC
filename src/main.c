#include "main.h"
// #include "stm32f4xx_ll_gpio.h"
int main() {
  //
  // Configure Clock
  //
  SystemClock_Config();
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  /* Enable HSI if not already activated*/
  // if (LL_RCC_HSI_IsReady() == 0) {
  //   /* Enable HSI and wait for activation*/
  //   LL_RCC_HSI_Enable();
  //   while (LL_RCC_HSI_IsReady() != 1) {
  //   };
  // }
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  LL_GPIO_InitTypeDef GPIO_Initstruct;
  LL_GPIO_StructInit(&GPIO_Initstruct);
  GPIO_Initstruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_Initstruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_Initstruct.Pin = USERLED_PIN;
  GPIO_Initstruct.Pull = LL_GPIO_PULL_NO;
  GPIO_Initstruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  LL_GPIO_Init(USERLED_PORT, &GPIO_Initstruct);

  GPIO_Initstruct.Pin = USERBTN_PIN;
  GPIO_Initstruct.Pull = LL_GPIO_PULL_UP;
  GPIO_Initstruct.Mode = LL_GPIO_MODE_INPUT;
  LL_GPIO_Init(USERBTN_PORT, &GPIO_Initstruct);

  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE13);
  LL_EXTI_InitTypeDef EXTI_InitStruct;
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_13;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
  EXTI_InitStruct.LineCommand = ENABLE;
  LL_EXTI_Init(&EXTI_InitStruct);

  uint32_t priority_grouping = 5;
  NVIC_SetPriorityGrouping(priority_grouping);
  uint32_t encoded_priority = NVIC_EncodePriority(priority_grouping, 0, 0);
  NVIC_SetPriority(EXTI15_10_IRQn, encoded_priority);
  NVIC_EnableIRQ(EXTI15_10_IRQn);
  USERLED_PORT->ODR |= USERLED_PIN;
  // USERLED_PORT->ODR &= ~USERLED_PIN;
  while (1) {
  }
}

void EXTI15_10_IRQHandler() {
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13)) {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
    USERLED_PORT->ODR ^= USERLED_PIN;
  }
}
// void WWDG_IRQHandler() {}
