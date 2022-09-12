/**
 * @brief frequency in milliseconds, to allow sending of zwave data to the controller.
 * this is a big number because we rely on polling from the controller,
 * instead of unsolicited lifeline updates from the node, for the gallons counter.
 *
 * 3600000 = 1 hour
 */
#define SEND_GALLONS_COUNTER_FREQUENCY 3600000

// the (digital) pin that we need to connect the water meter pulse switch.
// the other end, needs to go the ground (GND) pin, of zuno.
#define PULSE_SENSOR_PIN 12

// the (analog) pin that we connect to the
// InfraRed AO (analog output) pin of the sensor
// you may use 3-6, which maps to A0-A3
#define IR_SENSOR_PIN 4

// then 1-based, number of z-wave channels, as defined in ZUNO_SETUP_CHANNELS
#define FLOW_ZWAVE_CHANNEL 1
#define GALLONS_COUNTER_ZWAVE_CHANNEL 3

// the delta we must calculate between two infrared sensor values
// in order to be considered an actual change/motion
// (true, when greater than)
#define IR_DELTA_THRESHOLD 1

// time in milliseconds that a delta count period lasts
#define IR_COUNT_PERIOD 5000

// number of delta counts that need to happen within the timeout period
// for the IR sensor to be considered ON (to avoid potential noise)
// (true, when greater than)
#define IR_COUNT_THRESHOLD 9

// minimum gallons per minute that the water meter can detect.
// this helps us detect no-flow, by calculating a "time-out" when
// too much time has passed since a new pulse.
#define MIN_GPM 0.1

// time in milliseconds for our target rate
// 60000msecs = 60secs = 1minute rate for GPM
#define TARGET_RATE_TIME 60000.0

// frequency to update the flow, in milliseconds (between pulse and timeout only)
#define UPDATE_FLOW_FREQUENCY 2500

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

// time passed between previous pulse and the current one
// TODO: rename last/prev/current to clear things up
unsigned int prevTimePassedSinceLastPulse = 0;

// current gallons per minute
float gpm = 0.0;

// last flow GPM value we sent (to avoid let's say sending 0.0 twice in a row)
float lastGpmSent = 0.0;

// time that must pass without a pulse, in order to be considered no-flow
unsigned int flowTimeout = 0;

// previous infrared value we had (since the last delta)
int prevIrValue = 0;

// last time we had an infrared count increase
unsigned long lastIrCountTime = 0;

// last time the infrared count was above the threshold for the period
// signifying continuous flow
unsigned long lastIrCountAboveThresholdTime = 0;

// number of delta counts that are happening, within the timeout period
// @see IR_COUNTS_THRESHOLD
unsigned int irCounts = 0;

// current value
bool isIrSensorActive = false;

// gallons to increase the water meter by
float gallonsCounter = 0.0;

// buffer for  the gallons to increase the water meter by.
// this is the internal counter, before we update and send the new value to the controller.
float gallonsCounterBuffer = 0.0;

// last time we sent the gallons counter
unsigned long lastGallonsCounterSendTime = 0;

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
 * @brief if we should send the pressure to the controller.
 * a separate throttler to keep the unsolicited updated in lifeline prioritize the flow.
 *
 * @return true
 * @return false
 */
bool shouldSendGallonsCounter()
{
    return abs(millis() - lastGallonsCounterSendTime) > SEND_GALLONS_COUNTER_FREQUENCY && shouldSendData();
}

/**
 * @brief checks if the gallons counter needs to be set or reset and then sent to the controller.
 *        this should be called on every loop iteration (in case we need to reset the counter).
 *
 */
void checkGallonsCounter()
{
    if (shouldSendGallonsCounter())
    {
        bool send = false;
        if (gallonsCounter == 0.0)
        {
            // don't update when both counters are set to zero
            if (gallonsCounterBuffer != 0.0)
            {
                // set the exposed counter
                gallonsCounter = gallonsCounterBuffer;
                // reset the buffer
                gallonsCounterBuffer = 0.0;
                send = true;
            }
        }
        else
        {
            // reset the exposed counter
            gallonsCounter = 0.0;
            send = true;
        }

        // only send when there's a new value
        if (send)
        {
            // send the new value
            lastGallonsCounterSendTime = millis();
            sendData(GALLONS_COUNTER_ZWAVE_CHANNEL);
        }
    }
}

/**
 * @brief this should only be called, every time we get a new pulse.
 *        it increases the gallonsCounterBuffer by one
 */
void increaseGallonsCounter()
{
    gallonsCounterBuffer++;
}

/**
 * @brief checks if the infrared sensor is changing,
 * which means, movement is taking place on the spinning dial.
 *
 * when true, it sets irSensorValue to true and false when not.
 * it debounces the sensor properly and on the first reading, it leaves it at false.
 *
 * @see IR_SENSOR_PIN
 * @see IR_DELTA_THRESHOLD
 *
 * @return true
 * @return false
 */
void updateIrSensorActive()
{
    int irSensorValue = analogRead(IR_SENSOR_PIN); // read the input pin

    // when verbose debugging, log all the delta values
    #if defined(DEBUG_VERBOSE)
        if (irSensorValue != prevIrValue) {
            Serial.print("irSensorValue: ");
            Serial.print(irSensorValue);
            Serial.print(", delta: ");
            Serial.println(abs(irSensorValue - prevIrValue));
        }
    #endif


    // only when the value has changed
    if (prevIrValue == 0)
    {
        // during initial run, just set the previous value to the current one
        // isIrSensorActive should already be set to false
        prevIrValue = irSensorValue;
    }
    else if (abs(irSensorValue - prevIrValue) > IR_DELTA_THRESHOLD)
    {
        if (abs(millis() - lastIrCountTime > IR_COUNT_PERIOD)) {
            // when debugging, print the IR counts for the period
            // that were above the delta threshold but
            // even when they are under the count threshold
            #if defined(DEBUG)
                Serial.print("irSensorValue: ");
                Serial.print(irSensorValue);
                Serial.print(", irCounts: ");
                Serial.println(irCounts);
            #endif

            // when the IR counts have reached the threshold for the period
            if (irCounts > IR_COUNT_THRESHOLD)
            {
                if (!isIrSensorActive) {
                    // mark our IR sensor as active
                    isIrSensorActive = true;

                    // when debugging, log the flow ON event
                    #if defined(DEBUG)
                        Serial.println("flow ON!");
                    #endif
                }

                // reset the last time we had a count over the threshold,
                // within the period
                lastIrCountAboveThresholdTime = millis();
            }

            // reset the count period timer
            lastIrCountTime = millis();

            // reset the counts for the period
            irCounts = 0;
        } else {
            // increase the counts for the period
            irCounts++;
        }

        // keep the last value
        prevIrValue = irSensorValue;
    }
    else if (isIrSensorActive && abs(millis() - lastIrCountAboveThresholdTime > IR_COUNT_PERIOD * 2))
    {
        // when the delta is less than the threshold and
        // double the count time period has passed, only then,
        // mark the sensor as inactive.
        // (this is a debouncer of some sort)
        isIrSensorActive = false;
        // reset the count as well
        irCounts = 0;

        // when debugging, log the flow OFF event
        #if defined(DEBUG)
            Serial.println("flow OFF!");
        #endif
    }
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
        const unsigned long timePassed = abs(millis() - lastPulseTime);
        if (timePassed > 0)
        {
            return timePassed;
        }
    }
    return flowTimeout;
}

/**
 * @brief updates the GPM based on the pulses received
 */
void updateGPM()
{
    gpm = TARGET_RATE_TIME / timePassedSinceLastPulse() / PULSE_RATE;
}

/**
 * @brief sets the GPM to the specified value
 *
 * @param newValue
 */
void updateGPM(float newValue)
{
    gpm = newValue;
}

/**
 * @brief sends the zwave data for GPM
 *
 * According to Z-Wave Plus restrictions, values from Sensor Multilevel channels
 * (defined via ZUNO_SENSOR_MULTILEVEL macro) will not be sent unsolicitedly
 * to Life Line more often than every 30 seconds.
 *
 * It also only sends if there is a new value, since the last time sent it.
 */
void sendGPM()
{
    if (lastGpmSent != gpm && shouldSendData())
    {
        lastGpmSent = gpm;
        sendData(FLOW_ZWAVE_CHANNEL);
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
    updateIrSensorActive();

    if (isPulseSensorActive())
    {
        // since we got a pulse, force the IR sensor to be true
        // the pulse is more reliable
        isIrSensorActive = true;
        lastIrCountTime = millis();
        irCounts = IR_COUNT_THRESHOLD + 1;

        // when debugging, log the pulse received event
        #if defined(DEBUG)
            Serial.println("pulse received!");
        #endif

        // we got a pulse (this can only happen once, per pulse,
        // even if the meter stops right when the switch is on and the switch remains on)
        updateGPM();
        if (gpm < MIN_GPM)
        {
            // when there's pulse but too much time has passed since the last pulse
            // make sure we set a minimum flow.
            // This can happen when water starts flowing after a long period (pulse timeout)
            updateGPM(MIN_GPM);
        }

        sendGPM();
        digitalWrite(LED_BUILTIN, HIGH);
        increaseGallonsCounter();

        // keep the time passed, before we update the lastPulseTime
        prevTimePassedSinceLastPulse = timePassedSinceLastPulse();

        // reset the timer, after we have used it (with timePassedSinceLastPulse)
        lastPulseTime = millis();
    }
    else if (isIrSensorActive)
    {
        const float prevGPM = gpm;

        if (timePassedSinceLastPulse() > prevTimePassedSinceLastPulse)
        {
            // when the time that has passed since the last pulse
            // is greater than the time that had passed since the previous to last one and
            // enough time has passed to update the flow

            // when there's no pulse but there's flow (the IR sensor is active)
            // update the current GPM depending on the last pulse time
            updateGPM();
        }

        if (gpm < MIN_GPM)
        {
            // when there's no pulse but there's flow (the IR sensor is active) and
            // when the GPM has been set to zero because too much time has passed since the last pulse
            // make sure we set a minimum flow.
            // This can happen when water starts flowing after a long period (pulse timeout)
            // but before a pulse is sent.
            updateGPM(MIN_GPM);
        }

        if (prevGPM == 0.0 && gpm > 0.0)
        {
            // turn on the LED, when we just set the gpm > 0 from 0
            digitalWrite(LED_BUILTIN, HIGH);
        }

        sendGPM();
    }
    else if (gpm > 0.0)
    {
        // no pulse or flow (the IR sensor is inactive) but there's residual GPM
        // reset everything to 0
        updateGPM(0.0);
        sendGPM();
        digitalWrite(LED_BUILTIN, LOW);
    }

    // after all other checks have taken place and
    // any data has been sent, check if we need to set/reset the gallons counter
    checkGallonsCounter();
}