
#ifndef _DAY_TANK_FUEL_ANALYSIS
#define _DAY_TANK_FUEL_ANALYSIS

class DayTankFuelAnalysis {
    private:
     int DayTankFuelLevel;

    public:
     void startPump01FlowMeterHysteresis();
     void startPump02FlowMeterHysteresis();
     void startInputFlowMeterToDayTankHysteresis();
     void startOutputFlowMeterFromDayTankHysteresis();
     void activateOutputPumpFromDayTank(int);
     void deactivateOutputPumpFromDayTank(int);
     void monitorFlowFromDayTank(int);
};


void DayTankFuelAnalysis :: startPump01FlowMeterHysteresis() {
  char data1[750];
  StaticJsonDocument<1024> doc;
  pump01FlowMeterControl.currentMillis = millis();
  if (pump01FlowMeterControl.currentMillis - pump01FlowMeterControl.previousMillis > pump01FlowMeterControl.flowInterval) {
    
    pump01FlowMeterControl.pulse1Sec = pump01FlowMeterControl.pulseCount;
    pump01FlowMeterControl.pulseCount = 0;
    pump01FlowMeterControl.flowRate = ((1000.0 / (millis() - pump01FlowMeterControl.previousMillis)) * pump01FlowMeterControl.pulse1Sec) / pump01FlowMeterControl.calibrationFactor;
    pump01FlowMeterControl.previousMillis = millis();
    pump01FlowMeterControl.flowMilliLitres = (pump01FlowMeterControl.flowRate / 60) * 1000;
    pump01FlowMeterControl.totalMilliLitres += pump01FlowMeterControl.flowMilliLitres;
    
    Serial.print("Pump01 From DayTank =====>>>Flow rate: ");
    Serial.print(int(pump01FlowMeterControl.flowRate));  
    Serial.print("L/min");
    Serial.print("\t");       
    Serial.print("Liquid Quantity: ");
    Serial.print(pump01FlowMeterControl.totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(pump01FlowMeterControl.totalMilliLitres / 1000);
    Serial.print("L");

    doc["source"] = "dischargePump01ToDayTank";
    doc["rate"] = pump01FlowMeterControl.flowRate;
    doc["vol"] = pump01FlowMeterControl.totalMilliLitres / 1000;
    String payload = doc.as<String>();
    payload.toCharArray(data1, (payload.length() + 1));
    mqttClient.publish("FM-Report", data1);
  }
}


void DayTankFuelAnalysis :: startPump02FlowMeterHysteresis(){
  char data1[750];
  StaticJsonDocument<1024> doc;
  pump02FlowMeterControl.currentMillis = millis();
  if (pump02FlowMeterControl.currentMillis - pump02FlowMeterControl.previousMillis > pump02FlowMeterControl.flowInterval) {
    
    pump02FlowMeterControl.pulse1Sec = pump02FlowMeterControl.pulseCount;
    pump02FlowMeterControl.pulseCount = 0;
    pump02FlowMeterControl.flowRate = ((1000.0 / (millis() - pump02FlowMeterControl.previousMillis)) * pump02FlowMeterControl.pulse1Sec) / pump02FlowMeterControl.calibrationFactor;
    pump02FlowMeterControl.previousMillis = millis();
    pump02FlowMeterControl.flowMilliLitres = (pump02FlowMeterControl.flowRate / 60) * 1000;
    pump02FlowMeterControl.totalMilliLitres += pump02FlowMeterControl.flowMilliLitres;
    
    Serial.print("Pump02 From DayTank=====>>>Flow rate: ");
    Serial.print(int(pump02FlowMeterControl.flowRate));  
    Serial.print("L/min");
    Serial.print("\t");       
    Serial.print("Liquid Quantity: ");
    Serial.print(pump02FlowMeterControl.totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(pump02FlowMeterControl.totalMilliLitres / 1000);
    Serial.println("L");

    doc["source"] = "dischargePump02ToDayTank";
    doc["rate"] = pump02FlowMeterControl.flowRate;
    doc["vol"] = pump02FlowMeterControl.totalMilliLitres / 1000;
    String payload = doc.as<String>();
    payload.toCharArray(data1, (payload.length() + 1));
    mqttClient.publish("FM-SYS", data1);
  }

}

void DayTankFuelAnalysis :: startInputFlowMeterToDayTankHysteresis() {
  char data1[750];   char data[1050];
  StaticJsonDocument<1024> doc;
  inputFlowMeterControl.incomingFuelPublishCount++;
  inputFlowMeterControl.currentMillis = millis();
  //check Fuel Temperature
  flowTemperature.requestTemperatures(); // Send the command to get temperatures
  inputFlowMeterControl.flowTemp = flowTemperature.getTempCByIndex(0);
  doc["source"] = "Supply-to-day-tank";
  doc["temp"] =  inputFlowMeterControl.flowTemp;

  if (inputFlowMeterControl.currentMillis - inputFlowMeterControl.previousMillis > inputFlowMeterControl.flowInterval) {
    
    inputFlowMeterControl.pulse1Sec = inputFlowMeterControl.pulseCount;
    inputFlowMeterControl.pulseCount = 0;
    inputFlowMeterControl.flowRate = ((1000.0 / (millis() - inputFlowMeterControl.previousMillis)) * inputFlowMeterControl.pulse1Sec) / inputFlowMeterControl.calibrationFactor;
    inputFlowMeterControl.previousMillis = millis();
    inputFlowMeterControl.flowMilliLitres = (inputFlowMeterControl.flowRate / 60) * 1000;
    inputFlowMeterControl.totalMilliLitres += inputFlowMeterControl.flowMilliLitres;
    inputFlowMeterControl.flowVolume = inputFlowMeterControl.totalMilliLitres / 1000;

    if (inputFlowMeterControl.incomingFuelPublishCount == 10) {
      String payload = "DTKR"+(String)inputFlowMeterControl.flowVolume;
      payload.toCharArray(data1, (payload.length() + 1));
      mqttClient.publish("FMPIN", data1);
      inputFlowMeterControl.incomingFuelPublishCount = 0;
    }

    Serial.print("Inputput From DayTank======>>> Flow rate: ");
    Serial.print(int(inputFlowMeterControl.flowRate));  
    Serial.print("L/min");
    Serial.print("\t");       
    Serial.print("Liquid Quantity: ");
    Serial.print(inputFlowMeterControl.totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(inputFlowMeterControl.totalMilliLitres / 1000);
    Serial.println("L");
  }
  String payload1 = doc.as<String>();
  payload1.toCharArray(data, (payload1.length() + 1));
  mqttClient.publish("FM-Report", data, 1);

}


void DayTankFuelAnalysis :: startOutputFlowMeterFromDayTankHysteresis(){
  char data1[750];
  StaticJsonDocument<1024> doc;
  outputFlowMeterControl.outgoingFuelPublishCount++;
  outputFlowMeterControl.currentMillis = millis();
  if (outputFlowMeterControl.currentMillis - outputFlowMeterControl.previousMillis > outputFlowMeterControl.flowInterval) {
    
    outputFlowMeterControl.pulse1Sec = outputFlowMeterControl.pulseCount;
    outputFlowMeterControl.pulseCount = 0;
    outputFlowMeterControl.flowRate = ((1000.0 / (millis() - outputFlowMeterControl.previousMillis)) * outputFlowMeterControl.pulse1Sec) / outputFlowMeterControl.calibrationFactor;
    outputFlowMeterControl.previousMillis = millis();
    outputFlowMeterControl.flowMilliLitres = (outputFlowMeterControl.flowRate / 60) * 1000;
    outputFlowMeterControl.totalMilliLitres += outputFlowMeterControl.flowMilliLitres;
    outputFlowMeterControl.outgoingFlowVolume = outputFlowMeterControl.totalMilliLitres / 1000;

    if (outputFlowMeterControl.outgoingFuelPublishCount == 10) {
      String payload = "DTSR"+(String)outputFlowMeterControl.outgoingFlowVolume;
      payload.toCharArray(data1, (payload.length() + 1));
      mqttClient.publish("FMPOUT", data1);
      outputFlowMeterControl.outgoingFuelPublishCount = 0;
    }
    
    Serial.print("Output From DayTank====>>> Flow rate: ");
    Serial.print(int(outputFlowMeterControl.flowRate));  
    Serial.print("L/min");
    Serial.print("\t");       
    Serial.print("Liquid Quantity: ");
    Serial.print(outputFlowMeterControl.totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(outputFlowMeterControl.totalMilliLitres / 1000);
    Serial.println("L");

  }
}

void DayTankFuelAnalysis :: activateOutputPumpFromDayTank(int sensor) {
    digitalWrite(sensor, ACTIVATE);
    Serial.println("Output Pump To Generators: Activated");
}

void DayTankFuelAnalysis :: deactivateOutputPumpFromDayTank(int sensor) {
    digitalWrite(sensor, DEACTIVATE);
    Serial.println("Output Pump To Generators: Deactivated");
}


void DayTankFuelAnalysis :: monitorFlowFromDayTank(int sensor) {
    char data[1050];
    StaticJsonDocument<750> doc;
    doc["sensor"] = "outputFromDayTank";
    dayTankFuelTank.flowSwitchState = digitalRead(sensor);
    if (dayTankFuelTank.flowSwitchState != dayTankFuelTank.previousFlowSwitchState) {
        if (dayTankFuelTank.flowSwitchState == ACTIVATED) {
            Serial.println("Flow Dectected!!!");
            doc["state"] = dayTankFuelTank.flowSwitchState;
            activateOutputPumpFromDayTank(ELECTRIC_PUMP);
            ProcessOutputFromDayTank.enable();
        }else {
           doc["state"] = dayTankFuelTank.flowSwitchState;
           Serial.println("Flow Stopped!!!");
           deactivateOutputPumpFromDayTank(ELECTRIC_PUMP);
           ProcessOutputFromDayTank.disable();
           resetOutputParameters();
        }
         String payload = doc.as<String>();
         payload.toCharArray(data, (payload.length() + 1));
         mqttClient.publish("dayTank-r", data);
    }
    dayTankFuelTank.previousFlowSwitchState = dayTankFuelTank.flowSwitchState;
}


void ReportFlowDetected(){
  DayTankFuelAnalysis dayTankFuelAnalysis;
  dayTankFuelAnalysis.monitorFlowFromDayTank(OUTPUT_FROM_DAY_TANK_FLOW_SWITCH);
}


void computeFuelInputToDayTank(){
    DayTankFuelAnalysis dayTankFuelAnalysis;
    dayTankFuelAnalysis.startInputFlowMeterToDayTankHysteresis();
}

void computeFuelOutputFromDayTank() {
    DayTankFuelAnalysis dayTankFuelAnalysis;
    dayTankFuelAnalysis.startOutputFlowMeterFromDayTankHysteresis();
}

void measureFuelFlowFromPump01() {
    DayTankFuelAnalysis dayTankFuelAnalysis;
    dayTankFuelAnalysis.startPump01FlowMeterHysteresis();
}

void measureFuelFlowFromPump02() {
    DayTankFuelAnalysis dayTankFuelAnalysis;
    dayTankFuelAnalysis.startPump02FlowMeterHysteresis();
}

void beginFlowComputationToDayTank() {
  //enable the pumps flowMeter hysteresis, then the input to dayTank FlowMeter as well
  ProcessPump01FlowRate.enable();
  ProcessPump02FlowRate.enable();
  ProcessInputToDayTank.enable();
  ProcessInputToDayTank.enableDelayed(1000);
  inputFlowMeterControl.deactivateSignalCount = 0;
  StartInputFlowComputationToDayTank.disable();
}


void abortFlowComputationToDayTank(){
  ProcessPump01FlowRate.disable();
  ProcessPump02FlowRate.disable();
  ProcessInputToDayTank.disable();
  inputFlowMeterControl.activateSignalCount = 0;
  StopInputFlowComputationToDayTank.disable();
  //reset all input parameters
  resetInputParameters();
}



char parseCommand(String Value, String inTopic) {
  if (inTopic == "ADTKR") {
    inputFlowMeterControl.activateSignalCount++;
    if (inputFlowMeterControl.activateSignalCount == 2) {
      if ((Value == "UT01") || (Value == "UT02")) {      //E.DRA = Enable DayTank Runtime Analysis
        StartInputFlowComputationToDayTank.enable();
      }
    }
  }else if (inTopic == "DTKR"){
    if (Value == "D.DTKR"){     // Deactivate Day-Tank Replenishment
      inputFlowMeterControl.deactivateSignalCount++;
      if (inputFlowMeterControl.deactivateSignalCount == 2) {
        StopInputFlowComputationToDayTank.enable();
      }
    }
  }else if (inTopic == "FM-Node03"){
    if (Value == "E.G01R") {
      generator01FuelTankControl.aSignalCount++;
      if (generator01FuelTankControl.aSignalCount == 2) {
        StartProcessToReplenishGen01.enable();
        generator01FuelTankControl.aSignalCount = 0;
      }
    }else if (Value == "E.G02R") {
      generator02FuelTankControl.aSignalCount++;
      if (generator02FuelTankControl.aSignalCount == 2) {
        StartProcessToReplenishGen02.enable();
        generator02FuelTankControl.aSignalCount = 0;
      }
    }else if (Value == "E.G03R") {
      generator03FuelTankControl.aSignalCount++;
      if (generator03FuelTankControl.aSignalCount == 2) {
        StartProcessToReplenishGen03.enable();
        generator03FuelTankControl.aSignalCount = 0;
      }
    }else if (Value == "D.G01R") {
      generator01FuelTankControl.dSignalCount++;
      if(generator01FuelTankControl.dSignalCount == 2){
        DactivateGen01Replenishment.enable();
        generator01FuelTankControl.dSignalCount = 0;
      }
    }else if (Value == "D.G02R") {
      generator02FuelTankControl.dSignalCount++;
      if(generator02FuelTankControl.dSignalCount == 2){
        DactivateGen02Replenishment.enable();
        generator02FuelTankControl.dSignalCount = 0;
      }
    }else if (Value == "D.G03R") {
      generator03FuelTankControl.dSignalCount++;
      if(generator03FuelTankControl.dSignalCount == 2){
        DactivateGen03Replenishment.enable();
        generator03FuelTankControl.dSignalCount = 0;
      }
    }else if (Value == "E.335G01R") {
      generator01335kva.aSignalCount++;
      if (generator01335kva.aSignalCount == 2) {
        StartProcessToReplenish335KvaGen01.enable();
        generator01335kva.aSignalCount = 0;
      }
    }else if (Value == "E.335G02R") {
      generator02335kva.aSignalCount++;
      if (generator02335kva.aSignalCount == 2) {
        StartProcessToReplenish335KvaGen01.enable();
        generator02335kva.aSignalCount = 0;
      }
    }else if (Value == "D.335G01R") {
      generator01335kva.dSignalCount++;
      if (generator01335kva.dSignalCount == 2) {
        Stop335kvaGen01ReplenishmentProcess.enable();
        generator01335kva.dSignalCount = 0;
      }
    }else if (Value == "D.335G02R") {
      generator02335kva.dSignalCount++;
      if (generator02335kva.dSignalCount == 2) {
        Stop335kvaGen02ReplenishmentProcess.enable();
        generator02335kva.dSignalCount = 0;
      }
    }
  }
}

#endif