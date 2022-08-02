// #include "status.h"
#include "pressureSensor.h"
#include "pulseSensor.h"

// setup the z-wave channels
ZUNO_SETUP_CHANNELS(
    // channel 1 - water flow
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_GENERAL_PURPOSE_VALUE,
                           SENSOR_MULTILEVEL_SCALE_DIMENSIONLESS_VALUE,
                           SENSOR_MULTILEVEL_SIZE_TWO_BYTES,
                           SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS,
                           &getGPM),
    // channel 2 - water pressure
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_GENERAL_PURPOSE_VALUE,
                           SENSOR_MULTILEVEL_SCALE_DIMENSIONLESS_VALUE,
                           SENSOR_MULTILEVEL_SIZE_TWO_BYTES,
                           SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS,
                           &getPSI),
    // channel 3 - water meter pulse (per gallon)
    ZUNO_SENSOR_BINARY(ZUNO_SENSOR_BINARY_TYPE_WATER, &getWaterMeterPulse));

// enable S2 authenticated mode
ZUNO_SETUP_S2ACCESS(SKETCH_FLAG_S2_AUTHENTICATED_BIT);

// make sure we only setup once
bool firstRun = true;

void setup()
{
    if (firstRun)
    {
        pulseSensorSetup();
        pressureSensorSetup();
        firstRun = false;
    }

    // for debug
    // Serial.begin(115200);
}

void loop()
{
    pulseSensorLoop();
    pressureSensorLoop();
}

/**
 * @brief
 * @return
 */
word getGPM()
{
    // for SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL we need * 10
    // for SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS we need * 100
    return gpm * 100.0;
}

/**
 * @brief
 * @return
 */
word getPSI()
{
    return psi * 100.0;
}

/**
 * @brief Get the Water Meter Pulse
 *
 * @return true
 * @return false
 */
bool getWaterMeterPulse()
{
    return lastPulseSensorIsActive;
}
