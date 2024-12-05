/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "string.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lwipopts.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "ethernetif.h"
#include "lwip/api.h"

/* Variables Initialization */
#define USE_DHCP
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/dhcp.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
//SPI_HandleTypeDef hspi1;
/* network interface structure */
struct netif gnetif;

/* Semaphore to signal Ethernet Link state update */
osSemaphoreId Netif_IRQSemaphore = NULL;

const osThreadAttr_t Netif_Thread_attr = {
        .name="NETIF",
        .stack_size = configMINIMAL_STACK_SIZE * 2,
        .priority=osPriorityRealtime
};

/* Ethernet link thread Argument */
struct enc_irq_str irq_arg;

static void Netif_Config(void);


UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 8, //1024 bytes
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
//static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
//  MX_SPI1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
//static void MX_SPI1_Init(void)
//{
//
//  /* USER CODE BEGIN SPI1_Init 0 */
//
//  /* USER CODE END SPI1_Init 0 */
//
//  /* USER CODE BEGIN SPI1_Init 1 */
//
//  /* USER CODE END SPI1_Init 1 */
//  /* SPI1 parameter configuration*/
//  hspi1.Instance = SPI1;
//  hspi1.Init.Mode = SPI_MODE_MASTER;
//  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
//  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
//  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
//  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
//  hspi1.Init.NSS = SPI_NSS_SOFT;
//  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
//  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
//  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
//  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//  hspi1.Init.CRCPolynomial = 10;
//  if (HAL_SPI_Init(&hspi1) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  /* USER CODE BEGIN SPI1_Init 2 */
//
//  /* USER CODE END SPI1_Init 2 */
//
//}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask FOR TCP*/
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask FOR TCP*/

//void StartDefaultTask(void *argument)
//{
//  /* USER CODE BEGIN 5 */
//  /* Infinite loop */
//	//Let ENC28J60 startup
//	osDelay(500);
//
//	// Initilaize the lwIP stack
//	tcpip_init(NULL,NULL); //first is done function, 2nd its args, both null
//
//	Netif_Config();
//
//	//Simple TCP Client!
//	//IP address of the remote server
//	ip_addr_t server_ip;
//
//	//This holds the return values from network functions
//	//mainly used to check error or success
//	err_t status;
//
//	//This struct represent a network connection
//	struct netconn *conn;
//
//	//Allocate resources for a new connection
//	//Parameter tells what type of connection we need
//	//Other option is for NETCONN_UDP
//	conn=netconn_new(NETCONN_TCP);
//
//	IP4_ADDR(&server_ip,192,168,0,200);//My PC's local IP address is 192.168.2.17
//
//	printf("Connecting to server ... \r\n ");
//	status=netconn_connect(conn, &server_ip, 5000);
//
//	if(status==ERR_OK)
//	{
//	     printf("Connected to server! \r\n ");
//	}
//	else
//	{
//	     printf("Sorry ! Could not connect to server.\r\n ");
//
//	     while(1);
//	}
//
//	//Send a Hello Message 5 time, waiting 1 second
//	for(int i=0;i<5;i++)
//	{
//	    printf("Sending data to server \r\n ");
//
//	    status=netconn_write(conn,"HELLO FROM STM32\r\n",18,NETCONN_NOCOPY);
//
//	    if(status!=ERR_OK)
//	    {
//	         //Sending failed
//	         printf("Sending failed! \r\n ");
//
//	         netconn_delete(conn);
//
//	         while(1);
//	    }
//	    else if(status==ERR_OK)
//	    {
//	         printf("Data sent successfully ! \r\n ");
//	    }
//
//	    osDelay(1000);
//
//	}//end for loop, 5 times looping done
//
//	//Clean up resources
//	netconn_close(conn);
//	netconn_delete(conn);
//
//	for(;;)
//	{
//
//	}
//  /* USER CODE END 5 */
//}

/* USER CODE BEGIN Header_StartDefaultTask FOR UDP*/
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask FOR UDP*/

//void StartDefaultTask(void *argument)
//{
//    /* USER CODE BEGIN 5 */
//    /* Infinite loop */
//    //Let ENC28J60 startup
//    osDelay(500);
//
//    // Initialize the lwIP stack
//    tcpip_init(NULL,NULL);
//
//    Netif_Config();
//
//    //Simple UDP Client
//    //IP address of the remote server
//    ip_addr_t server_ip;
//
//    //This holds the return values from network functions
//    err_t status;
//
//    //This struct represent a network connection
//    struct netconn *conn;
//
//    //Allocate resources for new UDP connection
//    conn = netconn_new(NETCONN_UDP);
//
//    if(conn == NULL) {
//        printf("Failed to create UDP connection!\r\n");
//        while(1);
//    }
//
//    IP4_ADDR(&server_ip,192,168,0,200); // Server IP address
//
//    printf("Creating UDP connection...\r\n");
//
//    //Untuk UDP tidak perlu explicit connection seperti TCP
//    //Tapi kita bisa langsung mengirim data
//
//    //Send a Hello Message 5 time, waiting 1 second
//    for(int i=0; i<5; i++)
//    {
//        struct netbuf *buf;
//        const char *data = "HELLO FROM STM32\r\n";
//        uint16_t len = strlen(data);
//
//        printf("Sending UDP data to server\r\n");
//
//        // Buat buffer baru
//        buf = netbuf_new();
//        if(buf == NULL) {
//            printf("Failed to create netbuf!\r\n");
//            continue;
//        }
//
//        // Alokasi dan copy data ke buffer
//        void *msg = netbuf_alloc(buf, len);
//        if(msg == NULL) {
//            printf("Failed to allocate buffer!\r\n");
//            netbuf_delete(buf);
//            continue;
//        }
//        memcpy(msg, data, len);
//
//        // Kirim data ke server
//        status = netconn_sendto(conn, buf, &server_ip, 1337); // Port 5000
//
//        if(status != ERR_OK)
//        {
//            printf("Failed to send UDP data!\r\n");
//        }
//        else
//        {
//            printf("UDP data sent successfully!\r\n");
//        }
//
//        // Bersihkan buffer
//        netbuf_delete(buf);
//
//        osDelay(1000);
//    }
//
//    //Clean up resources
//    netconn_delete(conn);
//
//    for(;;)
//    {
////        osDelay(1000);
//    }
//    /* USER CODE END 5 */
//}

/* USER CODE BEGIN Header_StartDefaultTask FOR TCP Listener*/
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask FOR TCP*/

void StartDefaultTask(void *argument)
{
    /* USER CODE BEGIN 5 */
    /* Infinite loop */
    //Let ENC28J60 startup
    osDelay(500);

    // Initialize the lwIP stack
    tcpip_init(NULL,NULL);

    Netif_Config();

    // TCP Server variables
    struct netconn *conn, *newconn;
    err_t status;
    struct netbuf *buf;
    void *data;
    u16_t len;

    // Create new TCP connection
    conn = netconn_new(NETCONN_TCP);
    if (conn == NULL) {
        printf("Failed to create TCP connection!\r\n");
        while(1);
    }

    // Bind to port 5000
    status = netconn_bind(conn, IP_ADDR_ANY, 5000);
    if (status != ERR_OK) {
        printf("Failed to bind port!\r\n");
        while(1);
    }

    // Start listening for connections
    status = netconn_listen(conn);
    if (status != ERR_OK) {
        printf("Failed to start listening!\r\n");
        while(1);
    }

    printf("TCP Server started on port 5000\r\n");

    for(;;)
    {
        // Wait for new connection
        printf("Waiting for connection...\r\n");
        status = netconn_accept(conn, &newconn);

        if (status == ERR_OK)
        {
            printf("New client connected!\r\n");

            // Process received data
            while (1)
            {
                // Wait for data
                status = netconn_recv(newconn, &buf);

                if (status == ERR_OK)
                {
                    // Get data from buffer
                    netbuf_data(buf, &data, &len);

                    // Print received data
                    printf("\nReceived %d bytes: ", len);
                    for(int i = 0; i < len; i++) {
                        printf("%c", ((char*)data)[i]);
                    }
                    printf("\r\n");

                    // Send response back
                    const char *reply = "Message received by STM32\r\n";
                    netconn_write(newconn, reply, strlen(reply), NETCONN_COPY);

                    // Free the buffer
                    netbuf_delete(buf);
                }
                else if (status == ERR_CLSD)
                {
                    printf("Connection closed by client\r\n");
                    netconn_close(newconn);
                    netconn_delete(newconn);
                    break;
                }
                else
                {
                    printf("netconn_recv failed with error %d\r\n", status);
                    netconn_close(newconn);
                    netconn_delete(newconn);
                    break;
                }
            }
        }
        else
        {
            printf("netconn_accept failed with error %d\r\n", status);
        }
    }

    /* Never reached */
    netconn_close(conn);
    netconn_delete(conn);

    /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartDefaultTask FOR UDP Listener*/
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask FOR UDP Listener*/

//void StartDefaultTask(void *argument)
//{
//    /* USER CODE BEGIN 5 */
//    /* Infinite loop */
//    //Let ENC28J60 startup
//    osDelay(500);
//
//    // Initialize the lwIP stack
//    tcpip_init(NULL,NULL);
//
//    Netif_Config();
//
//    // UDP Server variables
//    struct netconn *conn;
//    struct netbuf *buf;
//    err_t status;
//    void *data;
//    u16_t len;
//
//    // Create new UDP connection
//    conn = netconn_new(NETCONN_UDP);
//    if (conn == NULL) {
//        printf("Failed to create UDP connection!\r\n");
//        while(1);
//    }
//
//    // Bind to port 5000
//    status = netconn_bind(conn, IP_ADDR_ANY, 1337);
//    if (status != ERR_OK) {
//        printf("Failed to bind port!\r\n");
//        while(1);
//    }
//
//    printf("UDP Server started on port 5000\r\n");
//
//    for(;;)
//    {
//        // Wait for data
//        status = netconn_recv(conn, &buf);
//
//        if (status == ERR_OK)
//        {
//            // Get sender info - menggunakan cara yang benar
//            ip_addr_t *client_addr = netbuf_fromaddr(buf);
//            u16_t client_port = netbuf_fromport(buf);
//
//            // Get data from buffer
//            netbuf_data(buf, &data, &len);
//
//            // Print received data
//            printf("Received %d bytes from %d.%d.%d.%d:%d: ",
//                len,
//                ip4_addr1(client_addr),
//                ip4_addr2(client_addr),
//                ip4_addr3(client_addr),
//                ip4_addr4(client_addr),
//                client_port);
//
//            for(int i = 0; i < len; i++) {
//                printf("%c", ((char*)data)[i]);
//            }
//            printf("\r\n");
//
//            // Optional: Send response back
//            struct netbuf *reply_buf;
//            const char *reply = "Message received by STM32\r\n";
//
//            reply_buf = netbuf_new();
//            if(reply_buf != NULL) {
//                void *reply_data = netbuf_alloc(reply_buf, strlen(reply));
//                if(reply_data != NULL) {
//                    memcpy(reply_data, reply, strlen(reply));
//                    netconn_sendto(conn, reply_buf, client_addr, client_port);
//                }
//                netbuf_delete(reply_buf);
//            }
//
//            // Free the received buffer
//            netbuf_delete(buf);
//        }
//        else
//        {
//            printf("netconn_recv failed with error %d\r\n", status);
//        }
//    }
//
//    /* Never reached */
//    netconn_delete(conn);
//
//    /* USER CODE END 5 */
//}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM10 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM10) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

int putchar(int ch)
{
    HAL_UART_Transmit(&huart2,(uint8_t *)&ch,1,1);
    return ch;
}

static void Netif_Config(void)
{
    ip4_addr_t ipaddr;
    ip4_addr_t netmask;
    ip4_addr_t gw;

    /* Default Static IP Configuration */
    IP4_ADDR(&ipaddr, IP_ADDR_4, IP_ADDR_3, IP_ADDR_2, IP_ADDR_1);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gw, IP_ADDR_4, IP_ADDR_3, IP_ADDR_2, 1);

    /* create a binary semaphore used for informing ethernetif of frame reception */
    Netif_IRQSemaphore = osSemaphoreNew(1,0,NULL);

    printf("Adding Network IF ...\r\n ");

    if(netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input)==NULL)
    {
        printf("Error!\r\n ");
        while(1);
    }
    else
    {
        printf("Success!\r\n ");
    }

    /* Registers the default network interface */
    netif_set_default(&gnetif);

    /* Set the link callback function */
    netif_set_link_callback(&gnetif, ethernetif_update_config);

    irq_arg.netif = &gnetif;
    irq_arg.semaphore = Netif_IRQSemaphore;

    /* Create the Ethernet IRQ handler thread */
    osThreadNew(ethernetif_process_irq, &irq_arg, &Netif_Thread_attr);

    printf("Waiting for cable ...");

    while(!netif_is_link_up(&gnetif))
    {
        // Menunggu koneksi kabel
    }

    printf("Cable plugged in!\r\n ");

    /* Aktifkan network interface */
    netif_set_up(&gnetif);

    #ifdef USE_DHCP
        printf("Starting DHCP client...\r\n");
        dhcp_start(&gnetif);

        // Tunggu sampai dapat IP dari DHCP dengan timeout
        printf("Waiting for IP from DHCP ...\r\n");

        uint32_t dhcp_wait = 0;
        const uint32_t DHCP_TIMEOUT_MS = 10000; // 10 detik timeout

        while(!dhcp_supplied_address(&gnetif))
        {
            osDelay(100);
            dhcp_wait += 100;

            if(dhcp_wait >= DHCP_TIMEOUT_MS)
            {
                printf("DHCP Failed! Using static IP...\r\n");
                dhcp_stop(&gnetif);  // Stop DHCP client
                netif_set_addr(&gnetif, &ipaddr, &netmask, &gw);
                break;
            }
        }

        if(dhcp_supplied_address(&gnetif))
        {
            printf("DHCP Success!\r\n");
        }
    #else
        printf("Using static IP configuration\r\n");
    #endif

    // Print IP Configuration
    printf("\r\nIP Configuration:\r\n");
    printf("IP Address: %d.%d.%d.%d\r\n",
           ip4_addr1(&gnetif.ip_addr),
           ip4_addr2(&gnetif.ip_addr),
           ip4_addr3(&gnetif.ip_addr),
           ip4_addr4(&gnetif.ip_addr));

    printf("Subnet Mask: %d.%d.%d.%d\r\n",
           ip4_addr1(&gnetif.netmask),
           ip4_addr2(&gnetif.netmask),
           ip4_addr3(&gnetif.netmask),
           ip4_addr4(&gnetif.netmask));

    printf("Gateway: %d.%d.%d.%d\r\n",
           ip4_addr1(&gnetif.gw),
           ip4_addr2(&gnetif.gw),
           ip4_addr3(&gnetif.gw),
           ip4_addr4(&gnetif.gw));
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
