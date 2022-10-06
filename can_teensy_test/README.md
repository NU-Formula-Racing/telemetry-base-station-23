Simple local program to test the PlatformIO (PIO) DE as well as compiling for the Teensy using NFR's custom CAN library.

PIO automates dependency management, this is managed via the platformio.ini file. This does not absolve dependency inclusion in programs to use their contents, with some dependencies requiring inclusion to build properly.

Running your program on the Teensy takes the following simple steps:
1) Connect the microUSB on the Teensy to your computer. NFR's microUSB cables do not have Thunderbolt terminals, so an additional converter will be necessary if, say, you're working on a Mac.
2) Open the PIO menu, then open teensy40/General. 
3) Open the Teensy loader program by clicking Upload.
4) On your connected Teensy, press the small white button on the opposite end of the microUSB. This will automatically flash your program—PIO and Teensy handle dirty work like searching for ports and rebooting for you.
5) Check your program status/Serial output by clicking Monitor.