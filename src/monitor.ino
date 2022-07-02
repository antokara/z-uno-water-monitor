#define PULSE_SENSOR_PIN 12

// holds the last pulse sensor isActive
boolean lastPulseSensorIsActive = false;

// Normal Flow Range: 0.25 - 15 GPM
// Pulse Rate is 1 Pulse/Gallon
ZUNO_SETUP_CHANNELS(
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_GENERAL_PURPOSE_VALUE,
                           SENSOR_MULTILEVEL_SCALE_DIMENSIONLESS_VALUE,
                           SENSOR_MULTILEVEL_SIZE_TWO_BYTES,
                           SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS,
                           &getterFunction));
unsigned long lastPulseTime = 0;

float gpm = 0.0;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PULSE_SENSOR_PIN, INPUT_PULLUP);
    Serial.begin(9600);
}

void loop()
{
    if (isPulseSensorActive())
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
 * handles debouncing on its own, so that it returns true only once.
 * even if it stays true and we call it multiple times, it will be true only once.
 *
 * @return true if the pulse sensor is active (meaning, a Gallon was just metered)
 * @return false when there is no pulse
 */
bool isPulseSensorActive()
{
    if (digitalRead(PULSE_SENSOR_PIN) == LOW)
    {
        // when the sensor is in active state
        if (!lastPulseSensorIsActive)
        {
            // it just turned active
            lastPulseSensorIsActive = true;
            // only the first time, return true
            return lastPulseSensorIsActive;
        }
    }
    else if (lastPulseSensorIsActive)
    {
        // it just turned inactive
        lastPulseSensorIsActive = false;
    }

    // any other time, return inactive
    return false;
}

/**
 * @brief
 *
 * @return unsigned long milliseconds since the last pulse
 */
unsigned long timePassedSinceLastPulse()
{
    return millis() - lastPulseTime;
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
