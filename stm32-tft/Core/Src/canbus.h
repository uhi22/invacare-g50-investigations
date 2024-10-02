
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
extern void can_mainfunction5ms(void);

extern uint8_t ucmJoystickX, ucmJoystickY;
extern uint8_t ucmState, motorState, servoLightState;
extern uint8_t motorUBattRaw;
extern uint8_t ucmLightDemand;
extern int8_t servoPosition;
