
# Invacare G50 Investigations

Investigating an electric wheel chair

![image](doc/2024-10-03_collage_titlepage.jpg)

## Progress

### 2025-01-07: DX-DASH Generation 2 ready for testing

The DX-DASH generation 2 consists of a board containing an STM32 controller, designed in KiCad,
a housing designed in FreeCad and the software designed in the STM CubeIDE.
The DX-DASH gen2 is controlling the Power Module, using either joystick or drive pedal and
direction switch.

![image](doc/2025-01-07_dxdash_gen2_collage.jpg)

[Instruction manual](doc/2025-01-06_DXDASH_installation_und_bedienung.md)

### 2024-10-15: STM32 simulates the UCM and the SLM

Simulating the UCM and the ServoLightModule, the STM32 is able to control
the motor speed directly.

### 2024-10-11: STM32 as UCM-replacement controls all functionality

The STM32 as UCM replacement controls a lot of functionality:
- Wakeup the other modules via the DXBUS
- Control the low beam light and the flashers
- Switch the power module into driving mode and back to idle mode
- Drive motor forwards and backwards using analog joystick input
- Turn the steering servo left and right using analog joystick input
- Send the power module and servo light module to sleep

### 2024-10-02: First spin

The STM32 sends all CAN messages to make the motor spin.

### 2024-09-30: Steering reacts

By sending some half-way proper CAN messages from the STM32 UCM replacement, the servo can be controlled to right-turn.
The motor disables the brake, but does not spin yet. It blinks 12 times.

### 2024-09-26: Controlling the lights works

Using the STM32 "blue pill" and a CAN transceiver, we control the lights via CAN. Due to the disconnected UCM,
the motor controller is stuck in state 0C, and the ServoLightModule in state 0. But nevertheless, low beam and
the turn indicators can be perfectly controlled with a 20ms message "0x040 B0 14 xx".
The ServoLightModule works even with very low battery voltage of 10V in this situation.

### 2024-09-24: STM32 interprets some network variables

Using the STM32 and attached display, we decode some network variables out of the CAN messages,
and show them on the display. 

## Goals

1. Understand the communication on the DX-BUS
2. Replace the original joystick ("UCM") by a self-made box which can control the motor, the lights and the steering.
3. Replace the UCM and the ServoLightModule by a self-made box to control only the motor.

## Pinout of the Power Module

The "Power Module" is a DX-PMB2-SAS, producer "Dynamic Controls", this seems to be the same as the DX-PMB2 mentioned in Ref1.

## Bus Communication

The "DXBUS" consists of four pins:
* Ground
* 24V
* CANH
* CANL

The ground and 24V are directly connected to the battery pins of the power module.

CAN bit time is 9.5µs, this means the CAN baud rate is 105.26 kBaud.
This baud rate can be created (according to STM32 cube IDE) with prescaler=57, T1=3, T2=2, SJW=1, TQ=1583.3ns, bittime=9500ns, 6 TQ per bit, inputClock=36MHz.

### CAN messages

UCM transmit messages (found by adding a separate CAN transceiver to the UCMs CAN_TX line):

* 0x040: The main data message. Contains lot of different sub-data.
* 0x0AA: not very interesting. Always "AA 04".
* 0x29C: "03 00 1A" during run. While turning off, "03 00 10".
* 0x2B4: Constant besides a toggle bit
* 0x2BC: nearly constant, only the last byte changes sometimes.
* 0x3A4: nearly constant, only the last byte changes during shutdown.
* 0x3AC: constant "03 00 1D"

ServoLightingModule transmit messages (found by adding a separate CAN transceiver to the ServoLightingModule CAN_TX line):

* 0x010 is the only transmit message. It contains various sub-messages, e.g.
    * 30 08 00 01 after startup
    * "B0 01 00" and "B0 01 02"
    * "10 00 ....", "12 00 ..."
    * and more

MotorModule transmit messages (all remaining messages which do not come from UCM and ServoLightingModule)
* 0x008 is the only transmit message. It contains various sub-messages, e.g.
    * A3, A1, A4 after startup
    * B0 01 02
    * B0 01 10 0F ...
    * B0 01 10 0C ...
    * B0 01 20 ...
    * B0 01 22
    * B0 01 25 
    * B0 0E ...
    * 30 02 ...
    * and more

```
0x008 B0 01 20 0C B5 0F FF
                   |
                   depends on the supply voltage. A7=22V, B6=24V, C6=26V. 150=19.8V, 200=26.5V 
                   
                   
                  
0x040 B0 93 E1 00 14 00
                      |
                      lights 00=off, 11=light, 02/22 blinker right, 04/44 blinker left, 08/88 hazard flashing
                      
                      
0x040 B0 01 25 90 FF 80
                   |  |
                   |  joystick left/right. 01 is full left. 80 is middle. FF is full right.
                   joystick up/down. 01 is full backwards. 80 is middle, FF is full forward.
                   
0x2B4 03 00 0A
             |
             different values: 02, 0A. And 00 before power-off.

0x2BC 03 00 0F
             |
             different values: 0B, 0F. And 01 before power-off.
```

#### Message types

- First byte B0: "Network Variable announcement". After the B0 there is one byte NV_ID, followed by the content of the
network variable. The length depends on the ID. Afterwards, more {NV_ID, value} pairs may follow as long as there is space left in the CAN frame.
It is confirmed, that the same network variable can be on different position: The light control variable 14 works for controlling the lights in
the following frames in the same way:
    * 0x040 B0 93 E1 00 14 11
    * 0x040 B0 14 11
    

- First byte 30: "Wanna know". A node sends this message, to request network variable from other nodes.
```
   I'm the motor and...
   |   I wanna know...
   |   |  from node 2 (ServoLightModule)...
   |   |  |     the network variable 11
   |   |  |     |
0x008 30 02 00 11
```
The addressed node responds accordingly with the value of the requested network variable:
```
   I'm the ServoLightModule and...
   |   I want to announce my network variable...
   |   |  number 11...
   |   |  |   has the value 10.
   |   |  |  |
0x010 B0 11 10
```

After the variable number in the "30" message, there is sometimes more data, sometimes not. E.g. "03". Unclear: Is this an additional
network variable number? Or is it a kind of "request type" which selects the cycle time or number of responses or on/off?

- First byte 31: "unsubscribe". Tells the addressed note that it does not need to send the network variable anymore. E.g.
```
   I'm the ServoLightModule and...
   |   I do not need...
   |   |  from node 8 (UCM)...
   |   |  |     the network variable 2E
   |   |  |     |
0x010 31 08 00 2E
```
After this unsubscription, the UCM stops the cyclic transmission of NV 2E.

- First byte 20: unclear. Used in the first messages after startup. 4297725430,00000040,false,Rx,9,8,20,00,00,08,00,00,00,00,
- First byte 23: unclear. E.g. 00000040,false,Rx,9,8,23,00,00,08,FC,80,00,00. Responded by A3.
- First byte 21: similar to 23
- First byte 24: similar to 23


Request-response:
* 4297804953,00000040,false,Rx,9,8,23,00,00,08,FC,80,00,00,  request from the UCM
* 4297805609,00000008,false,Rx,9,1,A3                        response from Motor, where A3 seems to be the response to 23
* 4297806161,00000010,false,Rx,9,1,A3                        response from Servo, where A3 seems to be the response to 23
(and repeated)

again with other number:
* 4297818227,00000040,false,Rx,9,8,21,00,00,08,FC,80,00,00,  request from the UCM
* 4297818888,00000008,false,Rx,9,1,A1                        response from Motor
* 4297819437,00000010,false,Rx,9,1,A1                        response from Servo
(and repeated)

again with other number:
* 4297830564,00000040,false,Rx,9,8,24,00,00,08,FC,40,00,00,  request from the UCM
* 4297832560,00000008,false,Rx,9,1,A4                        response from Motor
* 4297832579,00000010,false,Rx,9,1,A4                        response from Servo
(and repeated)


### Profile changes

If the user changes the driving profile ("gear"), e.g. from 2 to 3, this results in
1. 040: B0,93,63,00,14,00 (once) The UCM announces profile 3.
2. 040: B0,93,E3,00 (repeated each 200ms) The UCM announces profile 3 in an other way.
3. 010: 30,08,00,2C The servo light module requests from the UCM the NV 2C.
4. 040: B0,93,E3,00,14,00,2C,66 UCM provides the NV 2C. Repeated after 200ms.
5. 010: 31,08,00,2C The servo light module stops the subscription of NV 2C.
6. The request/response/unsubscribe continues for other variables.

The end speed (at full joystick way) is parametrized separately for forward and reverse directions. Example profiles,
to demonstrate the speed influence of NV 2C (forward speed) and NV 2F (reverse speed):

```
		/* profiles:
		  1     2     3     4     5  */
		0x10, 0x59, 0x66, 0x73, 0xff, /* NV 0x2C */ /* max speed fwd. 10=very low, ff=high speed */
		0x66, 0x66, 0x73, 0x80, 0x80, /* NV 0x2D */
		0x20, 0x80, 0x80, 0x80, 0x80, /* NV 0x2E */
		0x10, 0x66, 0x66, 0x66, 0xff, /* NV 0x2F */ /* max speed reverse. 10=very low, ff=high speed */
		0x66, 0x66, 0x73, 0x80, 0x80, /* NV 0x30 */
		0x99, 0x99, 0x99, 0x99, 0x99, /* NV 0x31 */
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, /* NV 0x32 */
		0xB3, 0xB3, 0xB3, 0xB3, 0xB3, /* NV 0x33 */
		0x33, 0x33, 0x4D, 0x4D, 0x4D, /* NV 0x34 */
		0x80, 0x80, 0x99, 0xA6, 0xA6, /* NV 0x35 */
```

These profiles are used only by the ServoLightModule. The motor module is not interested into these profiles. The motor
module is controlled by network variable 0B, which the ServoLightModule provides.

### Error Codes

The PowerModule provides with the network variable 07 some error information:
- 00: no error. No blink code on the UCM.
- 0C: uncoupled the G40 motor. Blink code 3 on the UCM.
- 10: Motor M1 disconnected. Blink code 5 on the UCM.
- (more to be tested)

To use this error reporting, the NV 07 needs to be requested once at the beginning:
`00000010,false,Rx,9,4,30,01,00,07`

## The Wakeup

As long as the system is "off", the DX-BUS still carries 24V battery voltage, and both CAN lines are sitting at ~2.5V.
The system is activated when the user pushes the on/off button on the joystick module ("UCM"). The UCM wakes the other
control units by pulling the CANH to 24V for ~40ms. Afterwards, the UCM sends CAN traffic to keep the other control units
awake. If we apply just the wakeup pulse and do not send any CAN messages, the control units go to sleep again after five seconds.

![image](doc/2024-09-22_osci_wakeup.jpg)

## The Go-To-Sleep

If the user pushes the on/off button while the system is running, the UDS sends some special messages to send the other control
units to sleep, and stops the CAN communication. (Todo: Details to be described.)
If we just stop sending any messages from the UCM, the ServoLightModule enters an "emergency mode" after some seconds, where
it activats the hazard-flashing.

## Using the ServoLightModule standalone

The ServoLightModule can be used without the PowerModule to control the lights and the steering servo. Several aspects need to be considered to make this work.

1. The CAN termination. The absence of the PowerModule causes a missing CAN termination. We need to add a 510 ohm (or 470ohm) between
CANH and CANL. Additionally, we need a pull-up of the CANH via a diode and 390ohms (in series) to 3.3V, because the CAN transceiver inside
the SLM seems not to be capable of driving the CANH to high; it only pulls the CANL to low. So without additional pull-up on CANH,
a normal CAN transceiver is not able to pick-up the messages of the SLM.

2. The light control. This is the easy part. The SLM listens to the network variable 0x14, which we can announce e.g. with a message `0x040 B0 93 E1 00 14 00`. The light control bits are explained above. The SLM does not need to see the PowerModule for this functionality.

3. The steering servo control. This part is more tricky. To make the steering function work, the SLM needs to see the PowerModule and needs
to go through the corrects states.

    - The UCM and PowerModule must simulate the 23/A3, 21/A1 and 24/A4 sequences during startup.
    - The PowerModule must send the status after the SLM requests it with 010 30 01 00 01.
    - The PowerModule must send the network variables 0F, 0E and 07 after the SLM requested it with 010 30 01 00 0F or 010 30 01 00 0E or 010 30 01 00 07. If the SLM does not receive them, it changes from state 10 (init) to state 0C (error).
    - Even if the SLM now removes the steering brake when entering state 0x25, still the steering only works sporadically. To be further investigated.

## UCM (Joystick Control Module)

CAN_TX: Is the pin 1 of the 74HC02 on the sub-board.

### Diagnostic Connector

according to ref1, this is RS232.

## The Motor Controller (Power Module, PM)

Pinout: ![image](doc/2024-10-04_motorConnection.jpg)

Motor PWM control:

![image](doc/2024-10-04_osci_motorPwmAtStandstill.jpg)

![image](doc/2024-10-04_osci_motorPwmAtFullForward.jpg)

Park brake control:

* The black park brake wire is grounded.
* The yellow park brake wire is switched to 24V to release the brake. Each 200ms, the PM turns the park brake off for 250µs, most likely for diagnotic purposes.

Current consumption:

* in sleep: 600µA. This means, a 20Ah usable battery capacity lasts for more than 3 years.

## The Motor

* Does does a motor of a G40 work at the motor controller of the G50? Yes.
* How does the "manual clutch switch" work?
    * In the G50 motor, when the user actuates the clutch to push the chair manually, the motor-integrated switch disconnects the *park brake magnet*.
    * In the G40plus motor, the motor integrated switch directly disconnects the *motor*.

## Self-made DXBUS controller

An STM32 development board ("blue pill"), combined with a ILI9341 TFT display and a SN65HVD230 CAN transceiver board and some small additions
is used to send and receive the CAN messages on the DXBUS, and also to send the wakeup pulse.

![image](doc/2024-10-16_dxbus_printed_plug.jpg)
![image](doc/2024-10-16_dxbus_stm32.jpg)
![image](doc/2024-10-16_dxbus_stm32_backside.jpg)
![image](doc/2024-10-16_dxbus_stm32_in_action.jpg)
![image](doc/2024-10-16_dxbus_stm32_powersupply.jpg)


The 3D model for the printed DXBUS plug is available as [FreeCad model](3d_models/DXBUS_plug.FCStd) and [step file](3d_models/DXBUS_plug.step).

The software (as STM CubeIDE project) is available [here](stm32-tft).

## Open Todos

- [ ] ServoPos shows permanent 0
- [ ] add direction switch and pedal input
- [ ] implement special case "power button during driving"
- [ ] improve CAN scheduling to avoid message losses

## Finished Todos

- [x] Send NV 92 with FF FF in 200ms cycle
- [x] Send the profile (e.g. NV 93 = E1 00)
- [x] Send the parameters of the profile (NVs 2C to 35), after the ServoLight 0x010 requested it, e.g.
    - servoLight requests the 2C from UCM with 00000010,false,Rx,9,4,30,08,00,2C
    - ...
    - servoLight requests the 2E from UCM with 00000010,false,Rx,9,4,30,08,00,2E
    - more general: UCM needs to satisfy all requests (search for 30,08,00)
- [x] Find out how to parametrize the speed
- [x] Reset the CAN controller if it enters bus-off
- [x] Implement UCM start/idle/drive/idle/shutdown state machine
- [x] implement emulation of ServoLightModule, to be able to run the motor control unit without physical ServoLightModule

## Cross References

* [Ref1] System manual for the DXBUS https://www.dynamiccontrols.com/sites/default/files/2018-05/dx-system-manual-issue1.pdf
* [Ref2] European Patent EP0725738A1 (year 1993) https://patents.google.com/patent/EP0725738A1/en
* [Ref3] European Patent EP0725738B1 (year 1993), including schematics, state machines, some network variables. https://patents.google.com/patent/EP0725738B1/en
* [Ref4] SavvyCAN open source CAN logging tool https://www.savvycan.com/
* [Ref5] wifican CAN adaptor via wifi. Adapted to support the 105kBaud of the DXBUS. https://github.com/uhi22/wifican/commit/1e717946a06a594cf2e79b092cbaf7b376e8bb55
* [Ref6] pictures and document collection for the Invacare G50 https://hnng.de/invacare-g50/
* [Ref7] Master Thesis of Markus Schuch / Technikum Wien including a good overview over different wheel chair concepts, and a raspberry-pi based CAN "man in the middle" approach for the DXBUS. https://epub.technikum-wien.at/urn/urn:nbn:at:at-ftw:1-39670 
