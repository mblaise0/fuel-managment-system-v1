
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
     void processDayTankFuelLevel();
     void setDayTankFuelLevel(int dayTankFuelLevel) {
       DayTankFuelLevel = dayTankFuelLevel;
     }
     int getDayTankFuelLevel() {
       return DayTankFuelLevel;
     }
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
    mqttClient.publish("FM-SYS", data1);
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
  char data1[750];
  StaticJsonDocument<1024> doc;
  
  doc["source"] = "DayTank";
  inputFlowMeterControl.currentMillis = millis();
  //check Fuel Temperature
  flowTemperature.requestTemperatures(); // Send the command to get temperatures
  inputFlowMeterControl.flowTemp = flowTemperature.getTempCByIndex(0);
  doc["temp"] =  inputFlowMeterControl.flowTemp;

  if (inputFlowMeterControl.currentMillis - inputFlowMeterControl.previousMillis > inputFlowMeterControl.flowInterval) {
    
    inputFlowMeterControl.pulse1Sec = inputFlowMeterControl.pulseCount;
    inputFlowMeterControl.pulseCount = 0;
    inputFlowMeterControl.flowRate = ((1000.0 / (millis() - inputFlowMeterControl.previousMillis)) * inputFlowMeterControl.pulse1Sec) / inputFlowMeterControl.calibrationFactor;
    inputFlowMeterControl.previousMillis = millis();
    inputFlowMeterControl.flowMilliLitres = (inputFlowMeterControl.flowRate / 60) * 1000;
    inputFlowMeterControl.totalMilliLitres += inputFlowMeterControl.flowMilliLitres;
    
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
  doc["rate"] = inputFlowMeterControl.flowRate;
  doc["vol"] = inputFlowMeterControl.totalMilliLitres / 1000;
  String payload = doc.as<String>();
  payload.toCharArray(data1, (payload.length() + 1));
  mqttClient.publish("FM-SYS", data1);
}


void DayTankFuelAnalysis :: startOutputFlowMeterFromDayTankHysteresis(){
  char data1[750];
  StaticJsonDocument<1024> doc;
  outputFlowMeterControl.currentMillis = millis();
  if (outputFlowMeterControl.currentMillis - outputFlowMeterControl.previousMillis > outputFlowMeterControl.flowInterval) {
    
    outputFlowMeterControl.pulse1Sec = outputFlowMeterControl.pulseCount;
    outputFlowMeterControl.pulseCount = 0;
    outputFlowMeterControl.flowRate = ((1000.0 / (millis() - outputFlowMeterControl.previousMillis)) * outputFlowMeterControl.pulse1Sec) / outputFlowMeterControl.calibrationFactor;
    outputFlowMeterControl.previousMillis = millis();
    outputFlowMeterControl.flowMilliLitres = (outputFlowMeterControl.flowRate / 60) * 1000;
    outputFlowMeterControl.totalMilliLitres += outputFlowMeterControl.flowMilliLitres;
    
    Serial.print("Output From DayTank====>>> Flow rate: ");
    Serial.print(int(outputFlowMeterControl.flowRate));  
    Serial.print("L/min");
    Serial.print("\t");       
    Serial.print("Liquid Quantity: ");
    Serial.print(outputFlowMeterControl.totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(outputFlowMeterControl.totalMilliLitres / 1000);
    Serial.println("L");
    
    doc["source"] = "outputFromDayTank";
    doc["rate"] = outputFlowMeterControl.flowRate;
    doc["vol"] = outputFlowMeterControl.totalMilliLitres / 1000;
    String payload = doc.as<String>();
    payload.toCharArray(data1, (payload.length() + 1));
    mqttClient.publish("FM-SYS", data1);
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

void DayTankFuelAnalysis :: processDayTankFuelLevel() {
  dayTankFuelTank.fuelLevelReport = getDayTankFuelLevel();
  if (dayTankFuelTank.fuelLevelReport >= MAXIMUM_FUEL_LEVEL){
    //send a message to FM-Node02 to stop the pumps and vavles
    if (dayTankFuelTank.DTMAXCount == 1) {
      dayTankFuelTank.DTMINCount = 0;
      dayTankFuelTank.DTACount = 0;
      mqttClient.publish("FM-Node02", "S.DAYR");          // S.DAYR = Stop DayTank Replenishment
      StopInputFlowComputationToDayTank.enable();
      Serial.println("DayTankFuelLEvelReport: Maximum");
    }
  }else{
    if (dayTankFuelTank.fuelLevelReport <= MINIMUM_FUEL_LEVEL){
      if (dayTankFuelTank.DTMINCount == 1) {
         dayTankFuelTank.DTMAXCount = 0;
         mqttClient.publish("FM-Node02", "R.DTR");      // R.DTANK = Replenish DayTank
         StartInputFlowComputationToDayTank.enable();
         Serial.println("DayTankFuelLEvelReport: Minimum");
      }
    }
  }
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
            ProcessOutputToDayTank.enable();
        }else {
           doc["state"] = dayTankFuelTank.flowSwitchState;
           Serial.println("Flow Stopped!!!");
           deactivateOutputPumpFromDayTank(ELECTRIC_PUMP);
           ProcessOutputToDayTank.disable();
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
  StartInputFlowComputationToDayTank.disable();
}


void abortFlowComputationToDayTank(){
  ProcessPump01FlowRate.disable();
  ProcessPump02FlowRate.disable();
  ProcessInputToDayTank.disable();
  StopInputFlowComputationToDayTank.disable();
}


void measureDayTankFuelLevel() {
  char data[750];
  StaticJsonDocument<750> doc;
  DayTankFuelAnalysis dayTankFuelAnalysis;
  //process dayTank fuel level
  dayTankFuelTank.fuelLevelResult = 78;
  // if (dayTankFuelTank.fuelLevelResult <= 70) {     
  //   //make sure it sends just once
  //   if (dayTankFuelTank.DTACount == 1){
  //     mqttClient.publish("ADTank", "A.DTR");      //inform FM-Node02 to get ready to replenish
  //   }
  // }

  if (dayTankFuelTank.fuelLevelResult != dayTankFuelTank.previousFuelLevelResult){
    Serial.println("DayTankFuelLevel: "+ (String)dayTankFuelTank.fuelLevelResult);
    doc["sensor"] = "DayTankGauge";
    doc["vol"] = dayTankFuelTank.fuelLevelResult;
    
    String payload = doc.as<String>();
    payload.toCharArray(data, (payload.length() + 1));
    mqttClient.publish("FM-DayTank", data);
    dayTankFuelAnalysis.setDayTankFuelLevel(dayTankFuelTank.fuelLevelResult);
    dayTankFuelTank.delayTime = 2000;
    dayTankFuelTank.start_time_micros = millis();  
    while ((millis() - dayTankFuelTank.start_time_micros) < dayTankFuelTank.delayTime){
        ;
    } 
  }
  dayTankFuelTank.previousFuelLevelResult = dayTankFuelTank.fuelLevelResult;
}

char recieveDirective(String Value, String inTopic) {
    if (Value == "E.DRA") {      //E.DRA = Enable DayTank Runtime Analysis
        StartInputFlowComputationToDayTank.enable();
    }else if (Value == "E.ODT"){    //E.ODT = Enable OutputDayTank 
        ProcessOutputToDayTank.enable();
    }else if (Value == "D.DRA") {    //D.IDT = Disable Input to Day Tank processing
        StopInputFlowComputationToDayTank.enable();
    }else if (Value == "D.ODT") {
        ProcessOutputToDayTank.disable();
    }
}

#endif