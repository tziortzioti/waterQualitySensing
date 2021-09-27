float getTemp(){              
  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {    
      Serial.println("no more sensors on chain, reset search!");
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); 
  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad

  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();
  byte MSB = data[1];
  byte LSB = data[0];
  float tempRead = ((MSB << 8) | LSB); 
  float TemperatureSum = tempRead / 16;
  return TemperatureSum;
}



float getPh() {
  phVoltage = analogRead(PH_PIN)/1024.0*5000;  // read the voltage
  phV = ph.readPH(phVoltage,temperature);  // convert voltage to pH with temperature compensation
  phV=phV+phOffset;
  return phV;
}


float getTDS() {
  
  for (int i=0; i<=TDS_samples; i++) {
    TDSanalogBuffer[i] = analogRead(TDS_PIN);
    delay(40);
  }

  TDSaverageVoltage = getMedianNum(TDSanalogBuffer,TDS_samples) * 5.0 / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
  float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  float compensationVolatge=TDSaverageVoltage/compensationCoefficient;  //temperature compensation
  tdsV=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
  return tdsV;
}



float getDO() {
    for (int i=0; i<=DO_samples; i++) {
    DOanalogBuffer[i] = analogRead(DO_PIN);
    delay(30);
  }

  DOaverageVoltage = getMedianNum(DOanalogBuffer,DO_samples) * 5000.00 / 1024.0; // read the value more stable by the median filtering algorithm
  doV = pgm_read_float_near( &SaturationValueTab[0] + (int)(SaturationDoTemperature+0.5) ) * DOaverageVoltage / SaturationDoVoltage;  //calculate the do value, doValue = Voltage / SaturationDoVoltage * SaturationDoValue(with temperature compensation)
  Serial.println(doV);
  return doV;
}



void readDoCharacteristicValues(void)
{
    EEPROM_read(SaturationDoVoltageAddress, SaturationDoVoltage);
    EEPROM_read(SaturationDoTemperatureAddress, SaturationDoTemperature);
    if(EEPROM.read(SaturationDoVoltageAddress)==0xFF && EEPROM.read(SaturationDoVoltageAddress+1)==0xFF && EEPROM.read(SaturationDoVoltageAddress+2)==0xFF && EEPROM.read(SaturationDoVoltageAddress+3)==0xFF)
    {
      SaturationDoVoltage = 1127.6;   //default voltage:1127.6mv
      EEPROM_write(SaturationDoVoltageAddress, SaturationDoVoltage);
    }
    if(EEPROM.read(SaturationDoTemperatureAddress)==0xFF && EEPROM.read(SaturationDoTemperatureAddress+1)==0xFF && EEPROM.read(SaturationDoTemperatureAddress+2)==0xFF && EEPROM.read(SaturationDoTemperatureAddress+3)==0xFF)
    {
      SaturationDoTemperature = 25.0;   //default temperature is 25^C
      EEPROM_write(SaturationDoTemperatureAddress, SaturationDoTemperature);
    }
}



int getMedianNum(int bArray[], int iFilterLen) {
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) {
        for (i = 0; i < iFilterLen - j - 1; i++) {
          if (bTab[i] > bTab[i + 1]) {
            bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
            bTab[i + 1] = bTemp;
          }
        }
      }
      if ((iFilterLen & 1) > 0) {
        bTemp = bTab[(iFilterLen - 1) / 2];
      } else {
        bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      }
      return bTemp;
}


float getEC() {
  ecVoltage = analogRead(EC_PIN)/1024.0*5000;  // read the voltage
  ecV =  ec.readEC(ecVoltage,temperature);  // convert voltage to EC with temperature compensation
  return ecV;
}
