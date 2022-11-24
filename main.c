/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "queue.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*Tasks Handler */
TaskHandle_t Button_1_Monitor_Handler           = NULL;
TaskHandle_t Button_2_Monitor_Handler           = NULL;
TaskHandle_t Periodic_Transmitter_Handler       = NULL;
TaskHandle_t Uart_Receiver_Handler              = NULL;
TaskHandle_t Load_1_Simulation_Handler          = NULL;
TaskHandle_t Load_2_Simulation_Handler          = NULL;

/* Queue Handlers */
QueueHandle_t MessageQueue1 = NULL;
QueueHandle_t MessageQueue2 = NULL;
QueueHandle_t MessageQueue3 = NULL;

/*Time Analysis*/
int Task1_InTime	, Task1_OutTime , Task1_TotalTime ;
int Task2_InTime	, Task2_OutTime , Task2_TotalTime ;
int Task3_InTime	, Task3_OutTime , Task3_TotalTime ;
int Task4_InTime	, Task4_OutTime , Task4_TotalTime ;
int Task5_InTime	, Task5_OutTime , Task5_TotalTime ;
int Task6_InTime	, Task6_OutTime , Task6_TotalTime ;
int systemTime;
int cpu_Load;

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */

static void prvSetupHardware( void );


/*-----------------------------------------------------------*/

/* Task to be created. */
void Button_1_Monitor(void * pvParameters ){
	
	char EdgeState;
	pinState_t button_1_CurrentState;
	pinState_t button_1_previousState=GPIO_read(PORT_1, PIN0);
	TickType_t xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL,(void *) 1);
	for(;;){
		button_1_CurrentState=GPIO_read(PORT_1, PIN0);
		if((button_1_previousState==PIN_IS_LOW)&&(button_1_CurrentState==PIN_IS_HIGH)){
			/*Rising Edge*/
			EdgeState='R';
		}else if((button_1_previousState==PIN_IS_HIGH)&&(button_1_CurrentState==PIN_IS_LOW)){
			/*Falling Edge*/
			EdgeState='F';
		}else{
			/*No Edge*/
			EdgeState=0;
		}
		/*write Edge State on MessageQueue1*/
		xQueueOverwrite(MessageQueue1, &EdgeState );
		/*Update previous State*/
		button_1_previousState=button_1_CurrentState;
		/*Delay -> Periodicity=50*/
		vTaskDelayUntil(&xLastWakeTime, 50);
	}
}



void Button_2_Monitor(void * pvParameters ){
	char EdgeState;
	pinState_t button_2_CurrentState;
	pinState_t button_2_previousState=GPIO_read(PORT_1, PIN1);
	TickType_t xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL,(void *) 2);
	for(;;){
		button_2_CurrentState=GPIO_read(PORT_1, PIN1);
		if((button_2_previousState==PIN_IS_LOW)&&(button_2_CurrentState==PIN_IS_HIGH)){
			/*Rising Edge*/
			EdgeState='R';
		}else if((button_2_previousState==PIN_IS_HIGH)&&(button_2_CurrentState==PIN_IS_LOW)){
			/*Falling Edge*/
			EdgeState='F';
		}else{
			/*No Edge*/
			EdgeState='E';
		}
		/*write Edge State on MessageQueue1*/
		xQueueOverwrite(MessageQueue2, &EdgeState );
		/*Update previous State*/
		button_2_previousState=button_2_CurrentState;
		/*Delay -> Periodicity=50*/
		vTaskDelayUntil(&xLastWakeTime, 50);
	}
}
void Periodic_Transmitter(void * pvParameters){
	TickType_t xLastWakeTime = xTaskGetTickCount();
	char str[8];
	char count;
	strcpy(str, " it's OK");
	vTaskSetApplicationTaskTag(NULL,(void *) 3);
	for(;;){
		for(count=0;count<8;count++){
			xQueueSend(MessageQueue3,(str + count), 100);
		}
		/*Delay -> Periodicity=100*/
		vTaskDelayUntil(&xLastWakeTime, 100);
	}
}
void Uart_Receiver(void * pvParameters){
	char Button_1_State;
	char Button_2_State;
	char str3[8];
	char count;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL,(void *) 4);
	for(;;){
		if(xQueueReceive(MessageQueue1, &Button_1_State, 0) && ( Button_1_State != 'E' )){
			xSerialPutChar('\n');
			if(Button_1_State=='R'){
				char Str1[27]="Button 1 State is Rising\n";
				vSerialPutString((signed char *) Str1, strlen(Str1));
			}else if(Button_1_State=='F'){
				char Str1[27]="Button 1 State is Falling\n";
				vSerialPutString((signed char *) Str1, strlen(Str1));
			}else{
				/*nothing*/
			}
		}else{
			/*nothing*/
		}
		
		if(xQueueReceive(MessageQueue2, &Button_2_State, 0) && ( Button_2_State != 'E' )){
			xSerialPutChar('\n');
			if(Button_2_State=='R'){
				char Str2[27]="Button 2 State is Rising\n";
				vSerialPutString((signed char *) Str2, strlen(Str2));
			}else if(Button_2_State=='F'){
				char Str2[27]="Button 2 State is Falling\n";
				vSerialPutString((signed char *) Str2, strlen(Str2));
			}else{
				/*nothing*/
			}
		}else{
			/*nothing*/
		}
		/*Receive String from Periodic Transmitter*/
		if(uxQueueMessagesWaiting(MessageQueue3) != 0)
			{
				for(count = 0; count<8 ; count++)
				{
					xQueueReceive(MessageQueue3, (str3+count), 0);
				}
				vSerialPutString( (signed char *) str3, strlen(str3));
				xQueueReset(MessageQueue3);
			}
		/*Delay -> Periodicity=100*/
		vTaskDelayUntil(&xLastWakeTime, 20);
		
	}
}
void Load_1_Simulation(void* pvParameters)
{
	int i;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL,(void *) 5);
	for(;;)
	{
		/* Execute for 5 ms */
		for(i=0 ;i < 32900;i++ )                                          
			{
				i=i;
			}
		
		/* priodicity = 10 ms */
		vTaskDelayUntil(&xLastWakeTime, 10);
	}
}

void Load_2_Simulation(void* pvParameters)
{
	int i;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL,(void *) 6);
	for(;;)
	{
		/* Execute for 12 ms */
		for(i=0 ;i < 79800;i++ )                                          
			{
				i=i;
			}
		
		/* priodicity = 100 ms */
		vTaskDelayUntil(&xLastWakeTime, 100);
	}
}


void vApplicationTickHook(void){
	GPIO_write(PORT_0, PIN0, PIN_IS_HIGH);
	GPIO_write(PORT_0, PIN0, PIN_IS_LOW);
}

/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
	/*Setup Message Queues*/
	MessageQueue1  = xQueueCreate( 1, sizeof(char) );
	MessageQueue2  = xQueueCreate( 1, sizeof(char) );
	MessageQueue3  = xQueueCreate( 8, sizeof(char) );
	
	/* Create Tasks here */
	xTaskPeriodicCreate(
			Button_1_Monitor,                  /* Function that implements the task. */
			"BUTTON 1 MONITOR",                /* Text name for the task. */
			100,                               /* Stack size in words, not bytes. */
			( void * ) 0,                      /* Parameter passed into the task. */
			1,                                 /* Priority at which the task is created. */
			&Button_1_Monitor_Handler,         /* Used to pass out the created task's handle. */
			50);    													 /* Period for the task */

	xTaskPeriodicCreate(
			Button_2_Monitor,                  /* Function that implements the task. */
			"BUTTON 2 MONITOR",                /* Text name for the task. */
			100,                               /* Stack size in words, not bytes. */
			( void * ) 0,                      /* Parameter passed into the task. */
			1,                                 /* Priority at which the task is created. */
			&Button_2_Monitor_Handler,       	 /* Used to pass out the created task's handle. */
			50);    													 /* Period for the task */
		xTaskPeriodicCreate(
			Periodic_Transmitter,               /* Function that implements the task. */
			"PERIODIC TRANSMITTER",             /* Text name for the task. */
			100,                                /* Stack size in womain.crds, not bytes. */
			( void * ) 0,                       /* Parameter passed into the task. */
			1,                                  /* Priority at which the task is created. */
			&Periodic_Transmitter_Handler,   /* Used to pass out the created task's handle. */
			100);  /* Period for the task */
			xTaskPeriodicCreate(
			Uart_Receiver,                      /* Function that implements the task. */
			"UART RECEIVER",                    /* Text name for the task. */
			100,                                /* Stack size in words, not bytes. */
			( void * ) 0,                       /* Parameter passed into the task. */
			1,                                  /* Priority at which the task is created. */
			&Uart_Receiver_Handler,          /* Used to pass out the created task's handle. */
			20);         /* Period for the task */
			xTaskPeriodicCreate(
			Load_1_Simulation,                 /* Function that implements the task. */
			"LOAD 1 SIMULATION",               /* Text name for the task. */
			100,                               /* Stack size in words, not bytes. */
			( void * ) 0,                      /* Parameter passed into the task. */
			1,                                 /* Priority at which the task is created. */
			&Load_1_Simulation_Handler,      		/* Used to pass out the created task's handle. */
			10);	   													/* Period for the task */

	xTaskPeriodicCreate(
			Load_2_Simulation,                 /* Function that implements the task. */
			"LOAD 1 SIMULATION",               /* Text name for the task. */
			100,                               /* Stack size in words, not bytes. */
			( void * ) 0,                      /* Parameter passed into the task. */
			1,                                 /* Priority at which the task is created. */
			&Load_2_Simulation_Handler,      	 /* Used to pass out the created task's handle. */
			100); 					


	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/
