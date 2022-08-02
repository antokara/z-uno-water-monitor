// the (digital) pin that we need to connect the water meter pulse switch.
// the other end, needs to go the ground (GND) pin, of zuno.
#define PULSE_SENSOR_PIN 12

// the (analog) pin that we connect to the
// InfraRed AO (analog output) pin of the sensor
// you may use 3-6, which maps to A0-A3
#define IR_SENSOR_PIN 4

// then 1-based, number of z-wave channels, as defined in ZUNO_SETUP_CHANNELS
#define FLOW_ZWAVE_CHANNEL 1
#define METER_ZWAVE_CHANNEL 3

// the delta we must calculate between two infrared sensor values
// in order to be considered an actual change/motion
#define IR_DELTA_THRESHOLD 3

// time in milliseconds that a delta lasts
#define IR_TIMEOUT 500

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

// time passed between previous pulse and the current one
// TODO: rename last/prev/current to clear things up
unsigned int prevTimePassedSinceLastPulse = 0;

// last time we updated the flow (between pulse and timeout)
unsigned long lastFlowUpdateTime = 0;

// last time we sent the flow
unsigned long lastFlowSendTime = 0;

// number of gallons the water meter has counted
// signed long gallons = 0;

// last time we got an infrared delta
unsigned long lastIrTime = 0;

// previous infrared value we had (since the last delta)
int prevIrValue = 0;

// current value
bool irSensorActive = false;

void pulseSensorSetup()
{
    // set the analog pin resolution to the default
    // @see https://z-uno.z-wave.me/Reference/analogReadResolution/
    // which should give us a range of 0–1023
    analogReadResolution(10);

    // calculate how much time must pass without a pulse, in order to consider no-flow
    flowTimeout = TARGET_RATE_TIME / MIN_GPM / PULSE_RATE;

    // set the mode for the digital pins
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PULSE_SENSOR_PIN, INPUT_PULLUP);
}

/**
 * @brief checks if the infrared sensor is changing,
 * which means, movement is taking place on the spinning dial.
 *
 * @see IR_SENSOR_PIN
 * @see IR_DELTA_THRESHOLD
 *
 * @return true
 * @return false
 */
bool isIrSensorActive()
{
    // when within the timeout period of the last delta
    if (millis() - lastIrTime < IR_TIMEOUT)
    {
        return true;
    }

    int irSensorValue = analogRead(IR_SENSOR_PIN); // read the input pin

    // during initial run, just return false and set the previous value to the current one
    if (prevIrValue == 0)
    {
        prevIrValue = irSensorValue;
        return false;
    }

    // calculate the delta
    if (abs(irSensorValue - prevIrValue) > IR_DELTA_THRESHOLD)
    {
        // update the the last time we had a delta
        lastIrTime = millis();

        prevIrValue = irSensorValue;
        return true;
    }

    return false;
}

/**
 * @brief if the time passed is negative (ie. due to overflow) or
 * if the lastPulseTime is zero (initial pulse), it will return the flowTimeout,
 * to indicate the lowest possible flow, since we need 2 pulses at least, to calculate the actual flow.
 *
 * @see https://www.arduino.cc/reference/en/language/functions/time/millis/
 * @return milliseconds since the last pulse
 */
unsigned long timePassedSinceLastPulse()
{
    if (lastPulseTime > 0)
    {
        const unsigned long timePassed = millis() - lastPulseTime;
        if (timePassed > 0)
        {
            return timePassed;
        }
    }
    return flowTimeout;
}

/**
 * @brief updates the GPM based on the pulses received
 *
 */
void updateGPM()
{
    if (irSensorActive)
    {
        gpm = TARGET_RATE_TIME / timePassedSinceLastPulse() / PULSE_RATE;
    }
    else
    {
        gpm = 0;
        digitalWrite(LED_BUILTIN, LOW);
    }
    lastFlowUpdateTime = millis();
}

/**
 * @brief sets the GPM to the specified value
 *
 * @param newValue
 */
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
        zunoSendReport(FLOW_ZWAVE_CHANNEL);
    }
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
        // (ie. it can't pulse more times than the spec... so ignore those pulses)
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

void pulseSensorLoop()
{
    // update the value
    irSensorActive = isIrSensorActive();

    if (isPulseSensorActive())
    {
        // we got a pulse (this can only happen once, per pulse,
        // even if the meter stops right when the switch is on and the switch remains on)
        updateGPM();

        // increase gallons counter
        // gallons++;

        // send the new water meter value
        // zunoSendReport(METER_ZWAVE_CHANNEL);

        // keep the time passed, before we update the lastPulseTime
        prevTimePassedSinceLastPulse = timePassedSinceLastPulse();

        // reset the timer, after we have used it (with timePassedSinceLastPulse)
        lastPulseTime = millis();

        sendGPM(true);
        digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (gpm > 0.0)
    {
        // there's reported flow
        unsigned long timePassed = timePassedSinceLastPulse();
        if (timePassed >= flowTimeout)
        {
            // flow timed-out. consider it now that there's no flow
            updateGPM(0.0);
            digitalWrite(LED_BUILTIN, LOW);
            sendGPM(true);
        }
        else if (timePassed > prevTimePassedSinceLastPulse && millis() - lastFlowUpdateTime >= UPDATE_FLOW_FREQUENCY)
        {
            // when the time that has passed since the last pulse
            // is greater than the time that had passed since the previous to last one and
            // enough time has passed to update the flow
            updateGPM();
            sendGPM(false);
        }
    }
    else if (irSensorActive)
    {
        gpm = MIN_GPM;
        sendGPM(true);
        digitalWrite(LED_BUILTIN, HIGH);
    }
}