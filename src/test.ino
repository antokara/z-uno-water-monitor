
// Define special command class macroses
// We use SwitchMultilevel and Basic command classes for dimmer, so we have to enable them:
ZUNO_ENABLE(WITH_CC_SWITCH_MULTILEVEL WITH_CC_BASIC)

byte dimmerValue = 99;

// create a structure to store getter & setter handlers
static zuno_handler_single_gettersetter_t switch_multilevel_handlers = {(void *)&getSwitchMultilevelValue, (void *)&setSwitchMultilevelValue};

void setup()
{

    if (zunoStartDeviceConfiguration())
    {

        zunoAddChannel(ZUNO_SWITCH_MULTILEVEL_CHANNEL_NUMBER, 0, 0);
        zunoSetZWChannel(0, 0);
        zunoCommitCfg();
    }
    zunoAppendChannelHandler(0, 1, CHANNEL_HANDLER_SINGLE_GETTERSETTER, (void *)&switch_multilevel_handlers);
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