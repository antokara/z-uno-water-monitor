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
#define PRESSURE_SENSOR_PSI_CALIBRATION_MULTIPLIER 1.2

// the min/max Voltage the sensor itself outputs, depending on the pressure
#define MIN_PRESSURE_SENSOR_VOLTAGE 0.333
#define MAX_PRESSURE_SENSOR_VOLTAGE 3

// the number of which we need to multiply voltage by, in order to get the expected input pin value
const float pressureSensorInputValueMultiplier = float(MAX_PRESSURE_SENSOR_PIN_RANGE / MAX_PRESSURE_SENSOR_PIN_RANGE_VOLTAGE);

// the adjusted/actual minimum input value the pressure sensor pin can provide,
// to match the minimum pressure sensor PSI
// ie. this should be around 65, which means around 0 PSI (with some fault tolerance)
const int adjustedMinPressureSensorInputValue = pressureSensorInputValueMultiplier * MIN_PRESSURE_SENSOR_VOLTAGE;
const int adjustedMaxPressureSensorInputValue = pressureSensorInputValueMultiplier * MAX_PRESSURE_SENSOR_VOLTAGE;

// the adjusted/actual number we need to multiply the input value - adjustedMinPressureSensorInputValue,
// in order to get the true PSI of the sensor
const float adjustedPressureSensorInputValueMultiplier = MAX_PRESSURE_SENSOR_PSI / adjustedMaxPressureSensorInputValue * PRESSURE_SENSOR_PSI_CALIBRATION_MULTIPLIER;

// current PSI
int psi = 0;

// previous PSI (so we only send changes)
int prevPsi = 0;

void pressureSensorSetup()
{
    // set the analog pin resolution to the default
    // @see https://z-uno.z-wave.me/Reference/analogReadResolution/
    // which should give us a range of 0–1023
    analogReadResolution(10);
}

void pressureSensorLoop()
{
    // TODO: add check if millis resets (if new value is < than old millis value, then, we need to reset lastPulseTime?)
    // TODO: check when starting from zero lastPulseTime
    // TODO: check how to add solicited request from HA

    int rawPressureSensorInputValue = analogRead(PRESSURE_SENSOR_PIN); // read the input pin
    psi = (rawPressureSensorInputValue - adjustedMinPressureSensorInputValue) * adjustedPressureSensorInputValueMultiplier;

    // delay(1000);

    // Serial.print("rawPressureSensorInputValue: ");
    // Serial.println(rawPressureSensorInputValue);

    // Serial.print("psi: ");
    // Serial.println(psi);

    if (psi != prevPsi)
    {
        prevPsi = psi;
        sendPSI();
        //     Serial.print("psi: ");
        //     Serial.println(psi);
    }
}

void sendPSI()
{
    zunoSendReport(2);
}

/**
 * @brief
 * @return
 */
int getPSI()
{
    return psi;
}
