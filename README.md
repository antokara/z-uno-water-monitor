# z-uno water monitor

A z-wave monitor for water flow and pressure for leaks, with an optional water shutoff valve in case of emergencies. Powered by the [z-uno board](https://z-uno.z-wave.me/).

## vs code installation

1. [follow vs-code install](https://z-uno.z-wave.me/vs-code-install/)
1. restart VS code between steps if you get errors of command not found
1. make sure you set the frequency/security/etc. settings, otherwise, it will not get included on the network

## inclusion/exclusion

for either, press 3 times rapidly the _service_ `BTN`.
it should turn the led red.

## troubleshooting

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
