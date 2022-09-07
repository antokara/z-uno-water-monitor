/**
 * @brief frequency in milliseconds, to allow sending of zwave data to the controller.
 * this is a big number because we rely on polling from the controller,
 * instead of unsolicited lifeline updates from the node, for the pressure.
 *
 * 3600000 = 1 hour
 * 10800000 = 3 hours
 */
#define SEND_PRESSURE_FREQUENCY 10800000

// the (analog) pin that we connect the pressure sensor output
// you may use 3-6, which maps to A0-A3
#define PRESSURE_SENSOR_PIN 3

// the min/max inputs the pressure sensor pin can give us
// @see https://z-uno.z-wave.me/Reference/analogReadResolution/
#define MAX_PRESSURE_SENSOR_PIN_RANGE 1023

// the actual max voltage the pin would need to reach that range
// meaning, even though the pin can only accept a max of 3volts and
// even though, the max range of the pin is 1023, it would theoritically
// reach that 1023 if we supplied it with 5volts,
// which would can't without damaging the board. Z-uno's documentation
// should have mentioned that but they do not state anything about this.
#define MAX_PRESSURE_SENSOR_PIN_RANGE_VOLTAGE 5

// the max PSI the sensor itself can measure
#define MAX_PRESSURE_SENSOR_PSI 100.0

// how much to multiply the PSI number we come up with
// to match the calibrated and expected reading from other sensors
//
// when connected to the USB of a computer, the 5v voltage, reports 4.7v and
// with that voltage, the calibration multiplier is 1.2
//
// when connected to the USB of a dedicated USB power supply, the 5v voltage, reports 5.1v and
// with that voltage, the calibration multiplier is 1.1
//
// the reason for that is 4.7/5.1=0.92*1.2=1.10
// changed to directly the voltage multiplier
#define PRESSURE_SENSOR_PSI_CALIBRATION_MULTIPLIER 0.92

// the min/max Voltage the sensor itself outputs, depending on the pressure
#define MIN_PRESSURE_SENSOR_VOLTAGE 0.333
#define MAX_PRESSURE_SENSOR_VOLTAGE 3

// then 1-based, number of z-wave channel, as defined in ZUNO_SETUP_CHANNELS
#define PRESSURE_ZWAVE_CHANNEL 2

// the number of which we need to multiply voltage by, in order to get the expected input pin value
const float pressureSensorInputValueMultiplier = float(MAX_PRESSURE_SENSOR_PIN_RANGE / MAX_PRESSURE_SENSOR_PIN_RANGE_VOLTAGE);

// the adjusted/actual minimum input value the pressure sensor pin can provide,
// to match the minimum pressure sensor PSI
// ie. this should be around 65, which means around 0 PSI (with some fault tolerance)
const float adjustedMinPressureSensorInputValue = pressureSensorInputValueMultiplier * MIN_PRESSURE_SENSOR_VOLTAGE;
const float adjustedMaxPressureSensorInputValue = pressureSensorInputValueMultiplier * MAX_PRESSURE_SENSOR_VOLTAGE;

// the adjusted/actual number we need to multiply the input value - adjustedMinPressureSensorInputValue,
// in order to get the true PSI of the sensor
const float adjustedPressureSensorInputValueMultiplier = MAX_PRESSURE_SENSOR_PSI / adjustedMaxPressureSensorInputValue * PRESSURE_SENSOR_PSI_CALIBRATION_MULTIPLIER;

// current PSI
float psi = 0.0;

// previous PSI (so we only send changes)
float prevPsi = 0.0;

// last time we sent the pressure
unsigned long lastPressureSendTime = 0;

void pressureSensorSetup()
{
    // set the analog pin resolution to the default
    // @see https://z-uno.z-wave.me/Reference/analogReadResolution/
    // which should give us a range of 0–1023
    analogReadResolution(10);
}

/**
 * @brief if we should send the pressure to the controller.
 * a separate throttler to keep the unsolicited updated in lifeline prioritize the flow.
 *
 * @return true
 * @return false
 */
bool shouldSendPSI()
{
    return abs(millis() - lastPressureSendTime) > SEND_PRESSURE_FREQUENCY && shouldSendData();
}

void pressureSensorLoop()
{
    int rawPressureSensorInputValue = analogRead(PRESSURE_SENSOR_PIN); // read the input pin
    psi = (rawPressureSensorInputValue - adjustedMinPressureSensorInputValue) * adjustedPressureSensorInputValueMultiplier;
    if (psi != prevPsi && shouldSendPSI())
    {
        prevPsi = psi;
        lastPressureSendTime = millis();
        sendData(PRESSURE_ZWAVE_CHANNEL);
    }
}