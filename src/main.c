#include "main.h"
#include "stm32f446xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_utils.h"
#include <stdint.h>
#define N_MOTOR_STATES 6
#define PWM_ARR 250
motorState_t MotorStates[N_MOTOR_STATES];
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
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
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
  GPIO_Initstruct.Pin = L1_EN_PIN | L2_EN_PIN | L3_EN_PIN;
  LL_GPIO_Init(EN_PORT, &GPIO_Initstruct);
  GPIO_Initstruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_Initstruct.Alternate = LL_GPIO_AF_2;
  GPIO_Initstruct.Pin =
      L1_SIG_PIN | L2_SIG_PIN | L3_SIG_PIN; // | LL_GPIO_PIN_7;
  LL_GPIO_Init(SIG_PORT, &GPIO_Initstruct);
  // GPIO_Initstruct.Pin = L3_EN_PIN | L3_SIG_PIN;
  // LL_GPIO_Init(L3_PORT, &GPIO_Initstruct);

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
  TIM_InitStruct.Prescaler = 840; // clk=100kHz -> T=10 us
  TIM_InitStruct.Autoreload = 2;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM1, &TIM_InitStruct);
  LL_TIM_EnableARRPreload(TIM1);
  LL_TIM_EnableIT_UPDATE(TIM1);
  LL_TIM_EnableCounter(TIM1);
  /*
   * Define PWM Timer
   */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
  TIM_InitStruct.Prescaler = 84;       // clk=1MHz
  TIM_InitStruct.Autoreload = PWM_ARR; // clk=4kHz
  LL_TIM_Init(TIM3, &TIM_InitStruct);
  LL_TIM_EnableARRPreload(TIM3);
  LL_TIM_EnableCounter(TIM3);
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct;
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.CompareValue = PWM_ARR / 2;
  TIM_OC_InitStruct.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_ENABLE;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct); // PC6
  // LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct); // PC7
  LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH3, &TIM_OC_InitStruct); // PC8
  LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH4, &TIM_OC_InitStruct); // PC9
  LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH1);
  // LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH2);
  LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH3);
  LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH4);

  // LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH2);
  LL_TIM_CC_DisableChannel(TIM3, LL_TIM_CHANNEL_CH1);
  LL_TIM_CC_DisableChannel(TIM3, LL_TIM_CHANNEL_CH3);
  LL_TIM_CC_DisableChannel(TIM3, LL_TIM_CHANNEL_CH4);

  uint32_t priority_grouping = 5;
  NVIC_SetPriorityGrouping(priority_grouping);
  uint32_t encoded_priority = NVIC_EncodePriority(priority_grouping, 0, 0);
  NVIC_SetPriority(EXTI15_10_IRQn, encoded_priority);
  NVIC_EnableIRQ(EXTI15_10_IRQn);
  NVIC_SetPriority(TIM1_UP_TIM10_IRQn, encoded_priority);
  NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
  USERLED_PORT->ODR |= USERLED_PIN;
  // USERLED_PORT->ODR &= ~USERLED_PIN;
  // int current_arr = 10000;
  // int arr_min = 2;
  int f_curr = 15; // Hz
  int f_max = 300; // Hz
  float pwm_value_start = 0.5;
  int f_max_pwm = 800;
  int f_pwm_start = 0;
  LL_TIM_SetAutoReload(TIM1,
                       (int)(1 / (float)f_curr / (10e-6 * N_MOTOR_STATES)));

  while (1) {
    if (f_curr < f_max) {
      LL_TIM_SetAutoReload(TIM1,
                           (int)(1 / (float)f_curr / (10e-6 * N_MOTOR_STATES)));
      // LL_TIM_SetAutoReload(TIM1, (int)(1 / (float)f_curr / 10e-6));
      f_curr += 1;
      LL_mDelay(100);
    }
    if (f_curr > f_pwm_start && f_curr < f_max_pwm) {
      LL_TIM_OC_SetMode(TIM3, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
      LL_TIM_OC_SetMode(TIM3, LL_TIM_CHANNEL_CH3, LL_TIM_OCMODE_PWM1);
      LL_TIM_OC_SetMode(TIM3, LL_TIM_CHANNEL_CH4, LL_TIM_OCMODE_PWM1);

      setPWMvalue(pwm_value_start + (1 - pwm_value_start) /
                                        (float)(f_max_pwm - f_pwm_start) *
                                        (float)(f_curr - f_pwm_start));
    } else {
      LL_TIM_OC_SetMode(TIM3, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_FORCED_ACTIVE);
      LL_TIM_OC_SetMode(TIM3, LL_TIM_CHANNEL_CH3, LL_TIM_OCMODE_FORCED_ACTIVE);
      LL_TIM_OC_SetMode(TIM3, LL_TIM_CHANNEL_CH4, LL_TIM_OCMODE_FORCED_ACTIVE);
    }
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
  writePin(EN_PORT, L1_EN_PIN, motorState->L1 & 0b10);
  setPWMstate(LL_TIM_CHANNEL_CH1, motorState->L1 & 0b01);

  writePin(EN_PORT, L2_EN_PIN, motorState->L2 & 0b10);
  setPWMstate(LL_TIM_CHANNEL_CH3, motorState->L2 & 0b01);

  writePin(EN_PORT, L3_EN_PIN, motorState->L3 & 0b10);
  setPWMstate(LL_TIM_CHANNEL_CH4, motorState->L2 & 0b01);
}
// Stes pin when anything otheer than 0 is given
void setPWMstate(uint32_t channel, int state) {
  if (state == 0) {
    LL_TIM_CC_DisableChannel(TIM3, channel);
  } else {
    LL_TIM_CC_EnableChannel(TIM3, channel);
  }
}
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
void setPWMvalue(float pwm) {
  if (pwm > 1) {
    pwm = 1;
  }
  int ccr = (int)(PWM_ARR * pwm);
  LL_TIM_OC_SetCompareCH1(TIM3, ccr);
  LL_TIM_OC_SetCompareCH3(TIM3, ccr);
  LL_TIM_OC_SetCompareCH4(TIM3, ccr);
}
