
/* interface for canbus.c */
extern void can_init(void);

extern uint32_t nNumberOfReceivedMessages;
extern uint32_t nNumberOfCanInterrupts;
extern uint16_t nNumberOfCanTxCompleteInterrupts;
extern uint16_t debug_CAN_IRQ_Counter;
extern uint32_t debug_CAN_interruptenablebits;
extern uint32_t debug_CAN_tsrflags;

extern uint32_t canRxDataUptime;
extern uint16_t canRxCheckpoint;

extern CAN_HandleTypeDef hcan;
extern CAN_RxHeaderTypeDef canRxMsgHdr;
extern uint8_t canRxData[8];

extern CAN_TxHeaderTypeDef   TxHeader;
extern uint8_t               TxData[8];
extern uint8_t               TxDataLowlayer[8];
extern uint32_t              TxMailbox;

extern uint16_t canTxQueueOverruns;
extern uint16_t can_txMailboxFreeCounter;
extern uint16_t canTxQueueSuccessfulQueuedCounter;

extern void canEvaluateReceivedMessage(void);
extern void can_mainfunction5ms(void);
extern void tryToTransmit(uint16_t canId, uint8_t length);
extern void can_transferTxQueueToHardware(void);
extern void can_mailbox0_complete_irq(CAN_HandleTypeDef *pcan);

extern uint8_t ucmJoystickX, ucmJoystickY;
extern uint8_t servoLightState;
extern uint8_t motorUBattRaw;
extern uint8_t ucmLightDemand;
extern int8_t servoPosition;
extern uint8_t flasherDivider;

extern uint8_t isSubscribedNv2C;
extern uint8_t isSubscribedNv2D;
extern uint8_t isSubscribedNv2E;
extern uint8_t isSubscribedNv2F;
extern uint8_t isSubscribedNv30;
extern uint8_t isSubscribedNv31;
extern uint8_t isSubscribedNv32;
extern uint8_t isSubscribedNv33;
extern uint8_t isSubscribedNv34;
extern uint8_t isSubscribedNv35;
