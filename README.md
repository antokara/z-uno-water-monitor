# z-uno water monitor

A z-wave monitor for water flow and pressure for leaks, with an optional water shutoff valve in case of emergencies. Powered by the [z-uno board](https://z-uno.z-wave.me/).

## vs code installation

1. [follow vs-code install](https://z-uno.z-wave.me/vs-code-install/)
1. restart VS code between steps if you get errors of command not found
1. make sure you set the frequency/security/etc. settings, otherwise, it will not get included on the network
    1. for security, enable S2

## SmartStart QR

Please note that this method did not work for me. HA kept it in provisioned mode but never included it, even after resetting the device, etc.

Creating the QR Code:

`Z-Uno2/tools/zme_make/zme_make boardInfo -q qrcode.png -d /dev/ttyUSB0`

you can also generate the QR manually by getting its value following this:

1. open commands
1. select `Z-UNO Extracts metadata from connected Z-Uno board`
1. click `Get Info`
1. you should see in the console output now lots of information
    1. including `SmartStart QR`

## inclusion/exclusion

for either, press 3 times rapidly the _service_ `BTN`.
it should turn the led red.

* The inclusion and interview may take longer, when S2 with PIN is enabled.
* Inclusion time is also affected by number of channels and their size (ie. four byte sensor/meters can take twice as much to get included)

## troubleshooting

### S2 PIN

the PIN for inclusion (also known as DSK or ZWS2DSK) can be found in VS Code by:

1. open commands
1. select `Z-UNO Extracts metadata from connected Z-Uno board`
1. click `Get Info`
1. you should see in the console output now lots of information
    1. including `SECURITY S2 PUK`, which the first 5 digits are the pin

### Permission denied on port

when you get

```bash
Error  0 Could not open serial port /dev/ttyUSB0: [Errno 13] could not open port /dev/ttyUSB0: [Errno 13] Permission denied: '/dev/ttyUSB0'
```

run: `$sudo chmod a+rw /dev/ttyUSB0`

### Can't sync with Z-Uno bootloader

when you get

```bash
Syncing with Z-Uno ('RST' btn?) [ * ]    ..............................
Syncing with Z-Uno ('RST' btn?) [  *]    ..............................
Syncing with Z-Uno                       ..............................                        FAILED

Error  0 Can't sync with Z-Uno bootloader. It doesn't respond!

Closing port                             ..............................
Closing port                             ..............................                            OK

Error  1 Can't request board information! Please try to update bootloader first.
```

make sure all Serial port monitors are closed (check on the bottom bar)

### stuck at "The device is being interviewed. This may take some time."

the z-wave configuration in the code is probably incorrect or incompatible with the z-wave on HA

### the pressure sensor reads wrong PSI

The pressure sensor depends on the 5v power supply pin and has been calibrated according to that.

When the power supply changes, ie. instead of 5.1v, it drops to 4.7v, the calibration multiplier
will have to change accordingly. Otherwise, the readings will be wrong by the same amount.

examples:

* with the USB of a computer, the `5v` voltage, reports `4.7v` and with that voltage, the `PRESSURE_SENSOR_PSI_CALIBRATION_MULTIPLIER` is `1.2`
* with a dedicated USB power supply, the `5v` voltage, reports `5.1v` and with that voltage, the `PRESSURE_SENSOR_PSI_CALIBRATION_MULTIPLIER` is `1.1`

## controller can't include/exclude the device

if a failed inclusion/exclusion or provisioning took place, you might have to forcefully
exclude/delete the device from your controller and/or [reset](https://z-uno.z-wave.me/Reference/Z-Wave/#Device%20reset) the device by:

1. hold the Service button until the service red LED is blinking
1. release the Service button and then triple click on it
1. The red LED will confirm the reset process

you should be able to include/exclude again.
