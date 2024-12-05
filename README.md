# stm32f401xx-enc28j60-lwip
programming ethernet connection using stm32f401

---

## 📦 Required devices
- **STM32F401 Development Board**
- **ENC28J60 Ethernet Module**
- **Ethernet Cable (RJ45)**
- **Breadboard and jumper cable**
- **Power supply as needed**

---

## 🔌 Skema Koneksi Perangkat
Hubungkan pin STM32F401 ke ENC28J60 sesuai tabel berikut:

| **STM32F401 Pin** | **ENC28J60 Pin** |
|--------------------|------------------|
| GND                | GND             |
| 3.3V               | VCC             |
| PA5 (SPI_SCK)      | SCK             |
| PA6 (SPI_MISO)     | SO              |
| PA7 (SPI_MOSI)     | SI              |
| PA4 (SPI_CS)       | CS              |
| PA8                | RESET           |
| PB2                | INT             |

**Note:**
- Make sure the ENC28J60 uses a 3.3V power supply.

---

## 🛠️ Software Installation

### 1. **Installing Toolchain**
- Download and install **STM32CubeIDE** from [STMicroelectronics](https://www.st.com).
- Make sure you have the appropriate STM32 HAL library (FreeRTOS).
  
### 2. **Adding ENC28J60 and LwIP Library**
- The **LwIP & ENC28J60** library is available in this repository. 

---

## ⚙️ STM32 Project Configuration

### 1. **Setting SPI**
1. Open **STM32CubeMX** and create a new project.
2. Enable **SPI1**:
   - Mode: *Full Duplex Master*
   - Prescaler: Adjust the baud rate to suit your needs.
3. Set SPI pins:
   - PA5: SPI1_SCK
   - PA6: SPI1_MISO
   - PA7: SPI1_MOSI
   - PA4: SPI1_CS
4. Enable clock for GPIO and SPI.
5. Enable pins UART for debuging :
   - PA2 : USART2_TX
   - PA3 : USART2_RX

   
### 2. **Clock Configuration**
- Set the system clock using **HSE/PLL** for high performance.

---

### ENC28J60 Function Implementation

Use functions from the ENC28J60 library to:

- Module initialization
- Sending data packets via Ethernet

---

### 🧪 Testing

1. Connect the ENC28J60 to a router or switch using an Ethernet cable.
2. Run the code on the STM32.
3. Use an application such as **Wireshark** to monitor the data packets sent.

4. ---

## 🎉 Conclusion

By following this tutorial, you can now connect the STM32F401 with the ENC28J60 module for Ethernet communications. This tutorial provides basic steps, but you can expand it further to suit your project needs. If you have any questions, suggestions, or problems, don't hesitate to open an *issue* in this repository or contact us.

Good luck, and good luck! 🚀
