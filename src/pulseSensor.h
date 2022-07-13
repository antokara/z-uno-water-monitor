// the (digital) pin that we need to connect the water meter pulse switch.
// the other end, needs to go the ground (GND) pin, of zuno.
#define PULSE_SENSOR_PIN 12

// minimum gallons per minute that the water meter can detect.
// this helps us detect no-flow, by calculating a "time-out" when
// too much time has passed since a new pulse.
#define MIN_GPM 0.1

// time in milliseconds for our target rate
// 60000msecs = 60secs = 1minute rate for GPM
#define TARGET_RATE_TIME 60000.0

// frequency to update the flow, in milliseconds (between pulse and timeout only)
#define UPDATE_FLOW_FREQUENCY 2500

// frequency to send the flow, in milliseconds (between pulse and timeout only)
#define SEND_FLOW_FREQUENCY 5000

// number of pulses per gallon (Pulse/Gallon)
#define PULSE_RATE 1.0

// holds the last pulse sensor isActive
boolean lastPulseSensorIsActive = false;

// formula for getting GPM, using pulse rate and duration between pulses
// [target rate time] / [duration between pulses] / [pulse rate] = Gallons Per Rate
// examples:
// target rate: 60 seconds to get (GPM)
// pulse rate: 1
// 60secs / 60secs / 1 pulse per gallon = 1 GPM
// 60 / 120 / 1 = 0.5
// pulse rate: 2
// 60 / 60 / 2 = 0.5
// 60 / 30 / 2 = 1

// formula for getting duration between pulses, using target rate time, Gallons Per Rate and pulse rate
// [target rate time] / [Gallons Per Rate] / [pulse rate] = [duration between pulses]
// Normal Flow Range: 0.25 - 15 GPM
// Pulse Rate is 1 Pulse/Gallon

// last time we had a pulse
unsigned long lastPulseTime = 0;

// current gallons per minute
float gpm = 0.0;

// time that must pass without a pulse, in order to be considered no-flow
unsigned int flowTimeout = 0;

// last time we updated the flow (between pulse and timeout)
unsigned long lastFlowUpdateTime = 0;

// last time we sent the flow
unsigned long lastFlowSendTime = 0;

// function overload signatures
void updateGPM();
void updateGPM(float newValue);

void pulseSensorSetup()
{
    // calculate how much time must pass without a pulse, in order to consider no-flow
    flowTimeout = TARGET_RATE_TIME / MIN_GPM / PULSE_RATE;

    // set the mode for the digital pins
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PULSE_SENSOR_PIN, INPUT_PULLUP);
}

void pulseSensorLoop()
{
    if (isPulseSensorActive())
    {
        // we got a pulse (this can only happen once, per pulse,
        // even if the meter stops right when the switch is on and the switch remains on)
        updateGPM();
        sendGPM(true);

        // reset the timer, after we have used it (with timePassedSinceLastPulse)
        lastPulseTime = millis();

        digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (gpm > 0.0)
    {
        unsigned long timePassed = timePassedSinceLastPulse();
        if (timePassed > flowTimeout)
        {
            // flow timed-out. consider it now that there's no flow
            updateGPM(0.0);
            digitalWrite(LED_BUILTIN, LOW);
            sendGPM(true);
        }
        else if (millis() - lastFlowUpdateTime >= UPDATE_FLOW_FREQUENCY)
        {
            // enough time has passed to update the flow
            updateGPM();
            sendGPM(false);
        }
    }
}

void updateGPM()
{
    gpm = TARGET_RATE_TIME / timePassedSinceLastPulse() / PULSE_RATE;
    lastFlowUpdateTime = millis();
}

void updateGPM(float newValue)
{
    gpm = newValue;
    lastFlowUpdateTime = millis();
}

/**
 * @brief sends the zwave data for all the channels
 *
 * According to Z-Wave Plus restrictions, values from Sensor Multilevel channels
 * (defined via ZUNO_SENSOR_MULTILEVEL macro) will not be sent unsolicitedly
 * to Life Line more often than every 30 seconds.
 *
 * @param force optional. defaults to false. if true, it will attempt to send,
 *              no matter how much time has passed since the last send.
 */
void sendGPM(boolean force)
{
    if (force || millis() - lastFlowSendTime >= SEND_FLOW_FREQUENCY)
    {
        lastFlowSendTime = millis();
        zunoSendReport(1);
    }
}

/**
 * @brief
 *
 * @return milliseconds since the last pulse
 */
unsigned long timePassedSinceLastPulse()
{
    return millis() - lastPulseTime;
}

/**
 * @brief
 * @return
 */
word getGPM()
{
    if (gpm > 0.0)
    {
        // for SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL we need * 10
        // for SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS we need * 100
        return gpm * 100.0;
    }
    return 0.0;
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
        // TODO: add 2nd debouncer based on the max speed of the sensor
        if (!lastPulseSensorIsActive)
        {
            // and it just turned active
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