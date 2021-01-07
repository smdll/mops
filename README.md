# Pinout

| GPIO/Arduino pinout | Component |
| ------------------- | --------- |
| 12                  | LED       |
| 13                  | Button    |
| 15                  | Relay     |

# Arduino IDE profile

Board: Generic ESP8266 Module

Upload Speed: 115200

CPU Frequency: 160MHz

Crystal Frequency: 26MHz

Flash Size: 4M (3M FS)

Flash Mode: QIO

Flash Frequency: 40MHz

Reset Method: no dtr, no_sync

Debug Port: Disabled

Debug Level: None

IwIP Variant: v2 Lower Memory

VTables: Flash

Bulitin LED: 1

Erase Flash: Only Sketch

# Preparations

Solder pin1 and pin2(as pin2 is EN and needs to be pulled up), connect pin8 and pin9(as pin8 is GPIO0 and needs to be pulled down in order to use UART during firmware uploading)

# Gateway