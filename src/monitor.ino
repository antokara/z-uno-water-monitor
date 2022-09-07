/**
 * According to Z-Wave Plus restrictions, values from Sensor Multilevel channels
 * (defined via ZUNO_SENSOR_MULTILEVEL macro) will not be sent unsolicitedly
 * to Life Line more often than every 30 seconds.
 */
#define SEND_DATA_FREQUENCY 30000

// true, if we sent any data using zunoSendReport in the current cycle.
// this is because of a z-wave/z-uno limitation, that prevents sending more than one per cycle
// @see https://z-uno.z-wave.me/Reference/zunoSendReport/
bool sentData = false;

#include "pressureSensor.h"
#include "pulseSensor.h"

// setup the z-wave channels
ZUNO_SETUP_CHANNELS(
    // channel 1 - water flow (in GPM)
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
    // channel 3 - gallons counter (to increase the water meter/counter)
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_GENERAL_PURPOSE_VALUE,
                           SENSOR_MULTILEVEL_SCALE_DIMENSIONLESS_VALUE,
                           SENSOR_MULTILEVEL_SIZE_TWO_BYTES,
                           SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS,
                           &getGallonsCounter));

// enable S2 authenticated mode
ZUNO_SETUP_S2ACCESS(SKETCH_FLAG_S2_AUTHENTICATED_BIT);

void setup()
{
    pulseSensorSetup();
    pressureSensorSetup();

    // for debug
    // Serial.begin(115200);
}

void loop()
{
    // reset on every cycle
    sentData = false;
    // run the cycles of each file
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
 * @brief the number of gallons to increase the water meter counter by.
 *        the goal is to only return a positive number, when at least one gallon has been counted.
 *        if more than 1 GPM have been counted, we will instead send that total number.
 * @return
 */
word getGallonsCounter()
{
    return gallonsCounter * 100.0;
}