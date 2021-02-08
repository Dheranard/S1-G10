/*Base register adddress header file*/
#include "stm32l1xx.h"
/*Library related header files*/
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_utils.h"
#include "stm32l1xx_ll_system.h"
#include "stm32l152_glass_lcd.h"
#include "stm32l1xx_ll_tim.h"

// PA1:Rising , PA2:Trigger , PA3:Falling 


void SystemClock_Config(void);
void Show_Stop_mode(void); //set show led lcd and buzzer for Stop mode
void Show_Pass_mode(void); //set show led lcd and buzzer for Pass mode
void counting_sec(void);
void GPIO_Config(void);
void cal_distance(void);

void GPIO_Config(void)
{
		LL_GPIO_InitTypeDef timic_gpio;
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
		//GPIO_Config PA1 as alternate
		timic_gpio.Mode = LL_GPIO_MODE_ALTERNATE;
		timic_gpio.Pull = LL_GPIO_PULL_NO;
		timic_gpio.Pin = LL_GPIO_PIN_1;
		timic_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		timic_gpio.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
		timic_gpio.Alternate = LL_GPIO_AF_1;
		LL_GPIO_Init(GPIOA, &timic_gpio);
		//GPIO_Config PA3 as alternate
		timic_gpio.Pin = LL_GPIO_PIN_3;
		LL_GPIO_Init(GPIOA, &timic_gpio);
		//GPIO_Config PA2 as output
		timic_gpio.Mode = LL_GPIO_MODE_OUTPUT;
		timic_gpio.Pull = LL_GPIO_PULL_NO;
		timic_gpio.Pin = LL_GPIO_PIN_2;
		timic_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		timic_gpio.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
		LL_GPIO_Init(GPIOA, &timic_gpio);
	
		LL_GPIO_InitTypeDef GPIO_InitSt; 
		//enable clock to GPIOA and GPIOB
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
		//set outout type, pull, and speed
		GPIO_InitSt.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitSt.Pull = LL_GPIO_PULL_NO;
		GPIO_InitSt.Speed = LL_GPIO_SPEED_FREQ_HIGH;
		//set input mode for PA12
		GPIO_InitSt.Mode = LL_GPIO_MODE_INPUT;
		GPIO_InitSt.Pin = LL_GPIO_PIN_12;
		LL_GPIO_Init(GPIOA, &GPIO_InitSt);
		//set output mode for PB6,7,5
		GPIO_InitSt.Mode = LL_GPIO_MODE_OUTPUT;
		GPIO_InitSt.Pin = LL_GPIO_PIN_6;
		LL_GPIO_Init(GPIOB, &GPIO_InitSt);
	
		GPIO_InitSt.Mode = LL_GPIO_MODE_OUTPUT;
		GPIO_InitSt.Pin = LL_GPIO_PIN_7;
		LL_GPIO_Init(GPIOB, &GPIO_InitSt);
	
		GPIO_InitSt.Mode = LL_GPIO_MODE_OUTPUT;
		GPIO_InitSt.Pin = LL_GPIO_PIN_5;
		LL_GPIO_Init(GPIOA, &GPIO_InitSt);
	
	
}
	
void TIMx_IC_Config(void)
{
		LL_TIM_IC_InitTypeDef timic;
	
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
		
		//TIM_IC Configure CH2
		timic.ICActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;
		timic.ICFilter = LL_TIM_IC_FILTER_FDIV1_N2;
		timic.ICPolarity = LL_TIM_IC_POLARITY_RISING;
		timic.ICPrescaler = LL_TIM_ICPSC_DIV1;
		LL_TIM_IC_Init(TIM2, LL_TIM_CHANNEL_CH2, &timic);
		
		NVIC_SetPriority(TIM2_IRQn, 0);
		
		NVIC_EnableIRQ(TIM2_IRQn);
		LL_TIM_EnableIT_CC2(TIM2);
		LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);
		
		//TIM_IC Configure CH4
		timic.ICActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;
		timic.ICFilter = LL_TIM_IC_FILTER_FDIV1_N2;
		timic.ICPolarity = LL_TIM_IC_POLARITY_FALLING;
		timic.ICPrescaler = LL_TIM_ICPSC_DIV1;
		LL_TIM_IC_Init(TIM2, LL_TIM_CHANNEL_CH4, &timic);
		
		NVIC_SetPriority(TIM2_IRQn, 0);
		
		NVIC_EnableIRQ(TIM2_IRQn);
		LL_TIM_EnableIT_CC4(TIM2);
		LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH4);
		
		
		LL_TIM_EnableCounter(TIM2);
}

uint16_t Val1 = 0;
uint16_t Val2 = 0;
uint16_t Echo_pulse = 0;
float period = 0;
float distance = 0;
uint8_t state = 0;

uint32_t TIM2CLK;
uint32_t PSC;
uint32_t IC4PSC;

char disp_str[7];


int main()
{
	
		SystemClock_Config();
		TIMx_IC_Config();
		GPIO_Config();
		//send trigger
		LL_GPIO_SetOutputPin(GPIOA,LL_GPIO_PIN_2);
		LL_mDelay(1);
		LL_GPIO_ResetOutputPin(GPIOA,LL_GPIO_PIN_2);
		LCD_GLASS_Init();
		
		while(1)
		{
			cal_distance();
			
			if(distance >= 0.1){
				LL_GPIO_SetOutputPin(GPIOB,LL_GPIO_PIN_7);
			}
			else{
				LL_GPIO_SetOutputPin(GPIOB,LL_GPIO_PIN_6);
			}
			state = 0;
			
			/*if(uhICIndex == 2)
			{
					//Period calculation
					PSC = LL_TIM_GetPrescaler(TIM2) + 1;
					TIM2CLK = SystemCoreClock / PSC;
					IC1PSC = __LL_TIM_GET_ICPSC_RATIO(LL_TIM_IC_GetPrescaler(TIM2, LL_TIM_CHANNEL_CH1));
					
					period = (uwDiff*(PSC) * 1.0*10e3) / (TIM2CLK *IC1PSC * 1.0); //calculate uptime period ms
					uhICIndex = 0;
			}*/
		}
}

void TIM2_IRQHandler(void)
{	  
	if(state == 0)
	{
		if(LL_TIM_IsActiveFlag_CC2(TIM2) == SET)
		{
			LL_TIM_ClearFlag_CC2(TIM2);
			Val1 = LL_TIM_IC_GetCaptureCH2(TIM2);
			state = 1;
		}
	}		
	else if(state == 1)
	{
		if(LL_TIM_IsActiveFlag_CC4(TIM2) == SET)
		{
			LL_TIM_ClearFlag_CC4(TIM2);
			Val2 = LL_TIM_IC_GetCaptureCH4(TIM2);
			if(Val2 > Val1)
						Echo_pulse = Val2 - Val1;
			else if(Val2 < Val1)
						Echo_pulse = ((LL_TIM_GetAutoReload(TIM2) - Val1) + Val2) + 1;
			state = 2;
		}
	}
	
	/*if(LL_TIM_IsActiveFlag_CC1(TIM2) == SET)
		{
			LL_TIM_ClearFlag_CC1(TIM2);
				//Detect 1st rising edge
			if(uhICIndex == 0)
			{
				uwIC1 = LL_TIM_IC_GetCaptureCH1(TIM2);
				uhICIndex = 1;
			}
			//Detect 2nd rising edge
			else if(uhICIndex == 1)
			{
					uwIC2 = LL_TIM_IC_GetCaptureCH1(TIM2);
					
					if(uwIC2 > uwIC1)
						uwDiff = uwIC2 - uwIC1;
					else if(uwIC2 < uwIC1)
						uwDiff = ((LL_TIM_GetAutoReload(TIM2) - uwIC1) + uwIC2) + 1;
					uhICIndex = 2;

			}*/

}

void cal_distance(void){
	
	if(state == 2)
			{
				PSC = LL_TIM_GetPrescaler(TIM2) + 1;
				TIM2CLK = SystemCoreClock / PSC;
				IC4PSC = __LL_TIM_GET_ICPSC_RATIO(LL_TIM_IC_GetPrescaler(TIM2, LL_TIM_CHANNEL_CH4));
				period = (Echo_pulse*(PSC) * 1.0) / (TIM2CLK *IC4PSC * 1.0); //calculate uptime period (s) as PA3
				distance = period*340/2;//calculate distance (m) 
				//state = 0;
				
			}
}

void counting_sec(void) //counting number
{
	for (int i=10;i>0;--i)
	{
		sprintf(disp_str,"PASS%d",i); 
		LCD_GLASS_DisplayString((uint8_t*)disp_str);
		LL_mDelay(800);
		
		//reset lcd showing	
		sprintf(disp_str,"      ");
		LCD_GLASS_DisplayString((uint8_t*)disp_str);
	}
}

void Show_Stop_mode(void) //set show led lcd and buzzer for Stop mode
{
	//show "stop" at lcd
	sprintf(disp_str,"Stop"); 
	LCD_GLASS_DisplayString((uint8_t*)disp_str);
	//switch status PB 6 ,7
	LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_6);
	LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_7);
	//enable led at PB6 and disable led at PB7
	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_6);
	LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_7);
	LL_mDelay(100);
	//reset lcd showing	
	sprintf(disp_str,"    ");
	LCD_GLASS_DisplayString((uint8_t*)disp_str);
}

void Show_Pass_mode(void)
{
	//wait for enable buzzer
	LL_mDelay(1000);
	//enable buzzer
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
	//wait for show "PASS" at lcd
	LL_mDelay(1000);
	sprintf(disp_str,"PASS");
	LCD_GLASS_DisplayString((uint8_t*)disp_str);
	//enable led at PB7 and disable led at PB6
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);	
	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_7);
	LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_6);
	//counting number
	counting_sec();//counting number
	LL_mDelay(1000); //set counting for PASS
	//enable buzzer for 1 sec
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
	LL_mDelay(1000);
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
	//reset lcd showing	
	sprintf(disp_str,"    ");
	LCD_GLASS_DisplayString((uint8_t*)disp_str);
}


void SystemClock_Config(void)
{
  /* Enable ACC64 access and set FLASH latency */ 
  LL_FLASH_Enable64bitAccess();; 
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

  /* Set Voltage scale1 as MCU will run at 32MHz */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  
  /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
  while (LL_PWR_IsActiveFlag_VOSF() != 0)
  {
  };
  
  /* Enable HSI if not already activated*/
  if (LL_RCC_HSI_IsReady() == 0)
  {
    /* HSI configuration and activation */
    LL_RCC_HSI_Enable();
    while(LL_RCC_HSI_IsReady() != 1)
    {
    };
  }
  
	
  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3);

  LL_RCC_PLL_Enable();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  };
  
  /* Sysclk activation on the main PLL */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  };
  
  /* Set APB1 & APB2 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  /* Set systick to 1ms in using frequency set to 32MHz                             */
  /* This frequency can be calculated through LL RCC macro                          */
  /* ex: __LL_RCC_CALC_PLLCLK_FREQ (HSI_VALUE, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3); */
  LL_Init1msTick(32000000);
  
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(32000000);
}