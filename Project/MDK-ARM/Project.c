/*Base register adddress header file*/
#include "stm32l1xx.h"
/*Library related header files*/
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_utils.h"
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_tim.h"

void SystemClock_Config(void);
void Call_distance(void);
void Show_Stop_mode(void); //set show led lcd and buzzer for Stop mode
void Show_Pass_mode(void); //set show led lcd and buzzer for Pass mode
void counting_sec(void);

void TIMBase_Config(void)
{
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
	LL_TIM_EnableCounter(TIM2);
}

void GPIO_Config(void)
{
		LL_GPIO_InitTypeDef hcsr04_gpio;
		
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
		
		hcsr04_gpio.Mode = LL_GPIO_MODE_OUTPUT;
		hcsr04_gpio.Pull = LL_GPIO_PULL_NO;
		hcsr04_gpio.Pin = LL_GPIO_PIN_2;
		hcsr04_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		hcsr04_gpio.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
		LL_GPIO_Init(GPIOA, &hcsr04_gpio);
	
		hcsr04_gpio.Mode = LL_GPIO_MODE_INPUT;
		hcsr04_gpio.Pin = LL_GPIO_PIN_1;
		hcsr04_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		hcsr04_gpio.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
		LL_GPIO_Init(GPIOA, &hcsr04_gpio);
	
		LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_2);
	
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

uint16_t rise_timestamp = 0;
uint16_t fall_timestamp = 0;
uint16_t up_cycle = 0;

uint8_t state = 0;
float period = 0;
float distance = 0;

uint32_t TIM2CLK;
uint32_t PSC;

char disp_str[7];
int main()
{
		SystemClock_Config();
		GPIO_Config();
		TIMBase_Config();

		while(1){
			Call_distance();
			if(distance >= 0.1){
				LL_GPIO_SetOutputPin(GPIOB,LL_GPIO_PIN_6);
				LL_GPIO_ResetOutputPin(GPIOB,LL_GPIO_PIN_7);
			}
			else{
				LL_GPIO_SetOutputPin(GPIOB,LL_GPIO_PIN_7);
				LL_GPIO_ResetOutputPin(GPIOB,LL_GPIO_PIN_6);}
		}
	}
			


void Call_distance(void){
	switch(state)
			{
				case 0:
					//Trigger measurement
					LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_2);
					LL_mDelay(1);
					LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_2);
					state = 1;
				break;
				
				case 1:
					if(LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_1))
					{
						rise_timestamp = LL_TIM_GetCounter(TIM2);
						state = 2;
					}
				break;
					
				case 2:
					if(LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_1) == RESET)
					{
						fall_timestamp = LL_TIM_GetCounter(TIM2);
						//Calculate uptime
						if(fall_timestamp > rise_timestamp)
						{
							up_cycle = fall_timestamp - rise_timestamp;
						}
						else if(fall_timestamp < rise_timestamp)
						{
							up_cycle = (LL_TIM_GetAutoReload(TIM2) - rise_timestamp) + fall_timestamp + 1; 
						}
						else
						{
							//cannot measure at this freq
							up_cycle = 0;
						}
						
						if(up_cycle != 0)
						{
							PSC = LL_TIM_GetPrescaler(TIM2) + 1;
							TIM2CLK = SystemCoreClock / PSC;
							
							period = (up_cycle*(PSC) * 1.0) / (TIM2CLK * 1.0); //calculate uptime period
							distance = (period * 340) / 2; //meter unit
						}
						state = 0;
					}
				break;
					
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