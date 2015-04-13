#include "bsp.h"

static __IO u32 TimingDelay;


void bsp_systick_init(void)
{
	/* SystemFrequency / 1000    1ms�ж�һ��
	 * SystemFrequency / 100000	 10us�ж�һ��
	 * SystemFrequency / 1000000 1us�ж�һ��
	 */
	if (SysTick_Config(SystemCoreClock / 1000000))	// ST3.5.0��汾
	{ 
		/* Capture error */ 
		while (1);
	}
	
	SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
}

void udelay(__IO u32 nTime)
{ 
#if 0
	TimingDelay = nTime;	

	// ʹ�ܵδ�ʱ��  
	SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;

	while(TimingDelay != 0);
#else
	uint8_t i;
	
	while(nTime--)
		for(i = 32; i >0; i--);
	
	
#endif
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval : None
  */
void SysTick_Handler(void)
{
	if (TimingDelay != 0x00)
	{ 
	TimingDelay--;
	}	
}
