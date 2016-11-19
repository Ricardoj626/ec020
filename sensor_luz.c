/*
 * sensor_luz.c
 *
 *  Created on: 12 de Set de 2016
 *      Author: Tolavio
 */
#include "lpc17xx_pinsel.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_timer.h"
#include "sensor_luz.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

#define I2CDEV LPC_I2C2

#define LIGHT_I2C_ADDR    (0x44)

#define ADDR_CMD        0x00
#define ADDR_CTRL       0x01
#define ADDR_IRQTH_HI   0x02
#define ADDR_IRQTH_LO   0x03
#define ADDR_LSB_SENSOR 0x04
#define ADDR_MSB_SENSOR 0x05
#define ADDR_LSB_TIMER  0x06
#define ADDR_MSB_TIMER  0x07

#define ADDR_CLAR_INT   0x40

#define CMD_ENABLE    (1 << 7)
#define CMD_APDCP     (1 << 6)
#define CMD_TIM_EXT   (1 << 5)
#define CMD_MODE(m)  ((m) << 2)
#define CMD_WIDTH(w) ((w) << 0)

#define CTRL_GAIN(g)        ((g) << 2)
#define CTRL_IRQ_PERSIST(p) ((p) << 0)
#define CTRL_IRQ_FLAG       (1 << 5)

/*
 * The Range (k) values are based on Rext = 100k
 */
#define RANGE_K1   973
#define RANGE_K2  3892
#define RANGE_K3 15568
#define RANGE_K4 62272

#define WIDTH_16_VAL (1 << 16)
#define WIDTH_12_VAL (1 << 12)
#define WIDTH_08_VAL (1 << 8)
#define WIDTH_04_VAL (1 << 4)

/******************************************************************************
 * External global variables
 *****************************************************************************/

/******************************************************************************
 * Local variables
 *****************************************************************************/

static uint32_t range = RANGE_K1;
static uint32_t width = WIDTH_16_VAL;


typedef struct {

	uint32_t light;
	BOOL_16 acorda;
}Sensor;

Sensor Class_luz;

/******************************************************************************
 * Local Functions
 *****************************************************************************/

static int I2CRead(uint8_t addr, uint8_t* buf, uint32_t len)
{
	I2C_M_SETUP_Type rxsetup;

	rxsetup.sl_addr7bit = addr;
	rxsetup.tx_data = NULL;	// Get address to read at writing address
	rxsetup.tx_length = 0;
	rxsetup.rx_data = buf;
	rxsetup.rx_length = len;
	rxsetup.retransmissions_max = 3;

	if (I2C_MasterTransferData(I2CDEV, &rxsetup, I2C_TRANSFER_POLLING) == SUCCESS){
		return (0);
	} else {
		return (-1);
	}
}

static int I2CWrite(uint8_t addr, uint8_t* buf, uint32_t len)
{
	I2C_M_SETUP_Type txsetup;

	txsetup.sl_addr7bit = addr;
	txsetup.tx_data = buf;
	txsetup.tx_length = len;
	txsetup.rx_data = NULL;
	txsetup.rx_length = 0;
	txsetup.retransmissions_max = 3;

	if (I2C_MasterTransferData(I2CDEV, &txsetup, I2C_TRANSFER_POLLING) == SUCCESS){
		return (0);
	} else {
		return (-1);
	}
}

void luz_init_i2c(void)
{
	PINSEL_CFG_Type PinCfg;

	/* Initialize I2C2 pin connect */
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	// Initialize I2C peripheral
	I2C_Init(LPC_I2C2, 100000);

	/* Enable I2C1 operation */
	I2C_Cmd(LPC_I2C2, ENABLE);
}

void luz_timer_init(uint32_t time)
{
	TIM_TIMERCFG_Type TIM_ConfigStruct;
	TIM_MATCHCFG_Type TIM_MatchConfigStruct ;

// Initialize timer 0, prescale count time of 1ms
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleValue	= 1000;
	// use channel 0, MR0
	TIM_MatchConfigStruct.MatchChannel = 0;
	// Enable interrupt when MR0 matches the value in TC register
	TIM_MatchConfigStruct.IntOnMatch   = TRUE;
	//Enable reset on MR0: TIMER will not reset if MR0 matches it
	TIM_MatchConfigStruct.ResetOnMatch = FALSE;
	//Stop on MR0 if MR0 matches it
	TIM_MatchConfigStruct.StopOnMatch  = TRUE;
	//do no thing for external output
	TIM_MatchConfigStruct.ExtMatchOutputType =TIM_EXTMATCH_NOTHING;
	// Set Match value, count value is time (timer * 1000uS =timer mS )
	TIM_MatchConfigStruct.MatchValue   = time;

	// Set configuration for Tim_config and Tim_MatchConfig
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE,&TIM_ConfigStruct);
	TIM_ConfigMatch(LPC_TIM0,&TIM_MatchConfigStruct);
	// To start timer 0
	TIM_Cmd(LPC_TIM0,ENABLE);
	while ( !(TIM_GetIntStatus(LPC_TIM0,0)));

		TIM_ClearIntPending(LPC_TIM0,0);

	Class_luz.acorda = FALSE;
	NVIC_EnableIRQ(TIMER0_IRQn);



}

void luz_init (void)
{
    /* nothing to initialize. light_enable enables the sensor */
}


/******************************************************************************
 *
 * Description:
 *    Enable the ISL29003 Device.
 *
 *****************************************************************************/
void luz_enable (void)
{
    uint8_t buf[2];
    buf[0] = ADDR_CMD;
    buf[1] = CMD_ENABLE;
    I2CWrite(LIGHT_I2C_ADDR, buf, 2);

    range = RANGE_K1;
    width = WIDTH_16_VAL;
}

/******************************************************************************
 *
 * Description:
 *    Read sensor value
 *
 * Returns:
 *      Read light sensor value (in units of Lux)
 *
 *****************************************************************************/
uint32_t luz_read(void)
{
    uint32_t data = 0;
    uint8_t buf[1];

    buf[0] = ADDR_LSB_SENSOR;
    I2CWrite(LIGHT_I2C_ADDR, buf, 1);
    I2CRead(LIGHT_I2C_ADDR, buf, 1);

    data = buf[0];

    buf[0] = ADDR_MSB_SENSOR;
    I2CWrite(LIGHT_I2C_ADDR, buf, 1);
    I2CRead(LIGHT_I2C_ADDR, buf, 1);

    data = (buf[0] << 8 | data);


    /* Rext = 100k */
    /* E = (range(k) * DATA)  / 2^n */

    return (range*data / width);
}

static uint8_t readControlReg(void)
{
    uint8_t buf[1];
    buf[0] = ADDR_CTRL;
    I2CWrite(LIGHT_I2C_ADDR, buf, 1);

    I2CRead(LIGHT_I2C_ADDR, buf, 1);

    return buf[0];
}

/******************************************************************************
 *
 * Description:
 *    Set new gain/range
 *
 * Params:
 *    [in]  newRange  - new gain/range
 *
 *****************************************************************************/
void luz_setRange(luz_range_t newRange)
{
    uint8_t buf[2];
    uint8_t ctrl = readControlReg();

    /* clear range */
    ctrl &= ~(3 << 2);

    ctrl |= CTRL_GAIN(newRange);

    buf[0] = ADDR_CTRL;
    buf[1] = ctrl;
    I2CWrite(LIGHT_I2C_ADDR, buf, 2);

    switch(newRange) {
    case LIGHT_RANGE_1000:
        range = RANGE_K1;
        break;
    case LIGHT_RANGE_4000:
        range = RANGE_K2;
        break;
    case LIGHT_RANGE_16000:
        range = RANGE_K3;
        break;
    case LIGHT_RANGE_64000:
        range = RANGE_K4;
        break;
    }
}

//retornando o status caso haja um novo valor
BOOL_16 luz_status(void){
	return Class_luz.acorda;
}

//retornando o valor de luz em luz
uint32_t retorna_luz(void){
	return Class_luz.light;
}

//Função de tratamento da interrupção pelo timer
void TIMER0_IRQHandler(void){

	//desabilita a interrupção do timer
	NVIC_DisableIRQ(TIMER0_IRQn);

	//Feita a leitura do sensor de luz
	Class_luz.light = 0;
	Class_luz.light = luz_read();

	//passa o valor da leitura de luminosidade para verdadeiro para demonstrar que teve um valor lido
	Class_luz.acorda = TRUE;
}


