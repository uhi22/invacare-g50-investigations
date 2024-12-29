
/* interface for can_lowlayer.c */

#define USE_ACTIVE_CONTROL
#define USE_TX_QUEUE
#define CAN_TX_QUEUE_LENGTH 20

#include "main.h"
#include "string.h" /* for memcpy */
#include "hardwareAbstraction.h"

extern CAN_TxHeaderTypeDef   TxHeader;
extern uint8_t               TxData[8];
extern uint32_t              TxMailbox;
extern CAN_RxHeaderTypeDef canRxMsgHdr;
extern uint8_t canRxData[8];

extern uint32_t nNumberOfReceivedMessages;
extern uint32_t nNumberOfCanInterrupts;
extern uint16_t nNumberOfCanTxCompleteInterrupts;
extern uint16_t debug_CAN_IRQ_Counter;
extern uint32_t debug_CAN_interruptenablebits;
extern uint32_t debug_CAN_tsrflags;
extern uint8_t canTxQueueUsedSize;
extern uint8_t canTxQueueUsedSizeMax;

extern uint32_t get_tAfterFirstTxMessage_ms(void);
extern void can_lowlayer_registerCallbacks(void);
