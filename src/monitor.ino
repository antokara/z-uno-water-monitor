#define PULSE_SENSOR_PIN 12

// Normal Flow Range: 0.25 - 15 GPM
// Pulse Rate is 1 Pulse/Gallon
ZUNO_SETUP_CHANNELS(
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_GENERAL_PURPOSE_VALUE,
                           SENSOR_MULTILEVEL_SCALE_DIMENSIONLESS_VALUE,
                           SENSOR_MULTILEVEL_SIZE_TWO_BYTES,
                           SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS,
                           &getterFunction));
int lastPulse;
int pulseSensorValue = LOW;
float gpm = 0.0;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PULSE_SENSOR_PIN, INPUT_PULLUP);
    Serial.begin(9600);
}

void loop()
{
    pulseSensorValue = digitalRead(PULSE_SENSOR_PIN);
    if (pulseSensorValue == LOW)
    {
        gpm = 15.0;
        digitalWrite(LED_BUILTIN, HIGH);
    }
    else
    {
        gpm = 0.25;
        digitalWrite(LED_BUILTIN, LOW);
    }
    Serial.print("gpm: ");
    Serial.println(gpm);

    // According to Z-Wave Plus restrictions, values from Sensor Multilevel channels
    // (defined via ZUNO_SENSOR_MULTILEVEL macro) will not be sent unsolicitedly
    // to Life Line more often than every 30 seconds.
    zunoSendReport(1);
    delay(1000);
}

/**
 * @brief
 * @return
 */
word getterFunction()
{
    if (gpm > 0.0)
    {
        // for SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL we need * 10
        // for SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS we need * 100
        return gpm * 100.0;
    }
    return 0.0;
}
