
# Invacare G50 Investigations

Investigating an electric wheel chair

![image](doc/foto1.jpg)

## Pinout of the Power Module

todo

The "Power Module" is a DX-PMB2-SAS, producer "Dynamic Controls", this seems to be the same as the DX-PMB2 mentioned in Ref1.

## Bus Communication

The "DXBUS" consists of four pins:
* Ground
* 24V
* CANH
* CANL

The ground and 24V are directly connected to the battery pins of the power module.

CAN bit time is 9.5µs, this means the CAN baud rate is 105.29 kBaud.

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

## UCM (Joystick Control Module)

CAN_TX: Is the pin 1 of the 74HC02 on the sub-board.


## Diagnostic Connector

according to ref1, this is RS232.

## Cross References

* [Ref1] https://www.dynamiccontrols.com/sites/default/files/2018-05/dx-system-manual-issue1.pdf
* [Ref2] European Patent EP0725738A1 (year 1993) https://patents.google.com/patent/EP0725738A1/en
* [Ref3] European Patent EP0725738B1 (year 1993), including schematics, state machines, some network variables. https://patents.google.com/patent/EP0725738B1/en