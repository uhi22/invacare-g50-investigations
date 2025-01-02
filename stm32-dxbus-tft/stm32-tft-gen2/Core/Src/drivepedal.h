
#include "stm32f1xx_hal.h"
#include "analogInputs.h"
#include "buttons.h"

extern float drp_u_pot1_V;
extern float drp_pot1_raw_percent;
extern float drp_pot1_plausi_percent;
extern float drp_speedrequest_percent;

extern void drivepedal_mainfunction20ms(void);