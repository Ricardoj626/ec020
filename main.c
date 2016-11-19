/*
    FreeRTOS V6.1.1 - Copyright (C) 2011 Real Time Engineers Ltd.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    ***NOTE*** The exception to the GPL is included to allow you to distribute
    a combined work that includes FreeRTOS without being obliged to provide the
    source code for proprietary components outside of the FreeRTOS kernel.
    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public 
    License and the FreeRTOS license exception along with FreeRTOS; if not it 
    can be viewed here: http://www.freertos.org/a00114.html and also obtained 
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "lpc17xx_pinsel.h"
#include "sensor_luz.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_i2c.h"
#include "oled.h"

/* Demo includes. */
#include "basic_io.h"


/* The tasks to be created.  Two instances are created of the sender task while
only a single instance is created of the receiver task. */
static void vSenderTask( void *pvParameters );
static void vReceiverTask( void *pvParameters );

/*-----------------------------------------------------------*/

/* Declare a variable of type xQueueHandle.  This is used to store the queue
that is accessed by all three tasks. */
xQueueHandle xQueue;

BOOL_16 acorda = FALSE;

static uint8_t buf[10];

static void intToString(int value, uint8_t* pBuf, uint32_t len, uint32_t base)
{
    static const char* pAscii = "0123456789abcdefghijklmnopqrstuvwxyz";
    int pos = 0;
    int tmpValue = value;

    // the buffer must not be null and at least have a length of 2 to handle one
    // digit and null-terminator
    if (pBuf == NULL || len < 2)
    {
        return;
    }

    // a valid base cannot be less than 2 or larger than 36
    // a base value of 2 means binary representation. A value of 1 would mean only zeros
    // a base larger than 36 can only be used if a larger alphabet were used.
    if (base < 2 || base > 36)
    {
        return;
    }

    // negative value
    if (value < 0)
    {
        tmpValue = -tmpValue;
        value    = -value;
        pBuf[pos++] = '-';
    }

    // calculate the required length of the buffer
    do {
        pos++;
        tmpValue /= base;
    } while(tmpValue > 0);


    if (pos > len)
    {
        // the len parameter is invalid.
        return;
    }

    pBuf[pos] = '\0';

    do {
        pBuf[--pos] = pAscii[value % base];
        value /= base;
    } while(value > 0);

    return;
}

static void init_ssp(void)
{
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;

	/*
	 * Initialize SPI pin connect
	 * P0.7 - SCK;
	 * P0.8 - MISO
	 * P0.9 - MOSI
	 * P2.2 - SSEL - used as GPIO
	 */
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 7;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 8;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 9;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Funcnum = 0;
	PinCfg.Portnum = 2;
	PinCfg.Pinnum = 2;
	PINSEL_ConfigPin(&PinCfg);

	SSP_ConfigStructInit(&SSP_ConfigStruct);

	// Initialize SSP peripheral with parameter given in structure above
	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);

	// Enable SSP peripheral
	SSP_Cmd(LPC_SSP1, ENABLE);

}

int main( void )
{
    /* The queue is created to hold a maximum of 5 long values. */
    xQueue = xQueueCreate( 5, sizeof( uint32_t ) );
    luz_init_i2c();

	if( xQueue != NULL )
	{
		/* Create two instances of the task that will write to the queue.  The
		parameter is used to pass the value that the task should write to the queue,
		so one task will continuously write 100 to the queue while the other task 
		will continuously write 200 to the queue.  Both tasks are created at
		priority 1. */
		xTaskCreate( vSenderTask, "Sender1", 240, ( void * ) 100, 2, NULL );


		/* Create the task that will read from the queue.  The task is created with
		priority 2, so above the priority of the sender tasks. */
		xTaskCreate( vReceiverTask, "Receiver", 240, NULL, 1, NULL );

		/* Start the scheduler so the created tasks start executing. */
		vTaskStartScheduler();
	}
	else
	{
		/* The queue could not be created. */
	}
		
    /* If all is well we will never reach here as the scheduler will now be
    running the tasks.  If we do reach here then it is likely that there was 
    insufficient heap memory available for a resource to be created. */
	for( ;; );
	return 0;
}
/*-----------------------------------------------------------*/

static void vSenderTask( void *pvParameters )
{
long lValueToSend;
portBASE_TYPE xStatus;

	/* Two instances are created of this task so the value that is sent to the
	queue is passed in via the task parameter rather than be hard coded.  This way
	each instance can use a different value.  Cast the parameter to the required
	type. */
	lValueToSend = ( long ) pvParameters;
	uint32_t luz;
	luz_enable();
	luz_setRange(LIGHT_RANGE_1000);


	/* As per most tasks, this task is implemented within an infinite loop. */
	for( ;; )
	{
		/* The first parameter is the queue to which data is being sent.  The 
		queue was created before the scheduler was started, so before this task
		started to execute.

		The second parameter is the address of the data to be sent.

		The third parameter is the Block time � the time the task should be kept
		in the Blocked state to wait for space to become available on the queue
		should the queue already be full.  In this case we don�t specify a block
		time because there should always be space in the queue. */
		luz = luz_read();
		acorda = TRUE;
		xStatus = xQueueSendToBack( xQueue, &luz, 100 );


		if( xStatus != pdPASS )
		{
			/* We could not write to the queue because it was full � this must
			be an error as the queue should never contain more than one item! */
			vPrintString( "Could not send to the queue.\r\n" );
		}

		/* Allow the other sender task to execute. */
		taskYIELD();
	}
}
/*-----------------------------------------------------------*/

static void vReceiverTask( void *pvParameters )
{
/* Declare the variable that will hold the values received from the queue. */
uint32_t lReceivedValue;
portBASE_TYPE xStatus;
const portTickType xTicksToWait = 100 / portTICK_RATE_MS;


	init_ssp();

	oled_init();

	oled_clearScreen(OLED_COLOR_WHITE);

	oled_putString(1,1,  (uint8_t*)"Light:   ", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
	/* This task is also defined within an infinite loop. */
	for( ;; )
	{
		/* As this task unblocks immediately that data is written to the queue this
		call should always find the queue empty. */
//		if( uxQueueMessagesWaiting( xQueue ) != 0 )
//		{
//			vPrintString( "Queue should have been empty!\r\n" );
//		}

		/* The first parameter is the queue from which data is to be received.  The
		queue is created before the scheduler is started, and therefore before this
		task runs for the first time.

		The second parameter is the buffer into which the received data will be
		placed.  In this case the buffer is simply the address of a variable that
		has the required size to hold the received data. 

		the last parameter is the block time � the maximum amount of time that the
		task should remain in the Blocked state to wait for data to be available should
		the queue already be empty. */
		xStatus = xQueueReceive( xQueue, &lReceivedValue, xTicksToWait );

		if( xStatus == pdPASS )
		{
			/* Data was successfully received from the queue, print out the received
			value. */
			vPrintStringAndNumber( "Received = ", lReceivedValue );

					intToString(lReceivedValue, buf, 10, 10);
					oled_fillRect((1+6*6),1, 80, 8, OLED_COLOR_WHITE);
					oled_putString((1+6*6),1, buf, OLED_COLOR_BLACK, OLED_COLOR_WHITE);


		}
		else
		{
			/* We did not receive anything from the queue even after waiting for 100ms.
			This must be an error as the sending tasks are free running and will be
			continuously writing to the queue. */
			vPrintString( "Could not receive from the queue.\r\n" );
		}
	}
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* This function will only be called if an API call to create a task, queue
	or semaphore fails because there is too little heap RAM remaining. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName )
{
	/* This function will only be called if a task overflows its stack.  Note
	that stack overflow checking does slow down the context switch
	implementation. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* This example does not use the idle hook to perform any processing. */
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
	/* This example does not use the tick hook to perform any processing. */
}


