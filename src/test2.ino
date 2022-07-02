
// Define special command class macroses
// We use SwitchMultilevel and Basic command classes for dimmer, so we have to enable them:
ZUNO_ENABLE(WITH_CC_SWITCH_MULTILEVEL WITH_CC_SENSOR_MULTILEVEL WITH_CC_BASIC)

byte dimmerValue = 99;

// create a structure to store getter & setter handlers
static zuno_handler_single_gettersetter_t switch_multilevel_handlers = {(void *)&getSwitchMultilevelValue, (void *)&setSwitchMultilevelValue};
zuno_handler_multi_getter_t multi_sen_getter = {(void *)&getterFunctionSensorBinary, 0}; // 0 is the index of first channel

void setup()
{

    if (zunoStartDeviceConfiguration())
    {

        zunoAddChannel(ZUNO_SWITCH_MULTILEVEL_CHANNEL_NUMBER, 0, 0);
        zunoSetZWChannel(0, 1 | ZWAVE_CHANNEL_MAPPED_BIT);

        zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER, ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_ONE_BYTE, SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS));
        zunoSetZWChannel(1, 2);

        zunoCommitCfg();
    }
    zunoAppendChannelHandler(0, 1, CHANNEL_HANDLER_SINGLE_GETTERSETTER, (void *)&switch_multilevel_handlers);
    zunoAppendChannelHandler(1, 1, CHANNEL_HANDLER_MULTI_GETTER, (void *)&multi_sen_getter);
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(dimmerValue * 10);
    digitalWrite(LED_BUILTIN, LOW);
    delay(dimmerValue * 10);
}

static void setSwitchMultilevelValue(byte newValue)
{
    dimmerValue = newValue;
}

static byte getSwitchMultilevelValue(void)
{
    return dimmerValue;
}

BYTE getterFunctionSensorBinary()
{
    return (0x0);
}