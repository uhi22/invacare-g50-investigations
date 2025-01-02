

#include "drivepedal.h"
#include "errors.h"

float drp_u_pot1_V;
float drp_pot1_raw_percent;
float drp_pot1_plausi_percent;
float drp_speedrequest_percent;
uint8_t drp_errorDebounceCounter;

#define POT_FULL_SCALE_VOLTAGE (6.6f) /* voltage divider is 47k/47k, so the input voltage for 3.3V is 6.6V */
#define POT1_VOLT_RELEASED (1.0f)
#define POT1_VOLT_FULLYDEPRESSED (4.0f)

#define REVERSE_DRIVING_SWITCH ((buttonField & BUTTON_MASK_SWITCH1)!=0)

void drivepedal_mainfunction20ms(void) {
    float f;
    
    f = adcValues[ADC_CHANNEL_POTI1];
    f *= POT_FULL_SCALE_VOLTAGE; /* volts fullscale */
    f /=4095; /* ADC fullscale */
    drp_u_pot1_V = f;
    
    f = drp_u_pot1_V - POT1_VOLT_RELEASED;
    drp_pot1_raw_percent = f * 100.0f / (POT1_VOLT_FULLYDEPRESSED - POT1_VOLT_RELEASED);
    /* the raw can be negative and above 100%, depending on the parameters and the input voltage. */
    
    if (drp_pot1_raw_percent<-20) {
        /* 20% below the idle position. This is very low. This is an error. */
        drp_pot1_plausi_percent = 0;
        drp_errorDebounceCounter++;
        if (drp_errorDebounceCounter>25) globalError = ERR_DRIVEPEDAL_SIGNAL_TOO_LOW;
    } else if (drp_pot1_raw_percent<0) {
        drp_pot1_plausi_percent = 0; /* slightly too low -> treat as zero */
        drp_errorDebounceCounter=0;
    } else if (drp_pot1_raw_percent<=100) {
        drp_pot1_plausi_percent = drp_pot1_raw_percent; /* normal case */
        drp_errorDebounceCounter=0;
    } else if (drp_pot1_raw_percent<=120) {
        drp_pot1_plausi_percent = 100; /* slightly overpressed is still fully pressed */
        drp_errorDebounceCounter=0;
    } else {
    	/* 20% above the fully depressed position. This is an error. */
    	drp_errorDebounceCounter++;
    	if (drp_errorDebounceCounter>25) globalError = ERR_DRIVEPEDAL_SIGNAL_TOO_HIGH;
        drp_pot1_plausi_percent = 0; /* too high -> treat as zero, even if the error debouncing is still ongoing. */
    }
    /* now we have a plausibilized pedal position in 0 to 100% range. */
    
    /* Calculate the speed request, considering the pedal position and the direction switch */
    drp_speedrequest_percent = drp_pot1_plausi_percent;
    if (REVERSE_DRIVING_SWITCH) {
        drp_speedrequest_percent = -drp_speedrequest_percent;
    }
}
