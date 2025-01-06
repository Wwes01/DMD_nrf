# Transmitter
Can be found in src folder. 

src/common.h contains a couple defines that are important, most important:
- US_BETWEEN_BITS: defines the symbol period in microseconds.
- USER_INTERFACE: if defined, the Serial Terminal can be used from "nRF Connect for Desktop" to input new messages for the transmitter to send. To enable or disable changes also need to be made in the .overlay file, and the prj.conf file. The lines after "User Interface:" can simply be commented or uncommented based on if the user interface is or is not required. If not defined, the button can still be used to send the predefined message for the predefined number of times.

src/buffer.c defines the signals to control the DMD.

# Receiver
Can be found in fast_receiver folder.

Important defines:
- PERIOD: the duration of the symbol period.
- DEBUG: if defined, prints after each message. If it is not defined, it waits until it received PAYLOAD_COUNTS messages to print, or until it receives "end". "end" can be sent via the serial terminal. 

## Libraries
- libcorrect: https://github.com/quiet/libcorrect
- DueAdcFast: https://github.com/AntonioPrevitali/DueAdcFast