
#ifndef _PUMP_SELECTION_TO_DAY_TANK
#define _PUMP_SELECTION_TO_DAY_TANK

#define ACTIVATED HIGH

class PumpSelectionToDayTank : UnderGroundFuelAnalysis{
    private:
     int Tank01Vol;
    public:
      void selectTank01ToReplenishDayTank();
      void selectTank02ToReplenishDayTank();
      void setTank01FuelLevel(int tank01Vol) {
         Tank01Vol = tank01Vol;
         Serial.println("Tank01Vol from set-func: "+(String)Tank01Vol);
      }

      int getTank01FuelLevel() {
         return Tank01Vol;
      }

      void StartDayTankReplenishment(int pump01DischargeValve, int pump02DischargeValve, int valveA, int valveB, int pump01, int pump02){
        int pumpAndValve[] = {valveA, valveB, pump01, pump02};
        for (int i = 0; i < 4; i++) {
          digitalWrite(pumpAndValve[i], ACTIVATE);
          undergroundControl.delayTime = 500;
          undergroundControl.start_time_micros = millis();  
          while ((millis() - undergroundControl.start_time_micros) < undergroundControl.delayTime){
            ;
          }
        }
        //activate discharge valves
        int dischargeValves[] = {pump01DischargeValve, pump02DischargeValve};
        for (int i = 0; i < 2; i++){
          digitalWrite(dischargeValves[i], ACTIVATE);
        }
          Serial.println("Discharge valves activated...");
      }
 
      void StopDayTankReplenishment(int pump01DischargeValve, int pump02DischargeValve, int valveA, int valveB, int pump01, int pump02){
        int pumpAndValve[] = {valveA, valveB, pump01, pump02};
        for (int i = 0; i < 4; i++) {
          digitalWrite(pumpAndValve[i], DEACTIVATE);
          undergroundControl.delayTime = 200;
          undergroundControl.start_time_micros = millis();  
          while ((millis() - undergroundControl.start_time_micros) < undergroundControl.delayTime){
            ;
          }
        }
        //deactivate discharge valves
        int dischargeValves[] = {pump01DischargeValve, pump02DischargeValve};
        for (int i = 0; i < 2; i++){
          digitalWrite(dischargeValves[i], DEACTIVATE);
        } 
        Serial.println("Stop DayTankReplenishment...");
        undergroundControl.stopDayTankReplenishmentSignalCount = 0;
      }
     
};


void PumpSelectionToDayTank :: selectTank01ToReplenishDayTank() {
  //select Tank01
  StartDayTankReplenishment(UNDER_GROUND_TANK_PUMP01, UNDER_GROUND_TANK_PUMP02, UNDER_GROUND_TANK01_VALVE_1A, UNDER_GROUND_TANK01_VALVE_1B, DISCHARGE_VALVE_FROM_PUMP_01, DISCHARGE_VALVE_FROM_PUMP_02);
  Serial.println("Pump01 Valve1A: Activated");
  ReplenishDayTankWithUndergroundTank01.disable();  
}

void PumpSelectionToDayTank :: selectTank02ToReplenishDayTank() {
  //select Tank02
  StartDayTankReplenishment(UNDER_GROUND_TANK_PUMP01, UNDER_GROUND_TANK_PUMP02, UNDER_GROUND_TANK02_VALVE_2A, UNDER_GROUND_TANK02_VALVE_2B, DISCHARGE_VALVE_FROM_PUMP_01, DISCHARGE_VALVE_FROM_PUMP_02);
  Serial.println("Pump02, Valve2A: Activated");
  ReplenishDayTankWithUndergroundTank02.disable();
}


 
void selectTank01(){
  PumpSelectionToDayTank pumpSelectionToDayTank;
  char data[750];
  StaticJsonDocument<750> doc;

  String payload = "E.DRA";       //Enable Day-tank Replenishment Analysis sent to FM-Node03
  payload.toCharArray(data, (payload.length() + 1));
  for (int i = 0; i < 3; i++){
    mqttClient.publish("FM-node03", data);
  }
  pumpSelectionToDayTank.selectTank01ToReplenishDayTank();

}

void selectTank02(){
  PumpSelectionToDayTank pumpSelectionToDayTank;
  char data[750];
  StaticJsonDocument<750> doc;

  String payload = "E.DRA";       //inform FM-node03
  payload.toCharArray(data, (payload.length() + 1));
  for (int i = 0; i < 3; i++){
     mqttClient.publish("FM-node03", data);
  }
  pumpSelectionToDayTank.selectTank02ToReplenishDayTank();
}

void flushBadQualityFuel () {
  UnderGroundFuelAnalysis underGroundFuelAnalysis;
  underGroundFuelAnalysis.activateFlowControlValve(EVACUATION_VALVE);
  undergroundControl.badFuelQualitySignalCount = 0;
}


void windUpDayTankReplenishment(){
  PumpSelectionToDayTank pumpSelectionToDayTank;
  char data[750];
  StaticJsonDocument<750> doc;
  
  String payload = "D.DRA";       //inform FM-node03
  payload.toCharArray(data, (payload.length() + 1));
  for (int i = 0; i < 3; i++){
    mqttClient.publish("FM-node03", data);
  }
  Serial.println("DayTankReplenishment: Terminated");
  pumpSelectionToDayTank.StopDayTankReplenishment(UNDER_GROUND_TANK_PUMP01, UNDER_GROUND_TANK_PUMP02, UNDER_GROUND_TANK01_VALVE_1A, UNDER_GROUND_TANK01_VALVE_1B, DISCHARGE_VALVE_FROM_PUMP_01, DISCHARGE_VALVE_FROM_PUMP_02);  
  PumpSelectionToDayTank pumpSelectionToDayTank2;
  pumpSelectionToDayTank2.StopDayTankReplenishment(UNDER_GROUND_TANK_PUMP01, UNDER_GROUND_TANK_PUMP02, UNDER_GROUND_TANK02_VALVE_2A, UNDER_GROUND_TANK02_VALVE_2B, DISCHARGE_VALVE_FROM_PUMP_01, DISCHARGE_VALVE_FROM_PUMP_02);
  TerminateDayTankReplenishment.disable();
  undergroundControl.stopDayTankReplenishmentSignalCount = 0;
}



char parseIncomingCommand(String Value, String inTopic) {
  Serial.println("From callback: " +Value);
  if (inTopic == "ADTKR"){    //Activate Day-Tank Replenishmment 
    dayTankControl.replenishCount++;
    if (dayTankControl.replenishCount == 2){
      if (Value == "UT01") {
        ReplenishDayTankWithUndergroundTank01.enable();
      }else if (Value == "UT02") {
          ReplenishDayTankWithUndergroundTank02.enable();
      }
      dayTankControl.replenishCount = 0;
    }
  }else if (inTopic == "DTKR"){
    if (Value == "D.DTKR"){     // Deactivate Day-Tank Replenishment
      undergroundControl.stopDayTankReplenishmentSignalCount++;
      if (undergroundControl.stopDayTankReplenishmentSignalCount == 2) {
          TerminateDayTankReplenishment.enable();
      }
    }
  }else if (inTopic == "FM-Node02") {
    if (Value == "FDQB1"){    //Flow Detected and Quality Bad
      undergroundControl.badFuelQualitySignalCount++;
      if (undergroundControl.badFuelQualitySignalCount == 2){
          EvacuateFuel.enable();
      }
    }else if (Value == "A.UT02R"){      //Activate Underground Tank02 Replensihment
      undergroundControl.replenishCount++;
      if (undergroundControl.replenishCount == 2) {
        ActivateUnderGroundTank02Replenishment.enable();
        undergroundControl.replenishCount = 0;
      }
    }else if (Value == "D.UT02R"){
      undergroundControl.stopReplenishmentCount++;
      if (undergroundControl.stopReplenishmentCount == 2) {
          TerminateUndergroundTank02Valve.enable();
          undergroundControl.stopReplenishmentCount = 0;
      }
    }
  }  

}

#endif