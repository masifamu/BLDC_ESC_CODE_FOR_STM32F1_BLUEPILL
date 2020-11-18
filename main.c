#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include "string.h"
#include "stdio.h"

#include "sysclk.h"
#include "adc_dma.h"
#include "bldc.h"

#ifdef UART_COMM_DEBUG
char printDataString[50] = "buffer here\r\n";//{'\0',};
#endif

int main(void)
{
	uint16_t pwmWidth=0;
	uint16_t throtle=0;
	// Set clock
	SetSysClockTo72();
	// ADC Init
	ADC_DMA_init();
	// TIM1, outputs, inputs, interrupts, etc. Init
	BLDC_Init();

	// Reverse pin Init
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_0);

	// Initialize USART
	#ifdef UART_COMM_DEBUG
    usart_init();
    USARTSend(" Hello.\r\nUSART1 is ready.\r\n");
	#endif

    while(1)
    {
    	throtle=ADCBuffer[0];

		#ifdef UART_COMM_DEBUG
    	sniprintf(printDataString,50, "Throttle = %d\n\r", throtle);
    	USARTSend(printDataString);
		#endif

    	if (throtle > BLDC_ADC_START) {
    		if (BLDC_MotorGetSpin() == BLDC_STOP) {
    			// Check Reverse pin
    			if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) != 0) {
    				// Forward
    				BLDC_MotorSetSpin(BLDC_CW);
    			}
    			else {
    				// Backward
    				BLDC_MotorSetSpin(BLDC_CCW);
    			}
    			BLDC_MotorCommutation(BLDC_HallSensorsGetPosition());
    		}
    		pwmWidth=BLDC_ADCToPWM(throtle);

    		#ifdef UART_COMM_DEBUG
    		sniprintf(printDataString,50, "pwm = %d\n\r", (uint16_t)pwmWidth);
    		USARTSend(printDataString);
			#endif

    		BLDC_SetPWM(pwmWidth);
    	}
    	else {
    		if (BLDC_MotorGetSpin() != BLDC_STOP) {
    			//meaning motor is still running
    			if (throtle < BLDC_ADC_STOP) {
					BLDC_MotorStop();
    			}
    		}
    	}
    }
}
