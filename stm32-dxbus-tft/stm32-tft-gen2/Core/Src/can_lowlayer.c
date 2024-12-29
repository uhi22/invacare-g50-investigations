
/* The lower layer CAN handling.
  - interrupt routines for CAN message reception and transmit-complete
  - transmit queue handling
*/

#include "can_lowlayer.h"
#include "can_application.h"

uint32_t tOfFirstTxMessage_ms;

uint32_t nNumberOfReceivedMessages;
uint32_t nNumberOfCanInterrupts;
uint16_t nNumberOfCanTxCompleteInterrupts;
uint16_t debug_CAN_IRQ_Counter;
uint32_t debug_CAN_interruptenablebits;
uint32_t debug_CAN_tsrflags;

CAN_TxHeaderTypeDef   TxHeader;
uint8_t               TxData[8];
uint32_t              TxMailbox;
CAN_RxHeaderTypeDef canRxMsgHdr;
uint8_t canRxData[8];

uint8_t canTxQueueWriteIndex;
uint8_t canTxQueueReadIndex;
uint16_t canTxQueueID[CAN_TX_QUEUE_LENGTH];
uint8_t canTxQueueLen[CAN_TX_QUEUE_LENGTH];
uint8_t canTxQueueData[CAN_TX_QUEUE_LENGTH*8];
uint16_t canTxQueueOverruns;
uint16_t can_txMailboxFreeCounter;
uint16_t canTxQueueSuccessfulQueuedCounter;
uint8_t  TxDataLowLayer[8];


/* Queue strategy: We want to keep the order of the messages. We do not want to consider the
 * priority of the messages. That's why we use only a single hardware mailbox, and take
 * the messages one-after-the-other from the software queue.
 * Because: If we would use multiple hardware mailboxes, we would run into the situation,
 * that a motor response message (ID=008) would be on the bus BEFORE the UCM request message (ID=040),
 * if they both are set nearly at the same point in time with the correct order.
 */
void can_transferTxQueueToHardware(void) {
	if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan)>2) {
		/* we have (all) hardware mailboxes free */
		can_txMailboxFreeCounter++;
		if (canTxQueueReadIndex!=canTxQueueWriteIndex) {
			/* there is something in the queue */

			 TxHeader.StdId = canTxQueueID[canTxQueueReadIndex];
			 TxHeader.DLC = canTxQueueLen[canTxQueueReadIndex];
			 memcpy(TxDataLowLayer, &canTxQueueData[8*canTxQueueReadIndex],8);
			 if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxDataLowLayer, &TxMailbox) != HAL_OK) {
				canTxErrorCounter++; /* todo: recovery */
			 } else {
				canTxOkCounter++;
			 }
			 canTxQueueReadIndex++; if (canTxQueueReadIndex>=CAN_TX_QUEUE_LENGTH) canTxQueueReadIndex = 0;
		}
	}
}

/* the CAN receive interrupt */
void can_rx_irq(CAN_HandleTypeDef *pcan) {
  nNumberOfCanInterrupts++;
  HAL_StatusTypeDef rc;
  rc = HAL_CAN_GetRxMessage(pcan, CAN_RX_FIFO0, &canRxMsgHdr, canRxData);
  if (rc==HAL_OK) {
    nNumberOfReceivedMessages++;
    canEvaluateReceivedMessage();
  }
}

/* the transmission-complete interupt */
void can_mailbox0_complete_irq(CAN_HandleTypeDef *pcan) {
  nNumberOfCanTxCompleteInterrupts++;
  can_transferTxQueueToHardware();
}

/* during init, we register our interrupt service routines in the HAL */
void can_lowlayer_registerCallbacks(void) {
  /* The callback registration only compiles, if the callback is enabled. To do this,
   * in the STM configuration tool, go to ProjectManager, AdvancedSettings, on the
   * right side there is the window "Register Callbacks", and there set CAN to ENABLE.
   */
  if (HAL_CAN_RegisterCallback(&hcan, HAL_CAN_RX_FIFO0_MSG_PENDING_CB_ID, can_rx_irq)) {
    Error_Handler();
  }
  if (HAL_CAN_RegisterCallback(&hcan, HAL_CAN_TX_MAILBOX0_COMPLETE_CB_ID, can_mailbox0_complete_irq)) {
    Error_Handler();
  }
  if (HAL_CAN_RegisterCallback(&hcan, HAL_CAN_TX_MAILBOX1_COMPLETE_CB_ID, can_mailbox0_complete_irq)) {
    Error_Handler();
  }
  if (HAL_CAN_RegisterCallback(&hcan, HAL_CAN_TX_MAILBOX2_COMPLETE_CB_ID, can_mailbox0_complete_irq)) {
    Error_Handler();
  }
}





void tryToTransmit(uint16_t canId, uint8_t length) {
uint8_t newWriteIndex;
 #ifdef USE_ACTIVE_CONTROL
	 if (tOfFirstTxMessage_ms==0) {
		 tOfFirstTxMessage_ms = HAL_GetTick(); /* store the time of the first transmitted message as reference. */
	 }
#ifdef USE_TX_QUEUE
	 newWriteIndex = canTxQueueWriteIndex;
	 newWriteIndex++; if (newWriteIndex>=CAN_TX_QUEUE_LENGTH) newWriteIndex = 0;
	 if (newWriteIndex==canTxQueueReadIndex) {
		 /* the new writeIndex would hit the readIndex, so we have a full queue. */
		 canTxQueueOverruns++;
	 } else {
	     /* we have free space in the queue. Fill the data. */
		 canTxQueueID[canTxQueueWriteIndex] = canId;
		 canTxQueueLen[canTxQueueWriteIndex] = length;
		 memcpy(&canTxQueueData[8*canTxQueueWriteIndex], TxData, 8);
		 canTxQueueWriteIndex=newWriteIndex;
		 canTxQueueSuccessfulQueuedCounter++;
	 }
	 HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
	 can_transferTxQueueToHardware(); /* try to transfer the message to the CAN hardware */
	 HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
	 //informAboutCanMessage(canId, TxData[0]);

#else
	 TxHeader.StdId = canId;
	 TxHeader.DLC = length;
	 if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan)>0) {
		 if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox) != HAL_OK) {
			canTxErrorCounter++; /* todo: recovery */
		 } else {
			canTxOkCounter++;
		 }
	 } else {
		 /* no free mailbox */
		 canTxErrorCounter++;
	 }
	 powermodule_informAboutCanMessage(canId, TxData[0]);
#endif
#endif
}


uint32_t get_tAfterFirstTxMessage_ms(void) {
	/* provides the time in milliseconds, measured after the first transmit message. */
	uint32_t t;
	t = HAL_GetTick();
	if ((tOfFirstTxMessage_ms>0) && (t>tOfFirstTxMessage_ms)) {
		return t-tOfFirstTxMessage_ms;
	} else {
		/* we have no first CAN message, or no time spent since it. So we say that the elapsed time
		 * is zero. */
		return 0;
	}
}

