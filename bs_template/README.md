This directory contains template files for a basic TX/RX comm program over CAN-Teensy-LoRa
to be added to any new project in the `telemetry-base-station-23/` repo.

To add these files to a new directory, run `populate.sh` with your destination
directory as the 1st (and only) argument.

Ex. `bash populate.sh <proj>`

General descriptions of the contents of this template are given below:

`telemetry.h` condenses external includes, hardware pin and freq configurations,
global vars, and task functions into a unified template file.
Its implementation is found in `telemetry.cpp`.

`target.h` sets the compilation macro to determine for which device to compile for.
It includes 2 macros; uncomment exactly **one** to specify device, as some libraries
will not work without the matching hardware.

`ser_des.h` contains serializer and deserializer functions to prepare data
for LoRa data packets. Its implementation can be found in 'ser_des.cpp'.

'main.cpp' contains a template program to set up virtual timers for the
setup and task functions.

'platformio.ini' is a file that sets dependencies for linking and compilation
used by PlatformIO (PIO). Since Base Station primarily compiles for the Teensy,
only its platform dependencies are used; additional libraries must be added/changed
in this file.

This file will not be copied using this script, as each subsequent project should
be documented uniquely and accordingly to its purpose. Refer to `lora_can_2/` for
information about the original project these files come from, though some of these
files have been modified based on results from previous projects.

Update history:

- 06/01/23: Commented out excess Serial prints on TX, macro-abstracted CAN bus number
  based on modifications from contemporary project bs_1/.