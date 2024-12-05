#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "enc28j60_io.h"
#include "ethernetif.h"

SPI_HandleTypeDef hspi_enc28j60;//the spi which connects to ENC28J60
extern osSemaphoreId Netif_IRQSemaphore;

/* Stores how many iterations the microcontroller can do in 1 �s */
static uint32_t _iter_per_us;

/**
 * Software delay in �s
 *  us: the number of �s to wait
 **/
__inline static void up_udelay(uint32_t us)
{
    volatile uint32_t i;

    for (i=0; i<us*_iter_per_us; i++) {
    }
}

static void _calibrate(void)
{
    uint32_t time;
    volatile uint32_t i;

    _iter_per_us = 1000000;

    time = HAL_GetTick();
    /* Wait for next tick */
    while (HAL_GetTick() == time) {
        /* wait */
    }
    for (i=0; i<_iter_per_us; i++) {
    }
    _iter_per_us /= ((HAL_GetTick()-time)*1000);
}

void ENC28J60SPIInit(void)
{
	//SPI Pins
	GPIO_InitTypeDef GPIO_InitStructure;

	__HAL_RCC_SPI1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStructure.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStructure.Alternate = GPIO_AF5_SPI1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);//MISO

	hspi_enc28j60.Instance = SPI1;
	hspi_enc28j60.Init.Mode = SPI_MODE_MASTER;
	hspi_enc28j60.Init.Direction = SPI_DIRECTION_2LINES;
	hspi_enc28j60.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi_enc28j60.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi_enc28j60.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi_enc28j60.Init.NSS = SPI_NSS_SOFT;
	hspi_enc28j60.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;//84/8=10.5MHz
	hspi_enc28j60.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi_enc28j60.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi_enc28j60.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi_enc28j60.Init.CRCPolynomial = 10;

	if (HAL_SPI_Init(&hspi_enc28j60) != HAL_OK)
	{
		Error_Handler();
	}
	_calibrate();
}
/*
 * Initialize IO pin for CS and Reset connections
 *
 * CS	---------->PA4
 * RESET---------->PA8
 *
 */
void ENC28J60GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIOs clocks */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStructure.Pin = ENC_CS_PIN|ENC_RESET_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Deselect ENC28J60 module */
	HAL_GPIO_WritePin(GPIOA, ENC_CS_PIN|ENC_RESET_PIN, GPIO_PIN_SET);

}

/*
 * Initialize interrupt for INT pin
 */

void ENC28J60INTInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//Interrupt Pin
	GPIO_InitStructure.Pin = ENC_INT_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStructure.Pull = GPIO_PULLUP;//i have changed this from no pull
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI2_IRQn , 0x0F, 0x0F);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn );
}

void ENC28J60EnableIRQ(void)
{
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
}

void ENC28J60DisableIRQ(void)
{
	HAL_NVIC_DisableIRQ(EXTI2_IRQn);
}

void EXTI2_IRQHandler (void)
{
	HAL_GPIO_EXTI_IRQHandler(ENC_INT_PIN);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == ENC_INT_PIN)
  {
      ethernet_irq_handler(Netif_IRQSemaphore);
  }
}

void ENC28J60AssertCS(void)
{
	HAL_GPIO_WritePin(GPIOA, ENC_CS_PIN, GPIO_PIN_RESET);
}

void ENC28J60ReleaseCS(void)
{
	HAL_GPIO_WritePin(GPIOA, ENC_CS_PIN, GPIO_PIN_SET);
}

/**
  * Implement SPI single byte send and receive.
  * The ENC28J60 slave SPI must already be selected and wont be deselected after transmission
  * Must be provided by user code
  * param  command: command or data to be sent to ENC28J60
  * retval answer from ENC28J60
  */

uint8_t ENC_SPI_SendWithoutSelection(uint8_t command)
{
    HAL_SPI_TransmitReceive(&hspi_enc28j60, &command, &command, 1, 1000);
    return command;
}

/**
  * Implement SPI single byte send and receive. Must be provided by user code
  * param  command: command or data to be sent to ENC28J60
  * retval answer from ENC28J60
  */

uint8_t ENC_SPI_Send(uint8_t command)
{
    /* Select ENC28J60 module */
    ENC28J60DisableIRQ();
    ENC28J60AssertCS();
    up_udelay(1);

    HAL_SPI_TransmitReceive(&hspi_enc28j60, &command, &command, 1, 1000);

    ENC28J60ReleaseCS();
    up_udelay(1);

    ENC28J60EnableIRQ();
    return command;
}

/**
  * Implement SPI buffer send and receive. Must be provided by user code
  * param  master2slave: data to be sent from host to ENC28J60, can be NULL if we only want to receive data from slave
  * param  slave2master: answer from ENC28J60 to host, can be NULL if we only want to send data to slave
  * retval none
  */

void ENC_SPI_SendBuf(uint8_t *master2slave, uint8_t *slave2master, uint16_t bufferSize)
{
    /* Select ENC28J60 module */
	ENC28J60DisableIRQ();
	ENC28J60AssertCS();
    up_udelay(1);

    /* Transmit or receuve data */
    if (slave2master == NULL) {
        if (master2slave != NULL) {
            HAL_SPI_Transmit(&hspi_enc28j60, master2slave, bufferSize, 1000);
        }
    } else if (master2slave == NULL) {
        HAL_SPI_Receive(&hspi_enc28j60, slave2master, bufferSize, 1000);
    } else {
        HAL_SPI_TransmitReceive(&hspi_enc28j60, master2slave, slave2master, bufferSize, 1000);
    }

    /* De-select ENC28J60 module */
    ENC28J60ReleaseCS();
    up_udelay(1);
    ENC28J60EnableIRQ();
}

/**
  * Implement SPI Slave selection and deselection. Must be provided by user code
  * param  select: true if the ENC28J60 slave SPI if selected, false otherwise
  * retval none
  */

void ENC_SPI_Select(bool select)
{
    /* Select or de-select ENC28J60 module */
    if (select) {
        ENC28J60DisableIRQ();
        ENC28J60AssertCS();
        up_udelay(1);
    } else {
        ENC28J60ReleaseCS();
        up_udelay(1);
        ENC28J60EnableIRQ();
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/



