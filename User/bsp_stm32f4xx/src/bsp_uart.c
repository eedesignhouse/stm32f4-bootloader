#include <bsp.h>

/*
*********************************************************************************************************
*	�� �� ��: bsp_init_uart
*	����˵��: ��ʼ��CPU��USART1����Ӳ���豸��δ�����жϡ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_init_uart(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
#define	USART1_ON
	
#ifdef	USART1_ON
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#if	0	/* ARMFLY */
	/* ����1 TX = PA9   RX = PA10 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	/* ���� USART Tx Ϊ���ù��� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	/* �������Ϊ���� */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* �ڲ���������ʹ�� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	/* ����ģʽ */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* ���� USART Rx Ϊ���ù��� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#else
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

	/* ���� USART Tx Ϊ���ù��� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	/* �������Ϊ���� */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* �ڲ���������ʹ�� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	/* ����ģʽ */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* ���� USART Rx Ϊ���ù��� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif

	/* ��2���� ���ô���Ӳ������ */
	USART_InitStructure.USART_BaudRate = 115200;	/* ������ */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

//	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	/* ʹ��USART1�ж� */
	USART_Cmd(USART1, ENABLE);		/* ʹ�ܴ��� */

	/* CPU��Сȱ�ݣ��������úã����ֱ��Send�����1���ֽڷ��Ͳ���ȥ
		�����������1���ֽ��޷���ȷ���ͳ�ȥ������ */
	USART_ClearFlag(USART1, USART_FLAG_TC);     /* �巢����ɱ�־��Transmission Complete flag */


#endif	
}

void bsp_init_nvic(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/***********************************************************************
*	�ض���printf
************************************************************************/
int fputc(int ch, FILE *f)
{
	/* дһ���ֽڵ�USART1 */
	USART_SendData(USART1, (uint8_t) ch);

	/* �ȴ����ͽ��� */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{}

	return ch;
}

/************************************************************************
*	�ض���scanf
************************************************************************/
int fgetc(FILE *f)
{
	/* �ȴ�����1�������� */
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);

	return (int)USART_ReceiveData(USART1);
}

/************************************************************************
*	send a char
************************************************************************/
int serial_putc (int ch)
{
	/* дһ���ֽڵ�USART1 */
	USART_SendData(USART1, (uint8_t) ch);

	/* �ȴ����ͽ��� */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{}

	return ch;
}


/************************************************************************
 *	send strings
 ***********************************************************************/
void serial_puts (const char *s)
{
	while(*s) {
		serial_putc(*s++);
	}
}

/************************************************************************
 *receive a char from uart
 ***********************************************************************/

int serial_getc (void)
{
	/* �ȴ�����1�������� */
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
	
	return (int)USART_ReceiveData(USART1);
}

/************************************************************************
 * data received already?
* 1: received; 0: data register is empty
 ***********************************************************************/
int serial_tstc (void)
{
	if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET) {
		return 1;
	} else {
		return 0;
	}
	//return USART_GetFlagStatus(USART1, USART_FLAG_RXNE);
}
