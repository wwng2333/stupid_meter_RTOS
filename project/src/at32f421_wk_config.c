/* add user code begin Header */
/**
  **************************************************************************
  * @file     at32f421_wk_config.c
  * @brief    work bench config program
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */
/* add user code end Header */

#include "at32f421_wk_config.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */

/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */

/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */

/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */

/* add user code end 0 */

/**
  * @brief  system clock config program
  * @note   the system clock is configured as follow:
  *         system clock (sclk)   = hick / 12 * pll_mult
  *         system clock source   = pll (hick)
  *         - sclk                = 120000000
  *         - ahbdiv              = 1
  *         - ahbclk              = 120000000
  *         - apb1div             = 1
  *         - apb1clk             = 120000000
  *         - apb2div             = 1
  *         - apb2clk             = 120000000
  *         - pll_mult            = 30
  *         - flash_wtcyc         = 3 cycle
  * @param  none
  * @retval none
  */
void wk_system_clock_config(void)
{
  /* reset crm */
  crm_reset();

  /* config flash psr register */
  flash_psr_set(FLASH_WAIT_CYCLE_3);

  /* enable lick */
  crm_clock_source_enable(CRM_CLOCK_SOURCE_LICK, TRUE);

  /* wait till lick is ready */
  while(crm_flag_get(CRM_LICK_STABLE_FLAG) != SET)
  {
  }

  /* enable hick */
  crm_clock_source_enable(CRM_CLOCK_SOURCE_HICK, TRUE);

  /* wait till hick is ready */
  while(crm_flag_get(CRM_HICK_STABLE_FLAG) != SET)
  {
  }

  /* config pll clock resource */
  crm_pll_config(CRM_PLL_SOURCE_HICK, CRM_PLL_MULT_30);

  /* enable pll */
  crm_clock_source_enable(CRM_CLOCK_SOURCE_PLL, TRUE);

  /* wait till pll is ready */
  while(crm_flag_get(CRM_PLL_STABLE_FLAG) != SET)
  {
  }

  /* config ahbclk */
  crm_ahb_div_set(CRM_AHB_DIV_1);

  /* config apb2clk */
  crm_apb2_div_set(CRM_APB2_DIV_1);

  /* config apb1clk */
  crm_apb1_div_set(CRM_APB1_DIV_1);

  /* enable auto step mode */
  crm_auto_step_mode_enable(TRUE);

  /* select pll as system clock source */
  crm_sysclk_switch(CRM_SCLK_PLL);

  /* wait till pll is used as system clock source */
  while(crm_sysclk_switch_status_get() != CRM_SCLK_PLL)
  {
  }

  /* disable auto step mode */
  crm_auto_step_mode_enable(FALSE);

  /* update system_core_clock global variable */
  system_core_clock_update();
}

/**
  * @brief  config periph clock
  * @param  none
  * @retval none
  */
void wk_periph_clock_config(void)
{
  /* enable gpioa periph clock */
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
}

/**
  * @brief  nvic config
  * @param  none
  * @retval none
  */
void wk_nvic_config(void)
{
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
	
	nvic_irq_enable(DMA1_Channel3_2_IRQn, 1, 0);
}

/**
  * @brief  init gpio_input/gpio_output/gpio_analog/eventout function.
  * @param  none
  * @retval none
  */
void wk_gpio_config(void)
{
  /* enable gpiob periph clock */
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
	
  gpio_init_type gpio_init_struct;

  /* configure the PB0 */
  gpio_default_para_init(&gpio_init_struct);
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_0;
  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
  gpio_init(GPIOB, &gpio_init_struct);
}

/**
  * @brief  init dma1 channel1 for "adc1"
  * @param  none
  * @retval none
  */
void wk_dma1_channel1_init(void)
{
  /* enable dma1 periph clock */
  crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
	
  dma_init_type dma_init_struct;

  dma_reset(DMA1_CHANNEL1);
  dma_default_para_init(&dma_init_struct);
  dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_HALFWORD;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = DMA_PRIORITY_HIGH;
  dma_init_struct.loop_mode_enable = TRUE;
  dma_init(DMA1_CHANNEL1, &dma_init_struct);
}

/**
  * @brief  init adc1 function.
  * @param  none
  * @retval none
  */
void wk_adc1_init(void)
{
	/* enable adc1 periph clock */
  crm_periph_clock_enable(CRM_ADC1_PERIPH_CLOCK, TRUE);
	
  /* add user code begin adc1_init 0 */

  /* add user code end adc1_init 0 */

  adc_base_config_type adc_base_struct;

  /* add user code begin adc1_init 1 */

  /* add user code end adc1_init 1 */

  /* gpio--------------------------------------------------------------------*/
  crm_adc_clock_div_set(CRM_ADC_DIV_6);

  adc_tempersensor_vintrv_enable(TRUE);

  /* adc_settings----------------------------------------------------------- */
  adc_base_default_para_init(&adc_base_struct);
  adc_base_struct.sequence_mode = TRUE;
  adc_base_struct.repeat_mode = TRUE;
  adc_base_struct.data_align = ADC_RIGHT_ALIGNMENT;
  adc_base_struct.ordinary_channel_length = 2;
  adc_base_config(ADC1, &adc_base_struct);

  /* adc_ordinary_conversionmode-------------------------------------------- */
  adc_ordinary_channel_set(ADC1, ADC_CHANNEL_16, 1, ADC_SAMPLETIME_239_5);
  adc_ordinary_channel_set(ADC1, ADC_CHANNEL_17, 2, ADC_SAMPLETIME_239_5);

  adc_ordinary_conversion_trigger_set(ADC1, ADC12_ORDINARY_TRIG_SOFTWARE, TRUE);

  adc_ordinary_part_mode_enable(ADC1, FALSE);

  adc_dma_mode_enable(ADC1, TRUE);
  adc_enable(ADC1, TRUE);

  /* adc calibration-------------------------------------------------------- */
  adc_calibration_init(ADC1);
  while(adc_calibration_init_status_get(ADC1));
  adc_calibration_start(ADC1);
  while(adc_calibration_status_get(ADC1));

  /* add user code begin adc1_init 2 */

  /* add user code end adc1_init 2 */
}

/**
  * @brief  init usart1 function
  * @param  none
  * @retval none
  */
void wk_usart1_init(void)
{
  /* add user code begin usart1_init 0 */
  crm_periph_clock_enable(CRM_USART1_PERIPH_CLOCK, TRUE);
  /* add user code end usart1_init 0 */

  gpio_init_type gpio_init_struct;
  gpio_default_para_init(&gpio_init_struct);

  /* add user code begin usart1_init 1 */

  /* add user code end usart1_init 1 */

  /* configure the TX pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_9;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOA, &gpio_init_struct);

  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE9, GPIO_MUX_1);

  /* configure the RX pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_10;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOA, &gpio_init_struct);

  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE10, GPIO_MUX_1);

  /* configure param */
  usart_init(USART1, 115200, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_transmitter_enable(USART1, TRUE);
  //usart_receiver_enable(USART1, TRUE);
  //usart_parity_selection_config(USART1, USART_PARITY_NONE);

  //usart_hardware_flow_control_set(USART1, USART_HARDWARE_FLOW_NONE);

  /**
   * Users need to configure USART1 interrupt functions according to the actual application.
   * 1. Call the below function to enable the corresponding USART1 interrupt.
   *     --usart_interrupt_enable(...)
   * 2. Add the user's interrupt handler code into the below function in the at32f421_int.c file.
   *     --void USART1_IRQHandler(void)
   */

  usart_enable(USART1, TRUE);

  /* add user code begin usart1_init 2 */

  /* add user code end usart1_init 2 */
}

/* add user code begin 1 */
/* support printf function, usemicrolib is unnecessary */
#if (__ARMCC_VERSION > 6000000)
  __asm (".global __use_no_semihosting\n\t");
  void _sys_exit(int x)
  {
    x = x;
  }
  /* __use_no_semihosting was requested, but _ttywrch was */
  void _ttywrch(int ch)
  {
    ch = ch;
  }
  FILE __stdout;
#else
 #ifdef __CC_ARM
  #pragma import(__use_no_semihosting)
  struct __FILE
  {
    int handle;
  };
  FILE __stdout;
  void _sys_exit(int x)
  {
    x = x;
  }
  /* __use_no_semihosting was requested, but _ttywrch was */
  void _ttywrch(int ch)
  {
    ch = ch;
  }
 #endif
#endif

#if defined (__GNUC__) && !defined (__clang__)
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

/**
  * @brief  retargets the c library printf function to the usart.
  * @param  none
  * @retval none
  */
PUTCHAR_PROTOTYPE
{
  while(usart_flag_get(USART1, USART_TDBE_FLAG) == RESET);
  usart_data_transmit(USART1, (uint16_t)ch);
  while(usart_flag_get(USART1, USART_TDC_FLAG) == RESET);
  return ch;
}

#if (defined (__GNUC__) && !defined (__clang__)) || (defined (__ICCARM__))
#if defined (__GNUC__) && !defined (__clang__)
int _write(int fd, char *pbuffer, int size)
#elif defined ( __ICCARM__ )
#pragma module_name = "?__write"
int __write(int fd, char *pbuffer, int size)
#endif
{
  for(int i = 0; i < size; i ++)
  {
    while(usart_flag_get(PRINT_UART, USART_TDBE_FLAG) == RESET);
    usart_data_transmit(PRINT_UART, (uint16_t)(*pbuffer++));
    while(usart_flag_get(PRINT_UART, USART_TDC_FLAG) == RESET);
  }

  return size;
}
#endif

/* add user code end 1 */
