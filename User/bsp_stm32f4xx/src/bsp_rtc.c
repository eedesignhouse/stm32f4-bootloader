#include "bsp.h"

/* ѡ��RTC��ʱ��Դ */
#define RTC_CLOCK_SOURCE_LSE       /* LSE */
//#define RTC_CLOCK_SOURCE_LSI     /* LSI */ 




/*
*********************************************************************************************************
*	�� �� ��: RTC_Config
*	����˵��: ��������ʱ�������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RTC_Config(void)
{
	RTC_InitTypeDef  RTC_InitStructure;	
	RTC_TimeTypeDef  RTC_TimeStructure;
	RTC_DateTypeDef  RTC_DateStructure;
	/* ��������RTC��Ƶ */
	__IO uint32_t uwAsynchPrediv = 0;
	__IO uint32_t uwSynchPrediv = 0;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);	/* ʹ��PWRʱ�� */	
	PWR_BackupAccessCmd(ENABLE);							/* ������ʱ��ݼĴ��� */

#if defined (RTC_CLOCK_SOURCE_LSI)					/* ѡ��LSI��Ϊʱ��Դ */	 
	RCC_LSICmd(ENABLE);												/* Enable the LSI OSC */	
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);	/* Wait till LSI is ready */

	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);		/* ѡ��RTCʱ��Դ */
	RCC_RTCCLKCmd(ENABLE);										/* ʹ��RTCʱ�� */	
	RTC_WaitForSynchro();											/* �ȴ�RTC APB�Ĵ���ͬ�� */
	
#elif defined (RTC_CLOCK_SOURCE_LSE)				/* ѡ��LSE��ΪRTCʱ�� */
	RCC_LSEConfig(RCC_LSE_ON);								/* ʹ��LSE����  */	  
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);	/* �ȴ����� */
	
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		/* ѡ��RTCʱ��Դ */	
	RCC_RTCCLKCmd(ENABLE);										/* ʹ��RTCʱ�� */
	RTC_WaitForSynchro();											/* �ȴ�RTC APB�Ĵ���ͬ�� */

#else
	#error Please select the RTC Clock source inside the main.c file
#endif 

	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x9527) {
		RTC_WriteProtectionCmd(DISABLE);
    RTC_EnterInitMode();
		
		/* ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
		uwSynchPrediv = 0xFF;
		uwAsynchPrediv = 0x7F;

//		RTC_TimeStampCmd(RTC_TimeStampEdge_Falling, ENABLE);  /* ʹ��ʱ��� */
//		RTC_TimeStampPinSelection(RTC_TamperPin_PC13);	

		/* ����RTC���ݼĴ����ͷ�Ƶ��  */
		RTC_InitStructure.RTC_AsynchPrediv = uwAsynchPrediv;
		RTC_InitStructure.RTC_SynchPrediv = uwSynchPrediv;
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;	
		RTC_Init(&RTC_InitStructure);						/* RTC��ʼ�� */

		/* ���������պ����� */
		RTC_DateStructure.RTC_Year = 0x14;
		RTC_DateStructure.RTC_Month = RTC_Month_December;
		RTC_DateStructure.RTC_Date = 0x23;
		RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Tuesday;
		RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);

		/* ����ʱ���룬�Լ���ʾ��ʽ */
		RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
		RTC_TimeStructure.RTC_Hours   = 0x00;
		RTC_TimeStructure.RTC_Minutes = 0x00;
		RTC_TimeStructure.RTC_Seconds = 0x00; 
		RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);

		RTC_ExitInitMode();
    RTC_WriteBackupRegister(RTC_BKP_DR0,0x9527);
    RTC_WriteProtectionCmd(ENABLE);
    RTC_WriteBackupRegister(RTC_BKP_DR0,0x9527);
	}
	PWR_BackupAccessCmd(DISABLE);
}


void rtc_get(struct rtc_time *tm)
{
	unsigned int have_retried = 0;
	RTC_TimeTypeDef  RTC_TimeStructure;
	RTC_DateTypeDef  RTC_DateStructure;

retry_get_time:	
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);/* �õ�ʱ�� */
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);/* �õ����� */
	
	tm->tm_sec = RTC_TimeStructure.RTC_Seconds;
	tm->tm_min = RTC_TimeStructure.RTC_Minutes;
	tm->tm_hour = RTC_TimeStructure.RTC_Hours;

	/* the only way to work out wether the system was mid-update
	 * when we read it is to check the second counter, and if it
	 * is zero, then we re-try the entire read
	 */
	if (tm->tm_sec == 0 && !have_retried) {
		have_retried = 1;
		goto retry_get_time;
	}	
	
	tm->tm_wday = RTC_DateStructure.RTC_WeekDay;
	tm->tm_mday = RTC_DateStructure.RTC_Date;
	tm->tm_mon = RTC_DateStructure.RTC_Month;
	tm->tm_year = RTC_DateStructure.RTC_Year;

}

void rtc_set(struct rtc_time *tm)
{
	RTC_TimeTypeDef  RTC_TimeStructure;
	RTC_DateTypeDef  RTC_DateStructure;

	RTC_TimeStructure.RTC_H12 = RTC_H12_AM;
	RTC_TimeStructure.RTC_Seconds = tm->tm_sec;
	RTC_TimeStructure.RTC_Minutes = tm->tm_min;
	RTC_TimeStructure.RTC_Hours = tm->tm_hour;
	
	RTC_DateStructure.RTC_WeekDay = tm->tm_wday;
	RTC_DateStructure.RTC_Date = tm->tm_mday;
	RTC_DateStructure.RTC_Month = tm->tm_mon;
	RTC_DateStructure.RTC_Year = (tm->tm_year % 100);

	PWR_BackupAccessCmd(ENABLE);	
	RTC_WriteProtectionCmd(DISABLE);
	RTC_EnterInitMode();
	
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
	
	RTC_ExitInitMode();
	RTC_WriteProtectionCmd(ENABLE);
	PWR_BackupAccessCmd(DISABLE);
}

void rtc_reset(void)
{
	return;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitRTC
*	����˵��: ��ʼ��RTC
*	��    �Σ���
*	�� �� ֵ: ��		        
*********************************************************************************************************
*/
void bsp_InitRTC(void)
{
	/* RTC ����  */
	RTC_Config();
}

