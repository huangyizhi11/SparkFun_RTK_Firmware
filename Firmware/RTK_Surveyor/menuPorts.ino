void menuPorts()
{
  if (productVariant == RTK_SURVEYOR)
    menuPortsSurveyor();
  else if (productVariant == RTK_EXPRESS || productVariant == RTK_EXPRESS_PLUS || productVariant == RTK_FACET || productVariant == RTK_FACET_LBAND)
    menuPortsMultiplexed();
}

//Set the baud rates for the radio and data ports
void menuPortsSurveyor()
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Port Menu"));

    Serial.print(F("1) Set serial baud rate for Radio Port: "));
    Serial.print(getSerialRate(COM_PORT_UART2));
    Serial.println(F(" bps"));

    Serial.print(F("2) Set serial baud rate for Data Port: "));
    Serial.print(getSerialRate(COM_PORT_UART1));
    Serial.println(F(" bps"));

    Serial.println(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
    {
      Serial.print(F("Enter baud rate (4800 to 921600) for Radio Port: "));
      int newBaud = getNumber(menuTimeout); //Timeout after x seconds
      if (newBaud < 4800 || newBaud > 921600)
      {
        Serial.println(F("Error: baud rate out of range"));
      }
      else
      {
        settings.radioPortBaud = newBaud;
        if (online.gnss == true)
          i2cGNSS.setSerialRate(newBaud, COM_PORT_UART2); //Set Radio Port
      }
    }
    else if (incoming == '2')
    {
      Serial.print(F("Enter baud rate (4800 to 921600) for Data Port: "));
      int newBaud = getNumber(menuTimeout); //Timeout after x seconds
      if (newBaud < 4800 || newBaud > 921600)
      {
        Serial.println(F("Error: baud rate out of range"));
      }
      else
      {
        settings.dataPortBaud = newBaud;
        if (online.gnss == true)
          i2cGNSS.setSerialRate(newBaud, COM_PORT_UART1); //Set Data Port
      }
    }

    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

  while (Serial.available()) Serial.read(); //Empty buffer of any newline chars
}

//Set the baud rates for the radio and data ports
void menuPortsMultiplexed()
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Port Menu"));

    Serial.print(F("1) Set Radio port serial baud rate: "));
    Serial.print(getSerialRate(COM_PORT_UART2));
    Serial.println(F(" bps"));

    Serial.print(F("2) Set Data port connections: "));
    if (settings.dataPortChannel == MUX_UBLOX_NMEA)
      Serial.println(F("NMEA TX Out/RX In"));
    else if (settings.dataPortChannel == MUX_PPS_EVENTTRIGGER)
      Serial.println(F("PPS OUT/Event Trigger In"));
    else if (settings.dataPortChannel == MUX_I2C_WT)
    {
      if (zedModuleType == PLATFORM_F9P)
        Serial.println(F("I2C SDA/SCL"));
      else if (zedModuleType == PLATFORM_F9R)
        Serial.println(F("Wheel Tick/Direction"));
    }
    else if (settings.dataPortChannel == MUX_ADC_DAC)
      Serial.println(F("ESP32 DAC Out/ADC In"));


    if (settings.dataPortChannel == MUX_UBLOX_NMEA)
    {
      Serial.print(F("3) Set Data port serial baud rate: "));
      Serial.print(getSerialRate(COM_PORT_UART1));
      Serial.println(F(" bps"));
    }
    else if (settings.dataPortChannel == MUX_PPS_EVENTTRIGGER)
    {
      Serial.println(F("3) Configure External Triggers"));
    }

    Serial.println(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
    {
      Serial.print(F("Enter baud rate (4800 to 921600) for Radio Port: "));
      int newBaud = getNumber(menuTimeout); //Timeout after x seconds
      if (newBaud < 4800 || newBaud > 921600)
      {
        Serial.println(F("Error: baud rate out of range"));
      }
      else
      {
        settings.radioPortBaud = newBaud;
        if (online.gnss == true)
          i2cGNSS.setSerialRate(newBaud, COM_PORT_UART2); //Set Radio Port
      }
    }
    else if (incoming == '2')
    {
      Serial.println(F("\n\rEnter the pin connection to use (1 to 4) for Data Port: "));
      Serial.println(F("1) NMEA TX Out/RX In"));
      Serial.println(F("2) PPS OUT/Event Trigger In"));
      if (zedModuleType == PLATFORM_F9P)
        Serial.println(F("3) I2C SDA/SCL"));
      else if (zedModuleType == PLATFORM_F9R)
        Serial.println(F("3) Wheel Tick/Direction"));
      Serial.println(F("4) ESP32 DAC Out/ADC In"));

      int muxPort = getNumber(menuTimeout); //Timeout after x seconds
      if (muxPort < 1 || muxPort > 4)
      {
        Serial.println(F("Error: Pin connection out of range"));
      }
      else
      {
        settings.dataPortChannel = (muxConnectionType_e)(muxPort - 1); //Adjust user input from 1-4 to 0-3
        setMuxport(settings.dataPortChannel);
      }
    }
    else if (incoming == '3' && settings.dataPortChannel == MUX_UBLOX_NMEA)
    {
      Serial.print(F("Enter baud rate (4800 to 921600) for Data Port: "));
      int newBaud = getNumber(menuTimeout); //Timeout after x seconds
      if (newBaud < 4800 || newBaud > 921600)
      {
        Serial.println(F("Error: baud rate out of range"));
      }
      else
      {
        settings.dataPortBaud = newBaud;
        if (online.gnss == true)
          i2cGNSS.setSerialRate(newBaud, COM_PORT_UART1); //Set Data Port
      }
    }
    else if (incoming == '3' && settings.dataPortChannel == MUX_PPS_EVENTTRIGGER)
    {
      menuPortHardwareTriggers();
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

  while (Serial.available()) Serial.read(); //Empty buffer of any newline chars
}

//Configure the behavior of the PPS and INT pins on the ZED-F9P
//Most often used for logging events (inputs) and when external triggers (outputs) occur
void menuPortHardwareTriggers()
{
  int menuTimeoutExtended = 30; //Increase time needed for complex data entry (mount point ID, ECEF coords, etc).

  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Port Hardware Trigger Menu"));

    Serial.print(F("1) Enable External Pulse: "));
    if (settings.enableExternalPulse == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    if (settings.enableExternalPulse == true)
    {
      Serial.print(F("2) Set time between pulses: "));
      Serial.print(settings.externalPulseTimeBetweenPulse_us / 1000.0);
      Serial.println(F("ms"));

      Serial.print(F("3) Set pulse length: "));
      Serial.print(settings.externalPulseLength_us / 1000.0);
      Serial.println(F("ms"));

      Serial.print(F("4) Set pulse polarity: "));
      if (settings.externalPulsePolarity == PULSE_RISING_EDGE) Serial.println(F("Rising"));
      else Serial.println(F("Falling"));
    }

    Serial.print(F("5) Log External Events: "));
    if (settings.enableExternalHardwareEventLogging == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.println(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
    {
      settings.enableExternalPulse ^= 1;
    }
    else if (incoming == '2' && settings.enableExternalPulse == true)
    {
      Serial.print(F("Time between pulses in milliseconds: "));
      double pulseTime = getDouble(menuTimeoutExtended); //Timeout after x seconds

      if (pulseTime != STATUS_GETNUMBER_TIMEOUT && pulseTime != STATUS_PRESSED_X)
      {
        settings.externalPulseTimeBetweenPulse_us = pulseTime * 1000;
      }
    }
    else if (incoming == '3' && settings.enableExternalPulse == true)
    {
      Serial.print(F("Pulse length in milliseconds: "));
      double pulseLength = getDouble(menuTimeoutExtended); //Timeout after x seconds

      if (pulseLength != STATUS_GETNUMBER_TIMEOUT && pulseLength != STATUS_PRESSED_X)
      {
        settings.externalPulseLength_us = pulseLength * 1000;
      }
    }
    else if (incoming == '4' && settings.enableExternalPulse == true)
    {
      if (settings.externalPulsePolarity == PULSE_RISING_EDGE)
        settings.externalPulsePolarity = PULSE_FALLING_EDGE;
      else
        settings.externalPulsePolarity = PULSE_RISING_EDGE;
    }
    else if (incoming == '5')
    {
      settings.enableExternalHardwareEventLogging ^= 1;
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

  while (Serial.available()) Serial.read(); //Empty buffer of any newline chars

  beginExternalTriggers(); //Update with new settings
}

void eventTriggerReceived(UBX_TIM_TM2_data_t ubxDataStruct)
{
  // It is the rising edge of the sound event (TRIG) which is important
  // The falling edge is less useful, as it will be "debounced" by the loop code
  if (ubxDataStruct.flags.bits.newRisingEdge) // 1 if a new rising edge was detected
  {
    Serial.println("Rising Edge Event");

    triggerCount = ubxDataStruct.count;
    towMsR = ubxDataStruct.towMsR; // Time Of Week of rising edge (ms)
    towSubMsR = ubxDataStruct.towSubMsR; // Millisecond fraction of Time Of Week of rising edge in nanoseconds

    newEventToRecord = true;
  }
}
