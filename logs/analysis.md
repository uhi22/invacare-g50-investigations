# 2024-12-28_CAN_gen2_run_but_stopped_002.csv

872,00000010,false,Rx,9,7,B0,01,25,11,25,0D,00,00,
878,00000010,false,Rx,9,3,B0,0B,E9,00,00,00,00,00, dT 6ms
883,00000010,false,Rx,9,3,B0,0E,00,00,00,00,00,00, dT 5ms
898,00000010,false,Rx,9,3,B0,0B,E9,00,00,00,00,00, dT 15ms
918,00000010,false,Rx,9,3,B0,0B,E9,00,00,00,00,00, dT 20ms
931,00000008,false,Rx,9,4,30,02,00,11,00,00,00,00,

Observed:
- The power module (00008) requests "11" from the SLM.
- This is because the SLM misses to send the long message B0,01,25,11,25,0D,00 in the expected 20ms cycle.
Improvement: In the tx queue handling, only mark the
message as "transmitted" if it was successfully given to the HAL.

