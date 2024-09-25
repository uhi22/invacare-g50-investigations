
/* interface for canbus.c */
extern void can_init(void);


extern uint32_t canRxDataUptime;
extern uint16_t canRxCheckpoint;

extern CAN_HandleTypeDef hcan;
extern CAN_RxHeaderTypeDef canRxMsgHdr;
extern uint8_t canRxData[8];

extern CAN_TxHeaderTypeDef   TxHeader;
extern uint8_t               TxData[8];
extern uint32_t              TxMailbox;

extern void canEvaluateReceivedMessage(void);

