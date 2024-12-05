#ifndef TARGET_ENC28J60_ENC28J60_IO_H_
#define TARGET_ENC28J60_ENC28J60_IO_H_

#include "stm32f4xx_hal.h"

/*
 * SPI: SPI1
 * ---------
 * ->MOSI -> PA7
 * ->MISO -> PA6
 * ->SCK  -> PA5
 * ->CS   -> PA4
 * ->RESET-> PA8
 * ->INT  -> PB2
 */

//On GPIOA
#define ENC_CS_PIN 		GPIO_PIN_4
#define ENC_RESET_PIN 	GPIO_PIN_8

//On GPIOB
#define ENC_INT_PIN 	GPIO_PIN_2

void ENC28J60SPIInit(void);
void ENC28J60GPIOInit(void);
void ENC28J60INTInit(void);
uint8_t ENC_SPI_SendWithoutSelection(uint8_t command);
uint8_t ENC_SPI_Send(uint8_t command);
void ENC_SPI_SendBuf(uint8_t *master2slave, uint8_t *slave2master, uint16_t bufferSize);
void ENC_SPI_Select(bool select);



#endif /* TARGET_ENC28J60_ENC28J60_IO_H_ */
