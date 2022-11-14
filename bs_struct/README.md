This program tests the ability of Teensy to pack data into a struct. This is of interest to significantly streamline
data serialization, add some support for dynamicism in data transfer, and better respond to subteam needs.

The design of the program is the same, with the exceptio that all data transfer is conducted through structs.
Any client program that can read bytes from a USB port and knows the structure of the incoming data
can receive the data and deserialize them accordingly. One example program is located in `usb_struct`, written in Rust.

Currently, only the RX has been modified and tested due to complications with TX hardware. Thus, the RX
data has been preset to be garbage, but easily readable, values.
Do NOT flash the RX with the TX program, as it is very much outdated!