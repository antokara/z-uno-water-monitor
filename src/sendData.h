/**
 * According to Z-Wave Plus restrictions, values from Sensor Multilevel channels
 * (defined via ZUNO_SENSOR_MULTILEVEL macro) will not be sent unsolicitedly
 * to Life Line more often than every 30 seconds.
 *
 * @see https://z-uno.z-wave.me/Reference/zunoSendReport/
 */
#define SEND_DATA_FREQUENCY 30000

// last time we sent the data
unsigned long lastSendTime = 0;

/**
 * @brief checks if we should send data to zwave lifeline
 *
 * @return true we could send
 * @return false we should not send
 * @see SEND_DATA_FREQUENCY
 */
bool shouldSendData()
{
    return abs(millis() - lastSendTime) > SEND_DATA_FREQUENCY;
}

/**
 * @brief attempts to send the data to the defined zwave channel.
 *        shouldSendData() must be called prior to this, otherwise a silent failure can happen
 *
 * @param zwaveChannel the zwave channel to send the data
 * @see shouldSendData()
 */
void sendData(byte zwaveChannel)
{
    lastSendTime = millis();
    zunoSendReport(zwaveChannel);
}
