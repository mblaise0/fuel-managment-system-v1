
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

void activateElectricIncomerPump(); 
void openIncomerSolenoidValve();
void closeIncomerSolenoidValve(); 
void postFuelTemperatureAnalysis();
void flowDetectionRunTime();
void supplyFlowMeterAnalysisRunTime();
void stopUndergroundTanksReplenishMentControlRuntime();
void tank02FlowMeterAnalysisRuntime(); 

Task StartTank02FlowMeterAnalysis(1000, TASK_FOREVER, &tank02FlowMeterAnalysisRuntime);
Task TerminateAllUndergroundTanksReplenishMent(1000, TASK_FOREVER, &stopUndergroundTanksReplenishMentControlRuntime);
Task StartSupplyFlowMeterAnalysis(1000, TASK_FOREVER, &supplyFlowMeterAnalysisRunTime);
Task DetectFlow(1000, TASK_FOREVER, &flowDetectionRunTime);
Task monitorPostFuelTemperature(1500, TASK_FOREVER, &postFuelTemperatureAnalysis);
Task activateElectricPump(1000, TASK_FOREVER, &activateElectricIncomerPump);
Task activateIncomerSolenoidValve(1000, TASK_FOREVER, &openIncomerSolenoidValve);
Task stopReplenishmentProcess(1000, TASK_FOREVER, &closeIncomerSolenoidValve);

class UnderGroundReplenishmentControl{
    private:
    int UnderGroundTankLevelGauge;
    int FuelQualitySense;
    
    public:
        StaticJsonDocument<1024> doc;
        void replenishUnderGroundFuelTank(String); 
        void stopReplenishmentProcess();
        void detectSupplyFlow();

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

        void ActivateSolenoidValve(int sensorPin) {
          digitalWrite(sensorPin, ACTIVATE);
        }

        void DeActivateSolenoidValve(int sensorPin) {
          digitalWrite(sensorPin, DEACTIVATE);
        }

        void ActivatePump(int pinNumber) {
          digitalWrite(pinNumber, ACTIVATE);
        }

        void DeActivatePump(int pinNumber){
          digitalWrite(pinNumber, DEACTIVATE);
        }


};


void UnderGroundReplenishmentControl :: replenishUnderGroundFuelTank(String tankName) {
    char data1[1050]; char data[750];
    underGroundTankAnalysis.activateCount++;
    underGroundTankAnalysis.updateServerCount++;
    underGroundTankTemperature01.requestTemperatures();

    underGroundTankAnalysis.tank01Temp = underGroundTankTemperature01.getTempCByIndex(0);  
    underGroundTankAnalysis.currentMillis = millis();
    if (underGroundTankAnalysis.activateCount == 1) {
      ActivatePump(INCOMER_PUMP_TO_UNDERGROUND_TANK);
      monitorPostFuelTemperature.enable();
      underGroundTankAnalysis.delayTime = 800;
      underGroundTankAnalysis.start_time_micros = millis();
      while ((millis() - underGroundTankAnalysis.start_time_micros) < underGroundTankAnalysis.delayTime){
        ;
      }
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

      doc["source"] = tankName;
      doc["rate"] = underGroundTankAnalysis.flowRate;
      doc["vol"] = underGroundTankAnalysis.currentRefillVolume;
      doc["temp"] =  underGroundTankAnalysis.tank01Temp;
      String payload = doc.as<String>();
      payload.toCharArray(data1, (payload.length() + 1));
      mqttClient.publish("refillVolume", data1);

      if (underGroundTankAnalysis.updateServerCount == 15) {
        String payload1 = tankName+(String)underGroundTankAnalysis.currentRefillVolume;  
        payload1.toCharArray(data, (payload1.length() + 1));
        mqttClient.publish("FMPIN", data, true);
        underGroundTankAnalysis.updateServerCount = 0;
      }
  }
}


void UnderGroundReplenishmentControl :: stopReplenishmentProcess() {
  underGroundTankAnalysis.flowRate = 0.0;      //reset the flowmeter reading
  underGroundTankAnalysis.flowMilliLitres = 0;
  underGroundTankAnalysis.totalMilliLitres = 0;
  underGroundTankAnalysis.previousMillis = 0;
  underGroundTankAnalysis.activateCount = 0;

  int flowControl[] = {UNDERGROUND_TANK01_SOLENOID_VALVE, INCOMER_SOLENOID_VAVLE};
  DeActivatePump(INCOMER_PUMP_TO_UNDERGROUND_TANK);
  monitorPostFuelTemperature.disable();
  StartTank02FlowMeterAnalysis.disable();
  StartSupplyFlowMeterAnalysis.disable();
  for (int i = 0; i < 2; i++) {
    DeActivateSolenoidValve(flowControl[i]);
    underGroundTankAnalysis.delayTime = 500;
    underGroundTankAnalysis.start_time_micros = millis();
    while ((millis() - underGroundTankAnalysis.start_time_micros) < underGroundTankAnalysis.delayTime){
    ;
    }
  }
}

void UnderGroundReplenishmentControl :: detectSupplyFlow() {
  char data[1050];
  String payload;
  underGroundTankAnalysis.tankReplenishmentReport = digitalRead(INCOMER_UNDERGROUND_TANK_FLOW_SWITCH);
  underGroundTankAnalysis.fuelQualityTest = getFuelQualityStatus(); 
  if (underGroundTankAnalysis.tankReplenishmentReport != underGroundTankAnalysis.previousTankReplenishmentReport) {
    if (underGroundTankAnalysis.tankReplenishmentReport == FUEL_FLOW_DICTATED) {
        Serial.println("Flow Detected....");
        if (underGroundTankAnalysis.fuelQualityTest >= FUEL_QUALITY_GOOD) {
           ActivateSolenoidValve(INCOMER_SOLENOID_VAVLE);
           //send a message to FM-Processor to select Tank to Replenish
            payload = "FDQG"+(String)underGroundTankAnalysis.tankReplenishmentReport;   //Flow Detected and Quality Good
           Serial.println("Fuel-Quality Test: passed     Incoming-Solenoid-Valve: Activated");
        }else {
          String payload1 = "FDQB"+(String)underGroundTankAnalysis.tankReplenishmentReport;  //Flow Detected and Quality Bad
          payload1.toCharArray(data, (payload1.length() + 1));
          mqttClient.publish("FM-Node02", data, 2);
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
           Serial.println("Fuel-Quality Test: Failed");
        }
    }else {
      DeActivatePump(INCOMER_PUMP_TO_UNDERGROUND_TANK);
      payload = "FND"+(String)underGroundTankAnalysis.tankReplenishmentReport;  
      Serial.println("Flow Stopped....");
    }
    payload.toCharArray(data, (payload.length() + 1));
    mqttClient.publish("FMPIN", data, 2);
  }
   underGroundTankAnalysis.previousTankReplenishmentReport = underGroundTankAnalysis.tankReplenishmentReport;
}


// StartSupplyFlowMeterAnalysis.enable();

void supplyFlowMeterAnalysisRunTime() {
  UnderGroundReplenishmentControl underGroundTank01ReplenishmentControl;
  underGroundTank01ReplenishmentControl.replenishUnderGroundFuelTank("UTk01");

}

void flowDetectionRunTime() {
  UnderGroundReplenishmentControl underGroundReplenishmentControl;
  underGroundTankAnalysis.fuelQualityTestResult = 70;  //Assumption that fuel quality is good
  underGroundReplenishmentControl.setFuelQualityStatus(underGroundTankAnalysis.fuelQualityTestResult);
  underGroundReplenishmentControl.detectSupplyFlow();
}


void stopUndergroundTanksReplenishMentControlRuntime() {
  UnderGroundReplenishmentControl underGroundReplenishmentControl;
  underGroundReplenishmentControl.stopReplenishmentProcess();

  underGroundTankAnalysis.flowRate = 0.0;      //reset the flowmeter reading
  underGroundTankAnalysis.flowMilliLitres = 0;
  underGroundTankAnalysis.totalMilliLitres = 0;
  underGroundTankAnalysis.previousMillis = 0;

  monitorPostFuelTemperature.disable();
  TerminateAllUndergroundTanksReplenishMent.disable();
}


void tank02FlowMeterAnalysisRuntime() {
  UnderGroundReplenishmentControl underGroundReplenishmentControl;
  underGroundReplenishmentControl.replenishUnderGroundFuelTank("UTk02");
}

/************************* Emergency Controls *************************************************************/

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

/**************************************************************************************************************/

void postFuelTemperatureAnalysis() {
   StaticJsonDocument<1024> doc;
   char data1[1050];
   underGroundTankTemperature02.requestTemperatures();
   underGroundTankAnalysis.postFuelTemp = underGroundTankTemperature02.getTempCByIndex(0);

   doc["source"] = "UndergroundTank-supply";
   doc["temp"] =  underGroundTankAnalysis.postFuelTemp;
   String payload = doc.as<String>();
   payload.toCharArray(data1, (payload.length() + 1));
   mqttClient.publish("FM-Report", data1, 2);
}

UnderGroundReplenishmentControl underGroundReplenishmentControl;

char reciever(String Value, String inTopic) {
  if (Value == "O.INVL"){
    activateElectricPump.enable();
  }else if (Value == "A.PUMP"){
    activateIncomerSolenoidValve.enable();
  }else if (Value == "C.INVL"){
    stopReplenishmentProcess.enable();
  }else if (Value == "A.UT01R"){   // Start Fuel Volume Analysis
    underGroundReplenishmentControl.ActivateSolenoidValve(UNDERGROUND_TANK01_SOLENOID_VALVE);
    StartSupplyFlowMeterAnalysis.enable();
  }else if (Value == "D.UT01R"){   //Deactivate Underground Tank01 Replenishment
    TerminateAllUndergroundTanksReplenishMent.enable();
  }else if (Value == "S.UTR") {   // Stop All Underground Tanks Replenishment Process
    TerminateAllUndergroundTanksReplenishMent.enable();
  }else if (Value == "A.UTR") {
    StartTank02FlowMeterAnalysis.enable();
  }
}

#endif