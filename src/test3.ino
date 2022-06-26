ZUNO_ENABLE(WITH_CC_SENSOR_MULTILEVEL WITH_CC_BASIC)

zuno_handler_multi_getter_t multi_sen_getter = {(void *)&getterFunctionSensorBinary, 0}; // 0 is the index of first channel

void setup()
{

    if (zunoStartDeviceConfiguration())
    {

        zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER, ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_ONE_BYTE, SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS));
        zunoSetZWChannel(0, 0);
        zunoCommitCfg();
    }
    zunoAppendChannelHandler(0, 1, CHANNEL_HANDLER_MULTI_GETTER, (void *)&multi_sen_getter);
}

void loop()
{
}

byte getterFunctionSensorBinary()
{
    // 10 celcius = 50 Fahrenheit
    return 10;
}