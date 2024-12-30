
#include "hardwareAbstraction.h"

uint8_t blTP21toggle;
uint8_t blTP22toggle;
uint8_t blTP33toggle;

void setKeepPower(uint8_t on) {
	if (on) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); /* keep power on */
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); /* turn power off */
	}
}

void setBusWake(uint8_t on) {
	if (on) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); /* keep power on */
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); /* turn power off */
	}
}

void setLED_D11(uint8_t on) {
	if (on) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
	}
}

void setLED_D1(uint8_t on) {
	if (on) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	}
}

void setLEDAlive(uint8_t on) {
	if (on) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
	}
}

void setOut1(uint8_t on) {
	if (on) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
	}
}

void setOut2(uint8_t on) {
	if (on) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
	}
}

void setOut3(uint8_t on) {
	if (on) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
	}
}

void setOutTP1(uint8_t on) {
	if (on) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
	}
}
void setOutTP33(uint8_t on) {
	if (on) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
	}
}
void setOutTP21(uint8_t on) {
	if (on) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
	}
}
void setOutTP22(uint8_t on) {
	if (on) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
	}
}


/* buttons and switches are low-active. The functions return TRUE if the button is pressed. */
uint8_t getSwitch1(void) {
	return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_4)==0;
}
uint8_t getSwitch2(void) {
	return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5)==0;
}
uint8_t getSwitch3(void) {
	return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6)==0;
}
uint8_t getSwitch4(void) {
	return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7)==0;
}
uint8_t getButton1(void) {
	return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8)==0;
}
uint8_t getButton2(void) {
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)==0;
}
uint8_t getButton3(void) {
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15)==0;
}
uint8_t getButton4(void) {
	return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0;
}
uint8_t getJoystickButton(void) {
	return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9)==0;
}
uint8_t getPowerButton(void) {
	return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10)==0;
}


