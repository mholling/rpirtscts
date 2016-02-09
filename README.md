# rpirtsrtc

rpirtscts is a simple linux command for enabling the hardware flow control pins for the `ttyAMA0` serial port on the [Raspberry Pi](http://www.raspberrypi.org/).

## Hardware Flow Control & the Raspberry Pi

[Hardware flow control](http://en.wikipedia.org/wiki/RS-232_RTS/CTS#RTS.2FCTS_handshaking) can be used with RS-232 serial ports to co-ordinate data transmission between the _data terminal equipment_ (DTE) and the _data circuit-terminating equipment_ (DCE). In our case, the Raspberry Pi is the DTE, and the serial peripheral the DCE. Flow control is useful when data coming from the DTE has the potential to overflow the buffer in the DCE, or vice-versa.

Two distinct handshake signal, known as RTS (_Request to Send_) and CTS (_Clear To Send_) are used to implement hardware flow control. RTS is asserted by the DTE (the Raspberry Pi) to indicate to the serial peripheral that it is able to receive data. Conversely, CTS is asserted by the peripheral to indicate its readiness to receive data. The peripheral de-asserts CTS to indicate that it has no buffer space left to receive further data. The serial driver on the Raspberry Pi will automatically pause data transmission until the peripheral re-asserts the CTS signal.

On the Raspberry Pi (except the revision 1 model B), an unpopulated header, [P5](http://elinux.org/RPi%5FLow-level%5Fperipherals#P5%5Fheader), brings out RTS and CTS. The signals are implemented as alternate functions of GPIO30 and GPIO31, which can be found on pins 5 and 6, respectively, of the header. (Pay close attention to the location of these pins; the mirrored numbering reflects the header's default position on the _underside_ of the board.) By soldering your own header, you can access RTS and CTS for use with your serial peripheral. Note that they are 3.3V, active-low logic signals.

Newer Raspberry Pi boards have a 40 pin GPIO header replacing the 26 pin and P5 headers.  This requires switching the GPIO which support CTS and RTS to GPIOs 16 (pin 36 on 40 pin header) and 17 (pin 11) respectively.  rpirtscts will attempt to detect the model of the Raspberry Pi and configure the appropriate GPIOs.

This code implements a simple program to enable the alternate functions on GPIO30 and GPIO31 (or GPIO16 and GPIO 17), thereby enabling the RTS and CTS flow control signals to be used along with the data signals TXD and RXD located on the [GPIO header P1](http://elinux.org/RPi_Low-level_peripherals#General_Purpose_Input.2FOutput_.28GPIO.29).

## Usage

Building the program is simple. You will need `gcc`, `make` and `git`. (Depending on the Linux distribution you're using, these packages may or may not come pre-installed.) From the command line, download the code:

    git clone git://github.com/mholling/rpirtscts.git

This will create a directory containing the files. Next, enter the directory and build the executable:

    cd rpirtscts
    make

All going well, an `rpirtscts` executable will be built. The command takes a single option, either `on` or `off`, to respectively enable or disable the flow control pins. Running it requires root permissions, so:

    sudo ./rpirtscts on

Any other invocation produces a short usage description.

It is also possible to alter file permissions so that `sudo` is not necessary:

    sudo chown root rpirtscts
    sudo chmod 4755 rpirtscts

Finally, you will need to instruct the serial port driver to use the hardware flow control signals. This is a simple matter:

    stty -F /dev/ttyAMA0 crtscts

(With `/dev/ttyAMA0` being the character device for the Raspberry Pi serial port.)
