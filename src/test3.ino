ZUNO_ENABLE(WITH_CC_SENSOR_MULTILEVEL WITH_CC_BASIC)

zuno_handler_multi_getter_t multi_sen_getter = {(void *)&getterFunctionSensorBinary, 0}; // 0 is the index of first channel

#define ZUNO_SENSOR_MULTILEVEL_TYPE_WATER_FLOW 0x38
#define SENSOR_MULTILEVEL_SCALE_LITERS_PER_HOUR 0x00

void setup()
{

    if (zunoStartDeviceConfiguration())
    {

        zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER, ZUNO_SENSOR_MULTILEVEL_TYPE_WATER_FLOW, SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_LITERS_PER_HOUR, SENSOR_MULTILEVEL_SIZE_FOUR_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS));
        zunoSetZWChannel(0, 0);
        zunoCommitCfg();
    }
    zunoAppendChannelHandler(0, 1, CHANNEL_HANDLER_SINGLE_GETTER, (void *)&multi_sen_getter);
}

void loop()
{
}

byte getterFunctionSensorBinary()
{
    // 10 celcius = 50 Fahrenheit
    return 10;
}