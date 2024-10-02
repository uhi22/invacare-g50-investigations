
# Invacare G50 Investigations

Investigating an electric wheel chair

![image](doc/foto1.jpg)

## Progress

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
                   depends on the supply voltage. A7=22V, B6=24V, C6=26V
                   
                   
                  
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
3. 010: 30,08,00,2C The light module requests from the UCM the NV 2C.
4. 040: B0,93,E3,00,14,00,2C,66 UCM provides the NV 2C. Repeated after 200ms.
5. 010: 31,08,00,2C The light module stops the abo of NV 2C.
6. The request/response/stopAbo continues for other variables.

## UCM (Joystick Control Module)

CAN_TX: Is the pin 1 of the 74HC02 on the sub-board.


## Diagnostic Connector

according to ref1, this is RS232.

## Open Todos

- [ ] ServoPos shows permanent 0
- [ ] add direction switch and pedal input

## Finished Todos

- [x] Send NV 92 with FF FF in 200ms cycle
- [x] Send the profile (e.g. NV 93 = E1 00)
- [x] Send the parameters of the profile (NVs 2C to 35), after the ServoLight 0x010 requested it, e.g.
    - servoLight requests the 2C from UCM with 00000010,false,Rx,9,4,30,08,00,2C
    - ...
    - servoLight requests the 2E from UCM with 00000010,false,Rx,9,4,30,08,00,2E
    - more general: UCM needs to satisfy all requests (search for 30,08,00)


## Cross References

* [Ref1] System manual for the DXBUS https://www.dynamiccontrols.com/sites/default/files/2018-05/dx-system-manual-issue1.pdf
* [Ref2] European Patent EP0725738A1 (year 1993) https://patents.google.com/patent/EP0725738A1/en
* [Ref3] European Patent EP0725738B1 (year 1993), including schematics, state machines, some network variables. https://patents.google.com/patent/EP0725738B1/en
* [Ref4] SavvyCAN open source CAN logging tool https://www.savvycan.com/
* [Ref5] wifican CAN adaptor via wifi. Adapted to support the 105kBaud of the DXBUS. https://github.com/uhi22/wifican/commit/1e717946a06a594cf2e79b092cbaf7b376e8bb55
* [Ref6] pictures and document collection for the Invacare G50 https://hnng.de/invacare-g50/
