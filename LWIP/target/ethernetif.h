#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"
#include "cmsis_os.h"

//MAC ADDRESS
#define ENC_MAC_6 0x80
#define ENC_MAC_5 0x34
#define ENC_MAC_4 0x28
#define ENC_MAC_3 0x74
#define ENC_MAC_2 0x92
#define ENC_MAC_1 0x12

//SELF IP ADDRESS
#define IP_ADDR_4 192
#define IP_ADDR_3 168
#define IP_ADDR_2 0
#define IP_ADDR_1 100

/* Exported types ------------------------------------------------------------*/
/* Structure that include link thread parameters */
struct enc_irq_str {
  struct netif *netif;
  osSemaphoreId semaphore;
};
/* Exported functions ------------------------------------------------------- */
err_t ethernetif_init(struct netif *netif);
void ethernetif_process_irq(void *argument);
void ethernetif_update_config(struct netif *netif);
void ethernetif_notify_conn_changed(struct netif *netif);
void ethernet_transmit(void);
void ethernet_irq_handler(osSemaphoreId Netif_LinkSemaphore);


/**
  * @}
  */

#endif
