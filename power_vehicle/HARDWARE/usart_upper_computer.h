#ifndef __USART_UPPER_COMPUTER_H__
#define	__USART_UPPER_COMPUTER_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "common_def.h"

#define COMM_RCC_UART				RCC_APB1Periph_USART2
#define COMM_RCC_UART_GPIO	RCC_APB2Periph_GPIOA
#define COMM_UART_GPIO			GPIOA
#define COMM_UART_TX_GPIO		GPIO_Pin_2
#define COMM_UART_RX_GPIO		GPIO_Pin_3
#define COMM_UART						USART2
#define COMM_UART_IRQ				USART2_IRQn

#define COMM_DATA_BUFF_SIZE	1

#define COMM_OTA_SYN_HEAD 	0x6688
#define COMM_OTA_SYN_HEAD1	0x88
#define COMM_OTA_SYN_HEAD2	0x66

#define COMM_POWER_VEHICLE_CONNECT		0x49
#define COMM_POWER_VEHICLE_READ_SPEED	0x44
#define COMM_POWER_VEHICLE_SET_POWER	0x57
#define COMM_POWER_VEHICLE_CHECK			0x0d

#define COMM_OTA_OFFSET(member) (UINT32)&(((struct ota_protocol *)0)->member)

typedef enum {
  UPPER_COMPUTER_RD800_DATA = 0,
  UPPER_COMPUTER_OTA_DATA,
  UPPER_COMPUTER_OTHER_DATA
} UPPER_COMPUTER_DATA_TYPE;

enum main_command {
	MAIN_COMMAND_START = 0,
	MAIN_COMMAND_OTA = MAIN_COMMAND_START,
	MAIN_COMMAND_END,
};

enum ota_sub_command {
	OTA_SUB_COMMAND_START = 0,
	OTA_SYN_HEAD = OTA_SUB_COMMAND_START,
	OTA_PROGRAM_FLASH,
	OTA_CHECKSUM_FLASH,
	OTA_RUN_BOOTLOADER,
	OTA_RUN_APPLICATION,
	OTA_SUB_COMMAND_END,
};

struct ota_protocol {
	UINT16 	synHead;			// 0x6688
	UINT32 	checkSum;			// check sum
	UINT8		mainCommand;	// main command value
	UINT8 	subCommand;		// sub command value
	UINT16 	dataLen;
	UINT8 	data[COMM_DATA_BUFF_SIZE];
} __packed;

struct comm_ota_data {
	UINT8 state;
	UINT16 rxCnt;
	struct ota_protocol *otaData;
} __packed;

struct comm_power_vehicle_data {
	UINT8 state;
	UINT16 rxCnt;
	UINT8 *powVehData;
} __packed;

struct upper_computer {
  UPPER_COMPUTER_DATA_TYPE dataType;
  struct ota_protocol* ota;
  UINT8 *rd800;
} __packed;

#ifdef __cplusplus
}
#endif

#endif 
