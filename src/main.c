#include "main.h"
#include "stm32f446xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_tim.h"
motorState_t MotorStates[6];
int state_counter = 0;
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
  initMotorStates();
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
  /*
   * COnfig GPIOS For Motor
   */
  GPIO_Initstruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_Initstruct.Pull = LL_GPIO_PULL_NO;
  GPIO_Initstruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_Initstruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_Initstruct.Pin = L1_EN_PIN | L1_SIG_PIN;
  LL_GPIO_Init(L1_PORT, &GPIO_Initstruct);
  GPIO_Initstruct.Pin = L2_EN_PIN | L2_SIG_PIN;
  LL_GPIO_Init(L2_PORT, &GPIO_Initstruct);
  GPIO_Initstruct.Pin = L3_EN_PIN | L3_SIG_PIN;
  LL_GPIO_Init(L3_PORT, &GPIO_Initstruct);

  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE13);
  LL_EXTI_InitTypeDef EXTI_InitStruct;
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_13;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
  EXTI_InitStruct.LineCommand = ENABLE;
  LL_EXTI_Init(&EXTI_InitStruct);
  /*
   * Set Timer for state change
   */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
  LL_TIM_InitTypeDef TIM_InitStruct;
  TIM_InitStruct.Prescaler = 8400; // clk=10kHz
  TIM_InitStruct.Autoreload = 1000;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM1, &TIM_InitStruct);
  LL_TIM_EnableIT_UPDATE(TIM1);
  LL_TIM_EnableCounter(TIM1);

  uint32_t priority_grouping = 5;
  NVIC_SetPriorityGrouping(priority_grouping);
  uint32_t encoded_priority = NVIC_EncodePriority(priority_grouping, 0, 0);
  NVIC_SetPriority(EXTI15_10_IRQn, encoded_priority);
  NVIC_EnableIRQ(EXTI15_10_IRQn);
  NVIC_SetPriority(TIM1_UP_TIM10_IRQn, encoded_priority);
  NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
  USERLED_PORT->ODR |= USERLED_PIN;
  // USERLED_PORT->ODR &= ~USERLED_PIN;
  while (1) {
  }
}
void initMotorStates() {
  // Initialize the BLDC states like in Maschinen und Antriebe Skriptun page 136
  // State A
  MotorStates[0].L1 = STATE_HIGH;
  MotorStates[0].L2 = STATE_LOW;
  MotorStates[0].L3 = STATE_HIGHZ;
  // State B
  MotorStates[1].L1 = STATE_HIGH;
  MotorStates[1].L2 = STATE_HIGHZ;
  MotorStates[1].L3 = STATE_LOW;
  // State C
  MotorStates[2].L1 = STATE_HIGHZ;
  MotorStates[2].L2 = STATE_HIGH;
  MotorStates[2].L3 = STATE_LOW;
  // State D
  MotorStates[3].L1 = STATE_LOW;
  MotorStates[3].L2 = STATE_HIGH;
  MotorStates[3].L3 = STATE_HIGHZ;
  // State E
  MotorStates[4].L1 = STATE_LOW;
  MotorStates[4].L2 = STATE_HIGHZ;
  MotorStates[4].L3 = STATE_HIGH;
  // State F
  MotorStates[5].L1 = STATE_HIGHZ;
  MotorStates[5].L2 = STATE_LOW;
  MotorStates[5].L3 = STATE_HIGH;
}
void TIM1_UP_TIM10_IRQHandler() {
  if (LL_TIM_IsActiveFlag_UPDATE(TIM1)) {
    LL_TIM_ClearFlag_UPDATE(TIM1);
    state_counter += 1;
    if (state_counter > 5) {
      state_counter = 0;
    }
    SetPinsFromState(&MotorStates[state_counter]);
  }
}
void SetPinsFromState(motorState_t *motorState) {
  writePin(L1_PORT, L1_EN_PIN, motorState->L1 & 0b10);
  writePin(L1_PORT, L1_SIG_PIN, motorState->L1 & 0b01);
  writePin(L2_PORT, L2_EN_PIN, motorState->L2 & 0b10);
  writePin(L2_PORT, L2_SIG_PIN, motorState->L2 & 0b01);
  writePin(L3_PORT, L3_EN_PIN, motorState->L3 & 0b10);
  writePin(L3_PORT, L3_SIG_PIN, motorState->L3 & 0b01);
}
// Stes pin when anything otheer than 0 is given
void writePin(GPIO_TypeDef *port, uint32_t pin, int value) {
  if (value == 0) {
    port->ODR &= ~pin;
  } else {
    port->ODR |= pin;
  }
}
void EXTI15_10_IRQHandler() {
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13)) {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
    USERLED_PORT->ODR ^= USERLED_PIN;
  }
}
// void WWDG_IRQHandler() {}
