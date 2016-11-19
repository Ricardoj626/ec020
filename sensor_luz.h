/*
 * sensor_luz.h
 *
 *  Created on: 15 de Set de 2016
 *      Author: Tolavio
 */

#ifndef SENSOR_LUZ_H_
#define SENSOR_LUZ_H_

typedef enum
{
    LIGHT_RANGE_1000,
    LIGHT_RANGE_4000,
    LIGHT_RANGE_16000,
    LIGHT_RANGE_64000
} luz_range_t;

static uint8_t readControlReg(void);
static int I2CRead(uint8_t addr, uint8_t* buf, uint32_t len);
static int I2CWrite(uint8_t addr, uint8_t* buf, uint32_t len);
void luz_init_i2c(void);
void luz_timer_init(uint32_t time);
void luz_init (void);
void luz_enable (void);
uint32_t luz_read(void);
void luz_setRange(luz_range_t newRange);
BOOL_16 luz_status();
uint32_t retorna_luz(void);



#endif /* SENSOR_LUZ_H_ */
