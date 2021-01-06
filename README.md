# Arduino IDE profile

Board: Generic ESP8266 Module

Upload Speed: 115200

CPU Frequency: 160MHz

Crystal Frequency: 26MHz

Flash Size: 2M (1M FS)

Flash Mode: QIO

Flash Frequency: 40MHz

Reset Method: no dtr, no_sync

Debug Port: Disabled

Debug Level: None

IwIP Variant: v2 Lower Memory

VTables: Flash

Bulitin LED: 1

Erase Flash: All Flash Contents

# Preparations

Solder following pins:

​	pin1 and pin2(as pin2 is EN and needs to be pulled up)

​	pin6 and pin9(as pin6 is GPIO15 and needs to be pulled down)

​	pin8 and pin9(as pin8 is GPIO0 and needs to be pulled down in order to use UART during firmware uploading)