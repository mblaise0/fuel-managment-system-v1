
#ifndef _UNDERGROUND_TANK_FUEL_CONTROL
#define _UNDERGROUND_TANK_FUEL_CONTROL

#define FUEL_FLOW_DICTATED HIGH
#define NO_FUEL_FLOW_DICTATED LOW

#define MAXIMUM_FUEL_LEVEL 92
#define MINIMUM_FUEL_LEVEL 65
#define AVERAGE_FUEL_LEVEL 75

#define FUEL_QUALITY_GOOD 50

#define ACTIVATE HIGH
#define DEACTIVATE LOW

void processTank01ReplenishingFuelQuality();
void activateElectricIncomerPump(); 
void openIncomerSolenoidValve();
void closeIncomerSolenoidValve(); 
void postFuelTemperatureAnalysis();

Task monitorPostFuelTemperature(1500, TASK_FOREVER, &postFuelTemperatureAnalysis);
Task monitorUndergroundTankFuelLevelAndTakeAction(1500, TASK_FOREVER, &processTank01ReplenishingFuelQuality);
Task activateElectricPump(1000, TASK_FOREVER, &activateElectricIncomerPump);
Task activateIncomerSolenoidValve(1000, TASK_FOREVER, &openIncomerSolenoidValve);
Task stopReplenishmentProcess(1000, TASK_FOREVER, &closeIncomerSolenoidValve);

class UnderGroundReplenishmentControl{
    private:
    int UnderGroundTankLevelGauge;
    int FuelQualitySense;
    
    public:
        StaticJsonDocument<1024> doc;
        void replenishUnderGroundFuelTank(); 
        void stopReplenishmentProcess();
        void processUnderGroundTankFuelLevelAnalysis();
        void ActivateSolenoidValve(int);
        void DeActivateSolenoidValve(int);
        void ActivatePump(int);
        void DeActivatePump(int);

        void setUnderGroundFuelTankLevel(int underGroundTankLevelGauge){
        UnderGroundTankLevelGauge = underGroundTankLevelGauge;
        }
    
        int getUnderGroundFuelTankLevel() {
        return UnderGroundTankLevelGauge;
        }
    
        void setFuelQualityStatus(int fuelQualitySense){
        FuelQualitySense = fuelQualitySense;
        }
    
        int getFuelQualityStatus() {
        return FuelQualitySense;
        }
};


void UnderGroundReplenishmentControl :: ActivateSolenoidValve(int sensorPin) {
   digitalWrite(sensorPin, ACTIVATE);
}

void UnderGroundReplenishmentControl :: DeActivateSolenoidValve(int sensorPin) {
   digitalWrite(sensorPin, DEACTIVATE);
}

void UnderGroundReplenishmentControl :: ActivatePump(int pinNumber) {
  digitalWrite(pinNumber, ACTIVATE);
}

void UnderGroundReplenishmentControl :: DeActivatePump(int pinNumber){
  digitalWrite(pinNumber, DEACTIVATE);
}

void UnderGroundReplenishmentControl :: replenishUnderGroundFuelTank() {
    char data1[1050];
    fuelQualityTest.qualityReport = getFuelQualityStatus(); 
//    Serial.println("Fuel-Quality: "+ (String)fuelQualityTest.qualityReport ); 
    //process tank temperature 
    underGroundTankTemperature01.requestTemperatures();
    underGroundTankAnalysis.tank01Temp = underGroundTankTemperature01.getTempCByIndex(0);
    
    underGroundTankAnalysis.tankReplenishmentReport = digitalRead(INCOMER_UNDERGROUND_TANK_FLOW_SWITCH);
    underGroundTankAnalysis.currentMillis = millis();

    if (underGroundTankAnalysis.tankReplenishmentReport == FUEL_FLOW_DICTATED) {
      underGroundTankAnalysis.replenishCount++;
      if (underGroundTankAnalysis.replenishCount == 1) {
        ActivateSolenoidValve(INCOMER_SOLENOID_VAVLE);
        underGroundTankAnalysis.stopReplenishCount = 0;
        doc["sensor"] = "incomingValve";
        doc["state"] = "Activated";
        String payload = doc.as<String>();
        payload.toCharArray(data1, (payload.length() + 1));
        mqttClient.publish("FM-report", data1);
        Serial.println("Flow Detected....");
      }
      if (fuelQualityTest.qualityReport >= FUEL_QUALITY_GOOD) {
          fuelQualityTest.goodFuelCount++;
          if (fuelQualityTest.goodFuelCount == 1)
          {
            mqttClient.publish("FM-node02", "F.GOOD");
            ActivatePump(INCOMER_PUMP_TO_UNDERGROUND_TANK);
            ActivateSolenoidValve(UNDERGROUND_TANK01_SOLENOID_VALVE);
          }
          
          if (underGroundTankAnalysis.currentMillis - underGroundTankAnalysis.previousMillis > underGroundTankAnalysis.flowInterval) {
        
            underGroundTankAnalysis.pulse1Sec = underGroundTankAnalysis.pulseCount;
            underGroundTankAnalysis.pulseCount = 0;
            underGroundTankAnalysis.flowRate = ((1000.0 / (millis() - underGroundTankAnalysis.previousMillis)) * underGroundTankAnalysis.pulse1Sec) / underGroundTankAnalysis.calibrationFactor;
            underGroundTankAnalysis.previousMillis = millis();
            underGroundTankAnalysis.flowMilliLitres = (underGroundTankAnalysis.flowRate / 60) * 1000;
            underGroundTankAnalysis.totalMilliLitres += underGroundTankAnalysis.flowMilliLitres;
            underGroundTankAnalysis.currentRefillVolume = underGroundTankAnalysis.totalMilliLitres / 1000;
            #ifdef DEBUG
            // Print the flow rate for this second in litres / minute
            Serial.print("Underground-Tank Flow rate: ");
            Serial.print(int(underGroundTankAnalysis.flowRate));  // Print the integer part of the variable
            Serial.print("L/min");
            Serial.print("\t");       // Print tab space
        
            // Print the cumulative total of litres flowed since starting
            Serial.print("Underground-Tank Output Liquid Quantity: ");
            Serial.print(underGroundTankAnalysis.totalMilliLitres);
            Serial.print("mL / ");
            Serial.print(underGroundTankAnalysis.totalMilliLitres / 1000);
            Serial.print("L ");
            Serial.println(" Temperature is: "+ (String)underGroundTankAnalysis.tank01Temp+ " Celcius");
            #endif
            
            doc["source"] = "underTank01";
            doc["rate"] = underGroundTankAnalysis.flowRate;
            doc["vol"] = underGroundTankAnalysis.currentRefillVolume;
            doc["temp"] =  underGroundTankAnalysis.tank01Temp;
            String payload = doc.as<String>();
            payload.toCharArray(data1, (payload.length() + 1));
            mqttClient.publish("refillVolume", data1);
            monitorPostFuelTemperature.enable();
        }
      }else{
         Serial.println("Fuel-quality-test: Failed");
         for (int i; i < 3; i++){
            mqttClient.publish("FM-node02",  "F.BAD", 2); 
            underGroundTankAnalysis.delayTime = 500;
            underGroundTankAnalysis.start_time_micros = millis();
            while ((millis() - underGroundTankAnalysis.start_time_micros) < underGroundTankAnalysis.delayTime){
            ;
          }
        }
        DeActivatePump(INCOMER_PUMP_TO_UNDERGROUND_TANK);
        monitorPostFuelTemperature.disable();
        int incomingValves[] = {INCOMER_SOLENOID_VAVLE, UNDERGROUND_TANK01_SOLENOID_VALVE};
        for (int i = 0; i < 2; i++){
          DeActivateSolenoidValve(incomingValves[i]);
          underGroundTankAnalysis.delayTime = 500;
          underGroundTankAnalysis.start_time_micros = millis();
          while ((millis() - underGroundTankAnalysis.start_time_micros) < underGroundTankAnalysis.delayTime){
            ;
          }
        }
      }
    }else{
      underGroundTankAnalysis.replenishCount = 0;
      underGroundTankAnalysis.stopReplenishCount = 0;
      fuelQualityTest.goodFuelCount = 0;

      underGroundTankAnalysis.flowRate = 0.0;      //reset the flowmeter reading
      underGroundTankAnalysis.flowMilliLitres = 0;
      underGroundTankAnalysis.totalMilliLitres = 0;
      underGroundTankAnalysis.previousMillis = 0;

      DeActivatePump(INCOMER_PUMP_TO_UNDERGROUND_TANK);
      monitorPostFuelTemperature.disable();
      int incomingValves[] = {INCOMER_SOLENOID_VAVLE, UNDERGROUND_TANK01_SOLENOID_VALVE};
      for (int i = 0; i < 2; i++){
        DeActivateSolenoidValve(incomingValves[i]);
        underGroundTankAnalysis.delayTime = 500;
        underGroundTankAnalysis.start_time_micros = millis();
        while ((millis() - underGroundTankAnalysis.start_time_micros) < underGroundTankAnalysis.delayTime){
          ;
        }
      }
    }
}

void UnderGroundReplenishmentControl :: stopReplenishmentProcess() {
  int flowControl[] = {INCOMER_SOLENOID_VAVLE, UNDERGROUND_TANK01_SOLENOID_VALVE};
  for (int i = 0; i < 2; i++) {
    DeActivateSolenoidValve(flowControl[i]);
    underGroundTankAnalysis.delayTime = 500;
    underGroundTankAnalysis.start_time_micros = millis();
    while ((millis() - underGroundTankAnalysis.start_time_micros) < underGroundTankAnalysis.delayTime){
    ;
    }
  }
}

void UnderGroundReplenishmentControl :: processUnderGroundTankFuelLevelAnalysis() {
  char data1[750];
  underGroundTankAnalysis.stopReplenishCount++;
  underGroundTankAnalysis.fuelLevelReport = getUnderGroundFuelTankLevel();
  if (underGroundTankAnalysis.fuelLevelReport >= MAXIMUM_FUEL_LEVEL) {
       if (underGroundTankAnalysis.stopReplenishCount == 1){
        undergroundTank01FuelTankState = FUEL_TANK_FULL;
        underGroundTankAnalysis.replenishCount = 0;
        fuelQualityTest.goodFuelCount = 0;

        underGroundTankAnalysis.flowRate = 0.0;    //reset the flowmeter reading
        underGroundTankAnalysis.flowMilliLitres = 0;
        underGroundTankAnalysis.totalMilliLitres = 0;
        underGroundTankAnalysis.previousMillis = 0;

        stopReplenishmentProcess();
        DeActivatePump(INCOMER_PUMP_TO_UNDERGROUND_TANK);
        #ifdef DEBUG
        Serial.println("Underground Tank01 Fuel Level has attained maximum level; replenishment Stopped...");
        #endif
        mqttClient.publish("fuelReport", "Underground Tank01 Fuel Level: 100%"); 
      }
  }else if (underGroundTankAnalysis.fuelLevelReport < MAXIMUM_FUEL_LEVEL) {
      undergroundTank01FuelTankState = FUEL_REFILL_IN_PROGRESS;
      underGroundTankAnalysis.stopReplenishCount = 0;
      if (underGroundTankAnalysis.fuelLevelReport <= MINIMUM_FUEL_LEVEL){
        replenishUnderGroundFuelTank();
        // #ifdef DEBUG
        // Serial.println("Underground Tank01 Fuel level has gone below average, refill immediately...");
        // #endif
      }
        
  }
}

void processTank01ReplenishingFuelQuality(){
   StaticJsonDocument<1024> doc;
   char data1[750];
   UnderGroundReplenishmentControl UnderGroundReplenishmentControl;

   fuelQualityTest.fuelQualityReading = 70;
   UnderGroundReplenishmentControl.setFuelQualityStatus(fuelQualityTest.fuelQualityReading);
//      delay(2000);
   underGroundTankAnalysis.fuelLevelReport = 54;      //update the server of the undergroundTank01FuelLevel periodically
   if(underGroundTankAnalysis.fuelLevelReport != underGroundTankAnalysis.previousFuelLevelReport) {
      String payload = (String)underGroundTankAnalysis.fuelLevelReport;
      payload.toCharArray(data1, (payload.length() + 1));
      mqttClient.publish("ADTank", data1);
   }
   underGroundTankAnalysis.previousFuelLevelReport = underGroundTankAnalysis.fuelLevelReport; 
   UnderGroundReplenishmentControl.setUnderGroundFuelTankLevel(underGroundTankAnalysis.fuelLevelReport);
   UnderGroundReplenishmentControl.processUnderGroundTankFuelLevelAnalysis();
         
}



/************************* Emergency Controls ******************************/
void activateElectricIncomerPump() {
  UnderGroundReplenishmentControl UnderGroundReplenishmentControl;
  UnderGroundReplenishmentControl.ActivatePump(INCOMER_PUMP_TO_UNDERGROUND_TANK);
  Serial.println("Pump activated...");
}

void openIncomerSolenoidValve() {
  UnderGroundReplenishmentControl UnderGroundReplenishmentControl;
  UnderGroundReplenishmentControl.ActivateSolenoidValve(INCOMER_SOLENOID_VAVLE);
  Serial.println("IncomingSolenoidValve activated...");
}

void closeIncomerSolenoidValve() {
  UnderGroundReplenishmentControl UnderGroundReplenishmentControl;
  UnderGroundReplenishmentControl.DeActivateSolenoidValve(INCOMER_SOLENOID_VAVLE);
  underGroundTankAnalysis.delayTime = 1500;
  underGroundTankAnalysis.start_time_micros = millis();
  while ((millis() - underGroundTankAnalysis.start_time_micros) < underGroundTankAnalysis.delayTime){
    ;
  }
  UnderGroundReplenishmentControl.DeActivatePump(INCOMER_PUMP_TO_UNDERGROUND_TANK);
}


void postFuelTemperatureAnalysis() {
   StaticJsonDocument<1024> doc;
   char data1[1050];
   underGroundTankTemperature02.requestTemperatures();
   underGroundTankAnalysis.postFuelTemp = underGroundTankTemperature02.getTempCByIndex(0);

   doc["source"] = "post-temp02";
   doc["temp"] =  underGroundTankAnalysis.postFuelTemp;
   String payload = doc.as<String>();
   payload.toCharArray(data1, (payload.length() + 1));
   mqttClient.publish("temp02", data1, 2);
}

char reciever(String Value, String inTopic) {
  UnderGroundReplenishmentControl UnderGroundReplenishmentControl;
  if (Value == "O.INVL"){
    activateElectricPump.enable();
  }else if (Value == "A.PUMP"){
    activateIncomerSolenoidValve.enable();
  }else if (Value == "C.INVL"){
    stopReplenishmentProcess.enable();
  }
}

#endif