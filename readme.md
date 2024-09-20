
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

## Diagnostic Connector

according to ref1, this is RS232.

## Cross References

[1] https://www.dynamiccontrols.com/sites/default/files/2018-05/dx-system-manual-issue1.pdf