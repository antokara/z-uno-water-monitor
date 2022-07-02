// Normal Flow Range: 0.25 - 15 GPM
// Pulse Rate is 1 Pulse/Gallon
ZUNO_SETUP_CHANNELS(
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_GENERAL_PURPOSE_VALUE,
                           SENSOR_MULTILEVEL_SCALE_DIMENSIONLESS_VALUE,
                           SENSOR_MULTILEVEL_SIZE_FOUR_BYTES,
                           SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL,
                           &getterFunction),
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_GENERAL_PURPOSE_VALUE,
                           SENSOR_MULTILEVEL_SCALE_DIMENSIONLESS_VALUE,
                           SENSOR_MULTILEVEL_SIZE_TWO_BYTES,
                           SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL,
                           &getterFunction2),
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_GENERAL_PURPOSE_VALUE,
                           SENSOR_MULTILEVEL_SCALE_DIMENSIONLESS_VALUE,
                           SENSOR_MULTILEVEL_SIZE_ONE_BYTE,
                           SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL,
                           &getterFunction3));

int lastPulse;
word gpm;

void setup()
{
}

void loop()
{
    zunoSendReport(1);
    delay(10000);
}

/**
 * @brief
 *
 * getter function works for floats whereas variables, so far, only work for integer values.
 *
 * @return float
 */
word getterFunction()
{
    // 1.0 = 106535321.6
    // 2.0 = 107374182.4    838860.8
    // 3.0 = 107793612.8    419430.4
    return 3.0;
}

word getterFunction2()
{
    // 1.0 = 106535321.6
    // 2.0 = 107374182.4    838860.8
    // 3.0 = 107793612.8    419430.4
    return 3.0;
}

word getterFunction3()
{
    // 1.0 = 106535321.6
    // 2.0 = 107374182.4    838860.8
    // 3.0 = 107793612.8    419430.4
    return 3.0;
}
