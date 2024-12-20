# Network Configuration Guide for STM32 with ENC28J60

This guide covers the implementation of network configurations using lwIP stack on STM32 with ENC28J60 Ethernet module.

## Table of Contents
- [Static IP Configuration](#static-ip-configuration)
- [DHCP Configuration](#dhcp-configuration)
- [TCP Client Implementation](#tcp-client-implementation)
- [TCP Server Implementation](#tcp-server-implementation)
- [UDP Client Implementation](#udp-client-implementation)
- [UDP Server Implementation](#udp-server-implementation)

## Required Headers
```c
#include <string.h>
#include <lwip.h>
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/api.h"
```

## Static IP Configuration
Basic network interface configuration with static IP:

```c
static void Netif_Config(void)
{
    ip4_addr_t ipaddr;
    ip4_addr_t netmask;
    ip4_addr_t gw;

    /* IP address setting */
    IP4_ADDR(&ipaddr, IP_ADDR_4, IP_ADDR_3, IP_ADDR_2, IP_ADDR_1);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gw, IP_ADDR_4, IP_ADDR_3, IP_ADDR_2, 1);

    netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
    netif_set_default(&gnetif);
    netif_set_up(&gnetif);
}
```

## DHCP Configuration
To enable DHCP, define `USE_DHCP` and modify network configuration:

```c
#define USE_DHCP    // Add at the start of file

static void Netif_Config(void)
{
    ip4_addr_t ipaddr;
    ip4_addr_t netmask;
    ip4_addr_t gw;

    /* Initialize with zeros for DHCP */
    ipaddr.addr = 0;
    netmask.addr = 0;
    gw.addr = 0;

    netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
    netif_set_default(&gnetif);
    netif_set_up(&gnetif);
    dhcp_start(&gnetif);
}
```

## TCP Client Implementation
Implementation of a simple TCP client:

```c
void TCP_Client_Task(void)
{
    ip_addr_t server_ip;
    struct netconn *conn;
    err_t status;

    conn = netconn_new(NETCONN_TCP);
    IP4_ADDR(&server_ip, 192, 168, 0, 248);

    status = netconn_connect(conn, &server_ip, 5000);
    if(status == ERR_OK)
    {
        netconn_write(conn, "HELLO FROM STM32\r\n", 18, NETCONN_NOCOPY);
        netconn_close(conn);
    }
    netconn_delete(conn);
}
```

## TCP Server Implementation
Implementation of a TCP server that listens for incoming connections:

```c
void TCP_Server_Task(void)
{
    struct netconn *conn, *newconn;
    struct netbuf *buf;
    void *data;
    u16_t len;
    err_t status;

    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, IP_ADDR_ANY, 5000);
    netconn_listen(conn);

    while(1)
    {
        status = netconn_accept(conn, &newconn);
        if(status == ERR_OK)
        {
            while((status = netconn_recv(newconn, &buf)) == ERR_OK)
            {
                netbuf_data(buf, &data, &len);
                netconn_write(newconn, data, len, NETCONN_COPY);
                netbuf_delete(buf);
            }
            netconn_close(newconn);
            netconn_delete(newconn);
        }
    }
}
```

## UDP Client Implementation
Implementation of a UDP client:

```c
void UDP_Client_Task(void)
{
    ip_addr_t server_ip;
    struct netconn *conn;
    struct netbuf *buf;
    void *data;
    err_t status;

    conn = netconn_new(NETCONN_UDP);
    IP4_ADDR(&server_ip, 192, 168, 0, 248);

    buf = netbuf_new();
    data = netbuf_alloc(buf, 18);
    memcpy(data, "HELLO FROM STM32\r\n", 18);
    
    status = netconn_sendto(conn, buf, &server_ip, 5000);
    netbuf_delete(buf);
    netconn_delete(conn);
}
```

## UDP Server Implementation
Implementation of a UDP server:

```c
void UDP_Server_Task(void)
{
    struct netconn *conn;
    struct netbuf *buf;
    void *data;
    u16_t len;
    err_t status;

    conn = netconn_new(NETCONN_UDP);
    netconn_bind(conn, IP_ADDR_ANY, 5000);

    while(1)
    {
        status = netconn_recv(conn, &buf);
        if(status == ERR_OK)
        {
            ip_addr_t *client_addr = netbuf_fromaddr(buf);
            u16_t client_port = netbuf_fromport(buf);
            
            netbuf_data(buf, &data, &len);
            // Process received data here
            
            netbuf_delete(buf);
        }
    }
}
```

## Testing
1. For TCP/UDP server testing, use netcat:
   - TCP: `nc 192.168.1.100 5000`
   - UDP: `nc -u 192.168.1.100 5000`

2. Check IP configuration:
   ```c
   printf("IP: %d.%d.%d.%d\n", 
          ip4_addr1(&gnetif.ip_addr),
          ip4_addr2(&gnetif.ip_addr),
          ip4_addr3(&gnetif.ip_addr),
          ip4_addr4(&gnetif.ip_addr));
   ```

## Common Issues
1. Missing string.h:
   - Solution: Add `#include <string.h>` for memcpy and strlen

2. DHCP timeout:
   - Add timeout handling in DHCP configuration
   - Fall back to static IP if DHCP fails

3. Network buffer issues:
   - Always free netbuf using netbuf_delete()
   - Check memory allocation success

## Notes
- Always initialize lwIP stack before any network operations
- Handle all error conditions
- Clean up resources properly
- Check network status before operations
- Use proper error handling