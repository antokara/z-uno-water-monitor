#include "status.h"
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
    // channel 3 - water meter in gallons
    ZUNO_METER(
        ZUNO_METER_TYPE_WATER,
        METER_RESET_ENABLE,
        ZUNO_METER_WATER_SCALE_GALLONS,
        METER_SIZE_FOUR_BYTES,
        METER_PRECISION_ZERO_DECIMALS,
        &getGallons,
        &resetGallons),
    // channel 4 - status
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_GENERAL_PURPOSE_VALUE,
                           SENSOR_MULTILEVEL_SCALE_DIMENSIONLESS_VALUE,
                           SENSOR_MULTILEVEL_SIZE_ONE_BYTE,
                           SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS,
                           &getStatus));

// enable S2 authenticated mode
ZUNO_SETUP_S2ACCESS(SKETCH_FLAG_S2_AUTHENTICATED_BIT);

void setup()
{
    pulseSensorSetup();
    pressureSensorSetup();

    // for debug
    // Serial.begin(9600);
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
 * @brief
 * @return
 */
dword getGallons()
{
    return gallons;
}

/**
 * @brief resets the gallons count
 *
 */
void resetGallons()
{
    gallons = 0;
}

/**
 * @brief
 * @return
 */
byte getStatus()
{
    return status;
}