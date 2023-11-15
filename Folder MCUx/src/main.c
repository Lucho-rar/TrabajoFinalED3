/**
 *
 * Nombre del proyecto: RELOJ RUNNING
 * Autores:Robles Karen Yesica, Rodriguez Luciano
 * Descripcion: Se registra el rendimiento de la actividad deportiva, con un teclado se puede seleccionar
 * el inicio de la actividad,pausar,finalizar y trackear el rendimiento,
 * utilizamos el ADC y sensor de temperatura LM35 que simulan la toma de medicion de temperatura corporal del corredor,
 * uso de un TIMER en modo captura para el calculo de la frecuencia cardiaca,
 * otro TIMER que nos devulve el tiempo completo de la actividad y la distancia recorrida,
 * y por ultimo se utiliza el DMA  para tranferir los valores memoria a memoria  de la frecuencia cardiaca,velocidad,distancia,tiempo y temperatura de corredor.
 *
 */

#include "lpc17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_dac.h"
#include <stdio.h>
#include <stdlib.h>

#define INPUT 0
#define OUTPUT 1
#define SIZE 10
#define SIZE_KEY 16

#define PORT_ZERO (uint8_t) 0
#define PORT_TWO (uint8_t) 2
#define PIN_22		((uint32_t)(1<<22))
#define PIN_10		((uint32_t)(1<<10))

#define EINT0 (1<<0)
#define EINT1 (1<<1)
#define EINT2 (1<<2)
#define EINT3 (1<<3)
#define SET_PR 4999
#define temperatura 24
#define velocidad 10 //esta en km/h

#define DMA_TRANSFER_SIZE 10
void configGpio(void);
void configTimer(void);
void configADC(void);
void configIntExt(void);
void configDMA(void);
void configUART(void);
uint8_t get_key(void);
void delay(uint32_t times);
void multiplexar(uint8_t cod);
void config_GPIO(void);
void confDAC(void);
void portada();
void delay_sin();
void cargar_valor_a_wform(uint8_t v);
void formar_onda(void);
float temp=0;
uint32_t cont_timer0=0;
uint32_t distancia = 0;
uint8_t cont=0;
uint8_t anterior=0;
uint8_t actual=0;
uint8_t periodo=0;
uint8_t suma=0;
uint8_t promedio=0;
uint8_t frec_card[SIZE];
uint32_t frecCard=0;
uint8_t upflag=0;

uint8_t auxiliar_ult =0;


#define DMA_SIZE 60
#define NUM_SINE_SAMPLE 42
#define SINE_FREQ_IN_HZ 60
#define PCLK_DAC_IN_MHZ 25 //CCLK divided by 4
uint32_t dac_lut[NUM_SINE_SAMPLE];
/*			Wform	 para el DAC		*/
uint32_t dac_sine_lut[11]={750,0,3,54,42,900,283,105,823,923,1023};
uint32_t salida_dac[10] = {0,123,223,323,423,523,623,723,823,1023};		//En este arreglo se van a ir cargando los valores recibidos por UART



uint8_t unidad_segundos = 0;
uint8_t decena_segundos = 0;
uint8_t unidad_minutos = 0;
uint8_t decena_minutos = 0;
uint8_t unidad_hora = 0 ;
uint8_t saludo = 0;
uint8_t valoresDisplay[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0xD7,0x7F,0x67};
uint32_t hola[4] = {0b1101110,0b0111111 , 0b0111000 , 0b1101111};
uint8_t final_lut[4];
uint32_t DMASrc_Buffer[DMA_TRANSFER_SIZE];
uint32_t DMADst_Buffer[DMA_TRANSFER_SIZE];

int main(void) {
	configGpio();
	LPC_SC->EXTINT |= EINT0;
	configIntExt();
	configTimer();
	//configADC();
	//configDMA();
	configUART();
	portada();
	LPC_GPIO2->FIOCLR |= (1<<8);
	LPC_GPIO2->FIOCLR |= (1<<7);
	LPC_GPIO0->FIOCLR |= (1<<16);
	LPC_GPIO0->FIOCLR |= (1<<17);
	LPC_GPIO0->FIOSET |= (1<<15);
	while(saludo==0){

		LPC_GPIO2->FIOCLR |= (0x7F<<0);
		LPC_GPIO2->FIOSET |= hola[3];
		delay(5);
		multiplexar(1);
		LPC_GPIO2->FIOCLR |= (0x7F<<0);
		LPC_GPIO2->FIOSET |= hola[2];
		delay(5);
		multiplexar(2);
		LPC_GPIO2->FIOCLR |= (0x7F<<0);
		LPC_GPIO2->FIOSET |= hola[1];
		delay(5);
		multiplexar(3);
		LPC_GPIO2->FIOCLR |= (0x7F<<0);
		LPC_GPIO2->FIOSET |=  hola[0];
		delay(5);
		multiplexar(4);
		LPC_GPIO2->FIOCLR |= 0x7F;
		delay(5);
		multiplexar(5);
	}

	while(1){
		LPC_GPIO2->FIOCLR |= (0x7F<<0);
		LPC_GPIO2->FIOSET |= valoresDisplay[unidad_segundos];
		delay(5);
		multiplexar(1);
		LPC_GPIO2->FIOCLR |= (0x7F<<0);
		LPC_GPIO2->FIOSET |= valoresDisplay[decena_segundos];
		delay(5);
		multiplexar(2);
		LPC_GPIO2->FIOCLR |= (0x7F<<0);
		LPC_GPIO2->FIOSET |= valoresDisplay[unidad_minutos];
		delay(5);
		multiplexar(3);
		LPC_GPIO2->FIOCLR |= (0x7F<<0);
		LPC_GPIO2->FIOSET |= valoresDisplay[decena_minutos];
		delay(5);
		multiplexar(4);
		LPC_GPIO2->FIOCLR |= (0x7F<<0);
		LPC_GPIO2->FIOSET |= valoresDisplay[unidad_hora];
		delay(5);
		multiplexar(5);

	}
    return 0 ;
}
void configGpio(void){
    PINSEL_CFG_Type pinCfg;
    pinCfg.Portnum 	=	PINSEL_PORT_0;
    pinCfg.Pinnum	=	PINSEL_PIN_10;
    pinCfg.Pinmode	=	PINSEL_PINMODE_PULLUP;
    pinCfg.Funcnum	= 	PINSEL_FUNC_0;
    pinCfg.OpenDrain	=	PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&pinCfg);
	GPIO_SetDir( PORT_ZERO , PIN_10 , OUTPUT );

	//CONFIGURACION DEL PUERTO 2


	    pinCfg.Portnum 	=	PINSEL_PORT_2;
		pinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;

		for (uint8_t i = 0; i < 9; i++){ // Filas output
			pinCfg.Pinnum = i;
	        PINSEL_ConfigPin(&pinCfg);
		}
		GPIO_SetDir(PORT_TWO, 0x1FF, OUTPUT);						//Agregue los pines de multiplexado

		pinCfg.Portnum = PINSEL_PORT_0;

		for (uint8_t i = 15 ; i < 18 ; i++  ){
			pinCfg.Pinnum = i;
			PINSEL_ConfigPin(&pinCfg);
		}
		GPIO_SetDir(0, 0x7 <<15, OUTPUT);

		/*			Pines de UART 			*/

		PINSEL_CFG_Type cfg;
		cfg.Funcnum = 2;
		cfg.OpenDrain =0;
		cfg.Portnum = 0;
		cfg.Pinnum = 0;
		PINSEL_ConfigPin(&cfg);
		cfg.Pinnum =1;
		PINSEL_ConfigPin(&cfg);


		/*  		PINSEL PARA DAC			*/
		PINSEL_CFG_Type pinsel_cfg;
		pinsel_cfg.Portnum = 0;
		pinsel_cfg.Pinnum = 26;
		pinsel_cfg.Funcnum = 2;
		pinsel_cfg.Pinmode = 0;
		pinsel_cfg.OpenDrain = 0;
		PINSEL_ConfigPin(&pinsel_cfg);
}

void confDAC(void){
	uint32_t tmp;

	DAC_CONVERTER_CFG_Type DAC_ConverterConfigStruct;
	DAC_ConverterConfigStruct.CNT_ENA =SET;			// Habilitamos el contador de DMA
	DAC_ConverterConfigStruct.DMA_ENA = SET;		// Habilitamos DMA para DAC

	DAC_Init(LPC_DAC);


	tmp = (PCLK_DAC_IN_MHZ*1000000)/(SINE_FREQ_IN_HZ*NUM_SINE_SAMPLE);
	DAC_SetDMATimeOut(LPC_DAC,tmp);

	DAC_ConfigDAConverterControl(LPC_DAC, &DAC_ConverterConfigStruct);
	return;
}
void configTimer(){
	    TIM_TIMERCFG_Type	timCfg;
		TIM_MATCHCFG_Type	match;

		timCfg.PrescaleOption	=	TIM_PRESCALE_USVAL;
		timCfg.PrescaleValue		=	100000;
		/***********************************************
		 *
		 *          CONFIGURACION DEL TIMER 0
		 *
		 *  Para el calculo del tiempo recorrido, hacemos match cada 1 [s]
		***********************************************/
		 match.MatchChannel		=	0;
		 match.IntOnMatch		=	ENABLE;
		 match.ResetOnMatch		=	ENABLE;
		 match.StopOnMatch		=	DISABLE;
		 match.ExtMatchOutputType	=	TIM_EXTMATCH_TOGGLE;
		 match.MatchValue			=	10;

		 TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timCfg);
		 TIM_ConfigMatch(LPC_TIM0, &match);

		 /***********************************************
		  *
		  *    CONFIGURACION DEL TIMER 2
		  *
		  *      Para la frecuencia cardiaca
		  ***********************************************/

		 //Configuracion del pin P0.4 como CAP2.0
	       PINSEL_CFG_Type pinCfg;

		 	pinCfg.Portnum = 0;
		 	pinCfg.Pinnum = 4;
		 	pinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
		 	pinCfg.Funcnum = 3;
		 	pinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;

		 	PINSEL_ConfigPin(&pinCfg);

		 	TIM_CAPTURECFG_Type capCfg;

		 	capCfg.CaptureChannel = 0;
		 	capCfg.RisingEdge = DISABLE;
		 	capCfg.FallingEdge = ENABLE; //Se habilita por flanco de bajada
		 	capCfg.IntOnCaption = ENABLE;

		 	TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &timCfg);
		 	TIM_ConfigCapture(LPC_TIM2, &capCfg);


        /***********************************************
		*
		*          CONFIGURACION DEL TIMER 1
		*
		* Para el uso del ADC en modo No Burst MAT1.0 cada 10 seg
		***********************************************/


		//LPC_PINCON->PINSEL0|=(3<<20);

		match.MatchChannel		=	0;
		match.IntOnMatch			=	DISABLE;
		match.ResetOnMatch		=	ENABLE;
		match.StopOnMatch		=	DISABLE;
		match.ExtMatchOutputType	=	TIM_EXTMATCH_TOGGLE;
		match.MatchValue			=	100;

		TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &timCfg);
		TIM_ConfigMatch(LPC_TIM1, &match);

		//Habilita interrupciones
		NVIC_EnableIRQ(TIMER0_IRQn);
		NVIC_EnableIRQ(TIMER2_IRQn);

		//Mayor prioridad al match que al capture
		//	NVIC_SetPriority(TIMER3_IRQn, 4);
		//	NVIC_SetPriority(TIMER2_IRQn, 8);

}
void configIntExt(void){
	//Configuracion del pin P2.10 como EINT0
	    PINSEL_CFG_Type pinCfg;
	    pinCfg.Portnum =PINSEL_PORT_2;
	    pinCfg.Pinnum	=	10;
		pinCfg.Funcnum = 1;
		pinCfg.OpenDrain = 0;
		pinCfg.Pinmode = 0;
		PINSEL_ConfigPin(&pinCfg);

		EXTI_InitTypeDef extCfg;
		extCfg.EXTI_Line=EXTI_EINT0;
		extCfg.EXTI_Mode=EXTI_MODE_EDGE_SENSITIVE;
		extCfg.EXTI_polarity=EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
		EXTI_Config(&extCfg);
		LPC_SC->EXTINT|=EINT0;
		EXTI_ClearEXTIFlag(EXTI_EINT0);
		NVIC_EnableIRQ(EINT0_IRQn);

		//Configuracion del pin P2.11 como EINT1
		//PINSEL_CFG_Type pinCfg;
	   // pinCfg.Portnum =PINSEL_PORT_2;
	    pinCfg.Pinnum	=	11;
		pinCfg.Funcnum = 1;
		pinCfg.OpenDrain = 0;
		pinCfg.Pinmode = 0;
		PINSEL_ConfigPin(&pinCfg);

		//EXTI_InitTypeDef extCfg;
		extCfg.EXTI_Line=EXTI_EINT1;
		extCfg.EXTI_Mode=EXTI_MODE_EDGE_SENSITIVE;
		extCfg.EXTI_polarity=EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
		EXTI_Config(&extCfg);
		LPC_SC->EXTINT|=EINT1;
		EXTI_ClearEXTIFlag(EXTI_EINT1);
		NVIC_EnableIRQ(EINT1_IRQn);

		//Configuracion del pin P2.12 como EINT2
	/*	pinCfg.Pinnum	=	12;
		pinCfg.Funcnum = 1;
		pinCfg.OpenDrain = 0;
		pinCfg.Pinmode = 0;
		PINSEL_ConfigPin(&pinCfg);

				//EXTI_InitTypeDef extCfg;
		extCfg.EXTI_Line=EXTI_EINT2;
		extCfg.EXTI_Mode=EXTI_MODE_EDGE_SENSITIVE;
		extCfg.EXTI_polarity=EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
		EXTI_Config(&extCfg);
		LPC_SC->EXTINT|=EINT2;
		EXTI_ClearEXTIFlag(EXTI_EINT2);
		NVIC_EnableIRQ(EINT2_IRQn);

		//Configuracion del pin P2.13 como EINT3
		pinCfg.Pinnum	=	13;
		pinCfg.Funcnum = 1;
		pinCfg.OpenDrain = 0;
		pinCfg.Pinmode = 0;
		PINSEL_ConfigPin(&pinCfg);

				//EXTI_InitTypeDef extCfg;
		extCfg.EXTI_Line=EXTI_EINT3;
		extCfg.EXTI_Mode=EXTI_MODE_EDGE_SENSITIVE;
		extCfg.EXTI_polarity=EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
		EXTI_Config(&extCfg);
		EXTI_ClearEXTIFlag(EXTI_EINT3);
		LPC_SC->EXTINT|=EINT3;
		NVIC_EnableIRQ(EINT3_IRQn);
*/
	  //  LPC_PINCON->PINSEL4 |= (1 << 20);

	   // LPC_SC->EXTMODE |= EINT0;
	   // LPC_SC->EXTPOLAR &= ~(EINT0); //EINT2 falling edge
	   // LPC_SC->EXTINT |= EINT0;
	     NVIC_SetPriority(EINT0_IRQn, 1);

	    //NVIC_EnableIRQ(EINT0_IRQn);
}

void EINT0_IRQHandler(void){
	saludo = 1;

	uint8_t tmp[] = "\n\n\n¡El ejercicio ha comenzado!\n\n\n\r";
	UART_Send(LPC_UART3,tmp, sizeof(tmp),BLOCKING);
	uint8_t tmp1[] = "Distancia \t Pulsaciones (xMin) \t Tiempo\n\r";
	UART_Send(LPC_UART3,tmp1, sizeof(tmp1),BLOCKING);

	TIM_Cmd(LPC_TIM0, ENABLE);
	TIM_Cmd(LPC_TIM2, ENABLE);

	LPC_SC->EXTINT|=EINT0;

}
void EINT1_IRQHandler(void){

	TIM_Cmd(LPC_TIM0, DISABLE);
	TIM_Cmd(LPC_TIM2, DISABLE);
	TIM_ClearIntCapturePending(LPC_TIM0, TIM_MR0_INT);
	TIM_ClearIntPending(LPC_TIM2, TIM_CR0_INT);
	uint8_t cantB = 4;
	if(distancia<10){
		cantB++;
	}else if(distancia>10 && distancia <100){
		cantB +=2;
	}else if(distancia>100 && distancia<1000){
		cantB +=3;
	}else{
		cantB += 4;
	}

	if(frecCard<10){
		cantB++;
	}else if(frecCard>10 && frecCard <100){
		cantB +=2;
	}else if(frecCard>100 && frecCard<1000){
		cantB +=3;
	}else{
		cantB += 4;
	}

	if(cont_timer0<10){
		cantB++;
	}else if(cont_timer0>10 && cont_timer0 <100){
		cantB +=2;
	}else if(cont_timer0>100 && cont_timer0<1000){
		cantB +=3;
	}else{
		cantB += 4;
	}
	char buffer2[cantB];
	sprintf(buffer2, "*%d*%d*%d*", distancia,frecCard,cont_timer0);
	UART_Send(LPC_UART3, (uint8_t*)buffer2, sizeof(buffer2), BLOCKING);

	distancia =0;
	cont_timer0 =0;

	DMASrc_Buffer[0] = distancia; //distancia recorrida en m
	DMASrc_Buffer[1] = cont_timer0; //tiempo recorrido en s
	DMASrc_Buffer[2] = velocidad;   //velocidad en km/h
	DMASrc_Buffer[3] = frecCard;    // frecuencia en lat/min
	DMASrc_Buffer[4] = temperatura; //temperatura en °C


	/*Reseteo el reloj */
	unidad_segundos = 0; decena_segundos = 0; unidad_minutos=0; decena_minutos=0; unidad_hora=0;


	LPC_SC->EXTINT|=EINT1;
}
void configDMA(void)
{
	GPDMA_LLI_Type DMA_LLI_Struct;
	//Prepare DMA link list item structure
	DMA_LLI_Struct.SrcAddr= (uint32_t)dac_lut;		//ente de los datos (array de datos)
	DMA_LLI_Struct.DstAddr= (uint32_t)&(LPC_DAC->DACR);		// Origen de los datos (DAC)
	DMA_LLI_Struct.NextLLI= (uint32_t)&DMA_LLI_Struct;		// Siguiente item de lista (el mismo)
	DMA_LLI_Struct.Control= DMA_SIZE						// Tamaños de los campos
			| (2<<18) //source width 32 bit
			| (2<<21) //dest. width 32 bit
			| (1<<26) //source increment
			;
	/* GPDMA block section -------------------------------------------- */

	/* Initialize GPDMA controller */
	GPDMA_Init();
	GPDMA_Channel_CFG_Type GPDMACfg;
	// Setup GPDMA channel --------------------------------
	// channel 0
	GPDMACfg.ChannelNum = 0;
	// Source memory
	GPDMACfg.SrcMemAddr = (uint32_t)(dac_lut);
	// Destination memory - unused
	GPDMACfg.DstMemAddr = 0;					// EL DESTINO NO ES UNA POSICION DE MEMORIA
	// Transfer size
	GPDMACfg.TransferSize = DMA_SIZE;
	// Transfer width - unused
	GPDMACfg.TransferWidth = 0;
	// Transfer type
	GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;	// Memoria a Periferico
	// Source connection - unused
	GPDMACfg.SrcConn = 0;
	// Destination connection
	GPDMACfg.DstConn = GPDMA_CONN_DAC;
	// Linker List Item - unused
	GPDMACfg.DMALLI = (uint32_t)&DMA_LLI_Struct;	// Se asocia la lista anterior

	// Setup channel with given parameter
	GPDMA_Setup(&GPDMACfg);

	return;
}
void TIMER0_IRQHandler(void){
	 cont_timer0++;
	 	 distancia = velocidad * cont_timer0 * 0.28;
		 if(distancia <10){
			 char buffer2[3];
			 sprintf(buffer2, "\t%d\t", distancia);
			 UART_Send(LPC_UART3, (uint8_t*)buffer2, sizeof(buffer2), BLOCKING);
		 }else if(distancia>10 && distancia<100){
			 char buffer2[4];
			 sprintf(buffer2, "\t%d\t", distancia);
			 UART_Send(LPC_UART3, (uint8_t*)buffer2, sizeof(buffer2), BLOCKING);
		 }else if (distancia>100 && distancia<1000){
			 char buffer2[5];
			 sprintf(buffer2, "\t%d\t", distancia);
			 UART_Send(LPC_UART3, (uint8_t*)buffer2, sizeof(buffer2), BLOCKING);
		 }else if (distancia>1000){
			 char buffer2[6];
			 sprintf(buffer2, "\t%d\t", distancia);
			 UART_Send(LPC_UART3, (uint8_t*)buffer2, sizeof(buffer2), BLOCKING);
		 }

		 if(frecCard <10){
		 	 char buffer2[3];
		 	 sprintf(buffer2, "\t%d\t", frecCard);
		 	 UART_Send(LPC_UART3, (uint8_t*)buffer2, sizeof(buffer2), BLOCKING);
		 }else if(frecCard>10 && frecCard<100){
			 char buffer2[4];
			 sprintf(buffer2, "\t%d\t", frecCard);
			 UART_Send(LPC_UART3, (uint8_t*)buffer2, sizeof(buffer2), BLOCKING);
		 }
		 else if(frecCard>100){
		 	 char buffer2[5];
		 	 sprintf(buffer2, "\t%d\t", frecCard);
		 	 UART_Send(LPC_UART3, (uint8_t*)buffer2, sizeof(buffer2), BLOCKING);
		 }else if(frecCard>1000){
		 	 char buffer2[6];
		 	 sprintf(buffer2, "\t%d\t", frecCard);
		 	 UART_Send(LPC_UART3, (uint8_t*)buffer2, sizeof(buffer2), BLOCKING);
		}


		 if(cont_timer0 <10){
		 	 char buffer2[5];
		 	 sprintf(buffer2, "\t%d\t\n\r", cont_timer0);
		 	 UART_Send(LPC_UART3, (uint8_t*)buffer2, sizeof(buffer2), BLOCKING);
		 }else if(cont_timer0>10 && cont_timer0<100){
			 char buffer2[6];
			 sprintf(buffer2, "\t%d\t\n\r", cont_timer0);
			 UART_Send(LPC_UART3, (uint8_t*)buffer2, sizeof(buffer2), BLOCKING);
		 }
		 else if(cont_timer0>100){
		 	 char buffer2[7];
		 	 sprintf(buffer2, "\t%d\t\n\r", cont_timer0);
		 	 UART_Send(LPC_UART3, (uint8_t*)buffer2, sizeof(buffer2), BLOCKING);
		 }else if(cont_timer0>1000){
		 	 char buffer2[8];
		 	 sprintf(buffer2, "\t%d\t\n\r", cont_timer0);
		 	 UART_Send(LPC_UART3, (uint8_t*)buffer2, sizeof(buffer2), BLOCKING);
		}




		   	// x:59:59 ->incremento Hora y reset Min&Seg
		   			if((unidad_segundos==9 && decena_segundos==5)&&(unidad_minutos==9 && decena_minutos==5)){
		   				unidad_hora++;
		   				unidad_segundos=0; decena_segundos=0; unidad_minutos=0; decena_minutos=0;
		   			}
		   			// x:x9:59 -> incremento Decena de minutos y reset UnidadMin&Segundos
		   			else if((unidad_segundos==9 && decena_segundos==5)&&(unidad_minutos==9 && decena_minutos!=5)){
		   				decena_minutos++;
		   				unidad_segundos=0; decena_segundos=0; unidad_minutos=0;
		   			}
		   			// x:xx:59 ->incremento unidad de minutos y reset Segundos
		   			else if(unidad_segundos==9 && decena_segundos==5){
		   				unidad_minutos++;
		   				unidad_segundos=0; decena_segundos=0;
		   			}
		   			// x:xx:x9 -> incremento decena de segundos  y reset Unidad de segundos
		   			else if(unidad_segundos==9 && decena_segundos!=5){
		   				decena_segundos++;
		   				unidad_segundos=0;
		   			}
		   			// x:xx:xx
		   			else{
		   				unidad_segundos++;
		   			}
	TIM_ClearIntCapturePending(LPC_TIM0, TIM_MR0_INT);
}
void TIMER2_IRQHandler(void){
	 actual = LPC_TIM2->CR0;
	    	periodo = actual-anterior;
	    	anterior = actual;
	    	if (cont<10){
	    		frec_card[cont]=periodo;
	    		cont++;
	    	}else{
	    		for(uint8_t i=0;i<10;i++){
	    			suma+=frec_card[i];
	    		}
	    		promedio=suma/SIZE;
	    		frecCard=600/promedio;
	    	}

//	   frecCard++;

	   TIM_ClearIntPending(LPC_TIM2, TIM_CR0_INT);
}

/*void configADC(void){ CONFIGURACION DEL ADC EN MODO BURST
	// Configuracion del pin P0.24 como AD0.1
		PINSEL_CFG_Type pinCfg;

		pinCfg.Portnum = 0;
		pinCfg.Pinnum = 24;
		pinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
		pinCfg.Funcnum = 1;
		pinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;

		PINSEL_ConfigPin(&pinCfg);

		// Configuracion del ADC

		ADC_Init(LPC_ADC, 200000);
		ADC_BurstCmd(LPC_ADC,ENABLE);//Modo burst
		//ADC_StartCmd(LPC_ADC,ADC_START_CONTINUOUS);
		ADC_ChannelCmd(LPC_ADC, 1, ENABLE);
		//ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_RISING);
		ADC_IntConfig(LPC_ADC, ADC_ADGINTEN, SET);

	//	NVIC_ClearPendingIRQ(ADC_IRQn);
		NVIC_EnableIRQ(ADC_IRQn);
}*/

/* Esta función configura el canal 1 del ADC
 * para que funcione con el start asociado a
 * al canal 1 de match del timer 1 (MAT1.0).
 */
void configADC(void){
	// Configuracion del pin P0.24 como AD0.1
		PINSEL_CFG_Type pinCfg;
		pinCfg.Portnum = 0;
		pinCfg.Pinnum = 24;
		pinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
		pinCfg.Funcnum = 1;
		pinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;

		PINSEL_ConfigPin(&pinCfg);
		// Configuracion del ADC
		ADC_Init(LPC_ADC, 200000);
		ADC_ChannelCmd(LPC_ADC, 1, ENABLE);
		ADC_BurstCmd(LPC_ADC,DISABLE);//Modo no burst
		ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT10);
		ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_RISING);
		ADC_IntConfig(LPC_ADC, ADC_ADGINTEN, SET);
		//NVIC_ClearPendingIRQ(ADC_IRQn);
		NVIC_EnableIRQ(ADC_IRQn);
}
void ADC_IRQHandler(void){
	uint16_t adcValue = (LPC_ADC->ADDR1>> 4) & 0xfff;
    temp = adcValue;
    LPC_ADC->ADGDR&= LPC_ADC->ADGDR;
}


void multiplexar(uint8_t cod){
	/*
			------------------------------------------------------------
			|	2.8    |	2.7 	| 	0.17	| 	0.16	|	0.15	|													|
			|	D5	   |	D4		|	D3		|	D2		|	D1		|
			|		   |		    | 			|			|					|
			------------------------------------------------------------

	*/
	if(cod==1){ 		//Si viene un 1 -> Está activo el 1 entonces hay que desactivarlo y activar el 2 (asi sucesivamente)
		LPC_GPIO0->FIOCLR |= (1<<15);
		LPC_GPIO0->FIOSET |= (1<<16);
	}else if(cod == 2){
		LPC_GPIO0->FIOCLR |= (1<<16);
		LPC_GPIO0->FIOSET |= (1<<17);
	}else if(cod == 3){
		LPC_GPIO0->FIOCLR |= (1<<17);
		LPC_GPIO2->FIOSET |= (1<<7);
	}else if(cod == 4){
		LPC_GPIO2->FIOCLR |= (1<<7);
		LPC_GPIO2->FIOSET |= (1<<8);

	}else if(cod ==5){
		LPC_GPIO2->FIOCLR |= (1<<8);
		LPC_GPIO0->FIOSET |= (1<<15);
	}
}


void delay(uint32_t times) {
	SysTick->LOAD = 0x1869f;
	SysTick->VAL = 0;
	SysTick->CTRL |= (1<<0)|(1<<2);
	for ( int i = 0 ; i < times ;  i++){
		while(!(SysTick->CTRL & (1<<16)));
	}
	SysTick->CTRL = 0;
}


void configUART(){
	UART_CFG_Type UARTConfigStruct;
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	//defecto
	UART_ConfigStructInit(&UARTConfigStruct);
	UART_Init(LPC_UART3, &UARTConfigStruct);
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	UART_FIFOConfig(LPC_UART3,&UARTFIFOConfigStruct);
	UART_IntConfig(LPC_UART3,UART_INTCFG_RBR, ENABLE);
	UART_IntConfig(LPC_UART3,UART_INTCFG_RLS,ENABLE);
	UART_TxCmd(LPC_UART3,ENABLE);
	NVIC_EnableIRQ(UART3_IRQn);
	return;
}

void UART3_IRQHandler(){
	uint32_t intsrc,temporal, temporal1;
	intsrc = UART_GetIntId(LPC_UART3);
	temporal = intsrc & UART_IIR_INTID_MASK;
    uint8_t val[1];
	//char val[1]= "";
	if(temporal  == UART_IIR_INTID_RLS){
		temporal1 =UART_GetLineStatus(LPC_UART2);
		temporal1 &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_BI | UART_LSR_RXFE);
		if(temporal1){
			while(1){};
		}
	}

	if((temporal==UART_IIR_INTID_RDA) || (temporal ==UART_IIR_INTID_CTI)){
		UART_Receive(LPC_UART3,val,sizeof(val),NONE_BLOCKING);
	}
	uint8_t nuevo  =  *val ;
	//uint32_t valor_final = *val;
	//cargar_valor_a_wform(val);
	//if(final_lut[3]!=0) formar_onda();


	if(auxiliar_ult!=3){
		final_lut[auxiliar_ult] =  nuevo;
		auxiliar_ult++;
	}else{
		auxiliar_ult=0;
		formar_onda();
	}
	//formar_onda();
	return;
}

void cargar_valor_a_wform(uint8_t vr){
	//for(int i = 0 ; i < 3 ; i++){
	//	final_lut[i] = final_lut[i+1];
	//}

	if (auxiliar_ult!=3){
		final_lut[auxiliar_ult] = vr;
		auxiliar_ult++;
	}else{
		auxiliar_ult=0;
		formar_onda();
	}
	//final_lut[3] = val;
}

void formar_onda(){

	confDAC();
	//uint32_t i;

	/*
	//Prepare DAC sine look up table
	for(i=0;i<NUM_SINE_SAMPLE;i++)
	{
		if(i<=15)
		{
			dac_lut[i] = 512 + 512*sin_0_to_90_16_samples[i]/10000;
			if(i==15) dac_lut[i]= 1023;
		}
		else if(i<=30)
		{
			dac_lut[i] =0;
		}
		else if(i<=45)
		{
			dac_lut[i] = 0;
		}
		else
		{
			dac_lut[i] = 0;
		}
		dac_lut[i] = (dac_lut[i]<<6);
	}*/
	for(uint32_t i = 0 ;  i < 42 ; i ++){
		if(i<12) dac_lut [i] = 24 * final_lut[0];
		else if(i>12 && i <=22) dac_lut[i] = 24*final_lut[1];
		else if(i>22 && i<=34) dac_lut[i] = 24*final_lut[2];
		else if(i>34) dac_lut[i] = 1023;
		dac_lut[i] = (dac_lut[i]<<6);
	}
	/*
	for(i=0;i<42;i++)
	{
			if(i<=12) dac_lut[i] = 600;
			else if(i>12 && i<=22) dac_lut[i]= 700;
			else if(i>22 && i<=34) dac_lut[i] = 200;
			else if (i>34) dac_lut[i]= 1023;
			//else dac_lut[i] = 24*(22-i);
			dac_lut[i] = (dac_lut[i]<<6);
	}*/
	configDMA();
	GPDMA_ChannelCmd(0, ENABLE);
	//while (1);

}
void portada(){
	uint8_t tit[] = "\tTrabajo Final - Electronica Digital 3 - 2023\n\r";
	uint8_t tit1[] = "\tAlumnos: Robles Karen Yesica, Rodriguez Luciano Ariel\n\r";
	UART_Send(LPC_UART3,tit, sizeof(tit),BLOCKING);
	UART_Send(LPC_UART3,tit1, sizeof(tit1),BLOCKING);
}


void delay_sin(){
	for (uint32_t i=0;i<4000000;i++){}
	return;
}
