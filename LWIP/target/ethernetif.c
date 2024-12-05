/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Netconn_RTOS/Src/ethernetif.c
  * @author  MCD Application Team
  * @version V1.2.1
  * @date    13-March-2015
  * @brief   This file implements Ethernet network interface drivers for lwIP
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "lwip/opt.h"
//#include "lwip/lwip_timers.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"
#include "lwip/opt.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/dhcp.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "ethernetif.h"
#include "enc28j60/enc28j60.h"
#include "enc28j60/enc28j60_io.h"
#include <string.h>

/* Variables Initialization */
#define USE_DHCP

/* Imported variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* The time to block waiting for input. */
#define TIME_WAITING_FOR_INPUT                 ( 100 )

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ENC_HandleTypeDef EncHandle;

#ifdef USE_PROTOTHREADS
static struct pt transmit_pt;
#endif

u32_t sys_now(void)
{
  return HAL_GetTick();
}

/*******************************************************************************
                       LL Driver Interface ( LwIP stack --> ETH)
*******************************************************************************/
/**
  * @brief In this function, the hardware should be initialized.
  * Called from ethernetif_init().
  *
  * @param netif the already initialized lwip network interface structure
  *        for this ethernetif
  */
static err_t low_level_init(struct netif *netif)
{
  //uint8_t macaddress[6]= { MAC_ADDR0, MAC_ADDR1, MAC_ADDR2, MAC_ADDR3, MAC_ADDR4, MAC_ADDR5 };


  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] =  ENC_MAC_6;
  netif->hwaddr[1] =  ENC_MAC_5;
  netif->hwaddr[2] =  ENC_MAC_4;
  netif->hwaddr[3] =  ENC_MAC_3;
  netif->hwaddr[4] =  ENC_MAC_2;
  netif->hwaddr[5] =  ENC_MAC_1;

  EncHandle.Init.MACAddr = netif->hwaddr;
  EncHandle.Init.DuplexMode = ETH_MODE_HALFDUPLEX;
  EncHandle.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
  EncHandle.Init.InterruptEnableBits =  EIE_LINKIE | EIE_PKTIE;

  /* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
  ENC28J60SPIInit();
  ENC28J60GPIOInit();
  ENC28J60INTInit();

  /* Set netif link flag */
  //  netif->flags |= NETIF_FLAG_LINK_UP;

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

  /* Start the EN28J60 module */
  if (ENC_Start(&EncHandle)) {
    /* Set the MAC address */
    ENC_SetMacAddr(&EncHandle);//copied MAC address from this handle to actual hardware registers

    /* Set netif link flag */
    //netif->flags |= NETIF_FLAG_LINK_UP;//Avinash says: why this? setting link up flag without any checks?

    return ERR_OK;
  }
  else
  {
	  return ERR_IF;
  }
}

/**
  * @brief This function should do the actual transmission of the packet. The packet is
  * contained in the pbuf that is passed to the function. This pbuf
  * might be chained.
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
  * @return ERR_OK if the packet could be sent
  *         an err_t value if the packet couldn't be sent
  *
  * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
  *       strange results. You might consider waiting for space in the DMA queue
  *       to become available since the stack doesn't retry to send a packet
  *       dropped because of memory failure (except for the TCP timers).
  */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    /* TODO use netif to check if we are the right ethernet interface */
  err_t errval;
  struct pbuf *q;
  uint32_t framelength = 0;

  if (EncHandle.transmitLength != 0) {
#ifdef USE_PROTOTHREADS
     while (PT_SCHEDULE(ENC_Transmit(&transmit_pt, &EncHandle))) {
         /* Wait for end of previous transmission */
     }
#else
     do {
         ENC_Transmit(&EncHandle);
     } while (EncHandle.transmitLength != 0);
#endif
  }

  /* Prepare ENC28J60 Tx buffer */
  errval = ENC_RestoreTXBuffer(&EncHandle, p->tot_len);
  if (errval != ERR_OK) {
      return errval;
  }

  /* copy frame from pbufs to driver buffers and send packet */
  for(q = p; q != NULL; q = q->next) {
    ENC_WriteBuffer(q->payload, q->len);
    framelength += q->len;
  }

  if (framelength != p->tot_len) {
     return ERR_BUF;
  }

  EncHandle.transmitLength = p->tot_len;

  /* If PROTOTHREADS are use, actual transmission is triggered in main loop */
#ifndef USE_PROTOTHREADS
    ENC_Transmit(&EncHandle);
#endif

  return ERR_OK;
}

/**
  * @brief Should allocate a pbuf and transfer the bytes of the incoming
  * packet from the interface into the pbuf.
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @return a pbuf filled with the received packet (including MAC header)
  *         NULL on memory error
  */
static struct pbuf * low_level_input(struct netif *netif)
{
  struct pbuf *p = NULL;
  struct pbuf *q;
  uint16_t len;
  uint8_t *buffer;
  uint32_t bufferoffset = 0;

  if (!ENC_GetReceivedFrame(&EncHandle)) {
    return NULL;
  }

  /* Obtain the size of the packet and put it into the "len" variable. */
  len = EncHandle.RxFrameInfos.length;
  buffer = (uint8_t *)EncHandle.RxFrameInfos.buffer;

  if (len > 0)
  {
    /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  }

  if (p != NULL)
  {
    bufferoffset = 0;

    for(q = p; q != NULL; q = q->next)
    {
      /* Copy data in pbuf */
      memcpy( (uint8_t*)((uint8_t*)q->payload), (uint8_t*)((uint8_t*)buffer + bufferoffset), q->len);
      bufferoffset = bufferoffset + q->len;
    }
  }

  return p;
}

/**
  * @brief This function should be called when a packet is ready to be read
  * from the interface. It uses the function low_level_input() that
  * should handle the actual reception of bytes from the network
  * interface. Then the type of the received packet is determined and
  * the appropriate input function is called.
  *
  * @param netif the lwip network interface structure for this ethernetif
  */
void ethernetif_input_do(struct netif * netif)
{
    struct pbuf *p;

    do {
        p = low_level_input(netif);
        if (p != NULL)
        {
          if (netif->input(p, netif) != ERR_OK )
          {
            pbuf_free(p);
          }
        }
    }while(p!=NULL);
}

/**
  * @brief Should be called at the beginning of the program to set up the
  * network interface. It calls the function low_level_init() to do the
  * actual setup of the hardware.
  *
  * This function should be passed as a parameter to netif_add().
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @return ERR_OK if the loopif is initialized
  *         ERR_MEM if private data couldn't be allocated
  *         any other err_t on error
  */
err_t ethernetif_init(struct netif *netif)
{
	err_t status;

  LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "stm32idisco";
#endif /* LWIP_NETIF_HOSTNAME */

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;

  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  /* initialize the hardware */
  status=low_level_init(netif);


  return status;
}

/**
  * @brief  This function actually process pending IRQs.
  * @param  handler: Reference to the driver state structure
  * @retval None
  */
void ethernetif_process_irq_do(void const *argument)
{
    struct enc_irq_str *irq_arg = (struct enc_irq_str *)argument;

    /* Handle ENC28J60 interrupt */
    ENC_IRQHandler(&EncHandle);

    /* Check whether the link is up or down*/
    if ((EncHandle.interruptFlags & EIE_LINKIE) != 0) {
        if((EncHandle.LinkStatus & PHSTAT2_LSTAT)!= 0) {
            netif_set_link_up(irq_arg->netif);
        } else {
            netif_set_link_down(irq_arg->netif);
        }
    }

    /* Check whether we have received a packet */
    if((EncHandle.interruptFlags & EIR_PKTIF) != 0) {
        ethernetif_input_do(irq_arg->netif);
    }

    /* Renable global interrupts */
    ENC_EnableInterrupts(EIE_INTIE);
}

/**
  * @brief  This function triggers the interrupt service callback.
  * @param  netif: the network interface
  * @retval None
  */
void ethernetif_process_irq(void *argument)
{
  struct enc_irq_str *irq_arg = (struct enc_irq_str *)argument;

  for(;;)
  {
	//if (osSemaphoreWait(irq_arg->semaphore, TIME_WAITING_FOR_INPUT) == osOK)
	if (osSemaphoreAcquire(irq_arg->semaphore, TIME_WAITING_FOR_INPUT) == osOK)
    {
        /* Handle ENC28J60 interrupt */
        tcpip_callback((tcpip_callback_fn) ethernetif_process_irq_do, (void *) argument);
    }
  }
}

/**
  * @brief  This function unblocks ethernetif_process_irq when a new interrupt is received
  * @param  netif: the network interface
  * @retval None
  */
void ethernet_irq_handler(osSemaphoreId Netif_IrqSemaphore)
{
    /* Release thread to check interrupt flags */
     osSemaphoreRelease(Netif_IrqSemaphore);
}

/**
  * @brief  Link callback function, this function is called on change of link status
  *         to update low level driver configuration.
* @param  netif: The network interface
  * @retval None
  */
void ethernetif_update_config(struct netif *netif)
{
  if(netif_is_link_up(netif)) {
      /* Restart the EN28J60 module */
      low_level_init(netif);
  }

  ethernetif_notify_conn_changed(netif);
}

/**
  * @brief  This function notify user about link status changement.
  * @param  netif: the network interface
  * @retval None
  */
__weak void ethernetif_notify_conn_changed(struct netif *netif)
{
  /* NOTE : This is function could be implemented in user file
            when the callback is needed,
  */
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
