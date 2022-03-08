
#ifndef _UNDER_GROUND_TANK_ANALYSIS
#define _UNDER_GROUND_TANK_ANALYSIS

#define FUEL_FLOW_DICTATED HIGH
#define NO_FUEL_FLOW_DICTATED LOW

#define MAXIMUM_FUEL_LEVEL 92
#define MINIMUM_FUEL_LEVEL 65
#define AVERAGE_FUEL_LEVEL 75

#define ACTIVATE HIGH
#define DEACTIVATE LOW

void selectTank01();
void selectTank02(); 
void flushBadQualityFuel ();
void moinitorPumpState();
void processUndergroundTank02();
void windUpDayTankReplenishment();
void replenishUnderGroundTank02();

Task StartProcessToReplenishDayTank(100, TASK_FOREVER, &processUndergroundTank02);
Task ProcessActivitiesToReplenishDayTankWithTank01(3000, TASK_FOREVER, &selectTank01);
Task ProcessActivitiesToReplenishDayTankWithTank02(3000, TASK_FOREVER, &selectTank02);
Task EvacuateFuel(2000, TASK_FOREVER, &flushBadQualityFuel);
Task ReportPumpState(100, TASK_FOREVER, &moinitorPumpState);
Task TerminateDayTankReplenishment(3000, TASK_FOREVER, &windUpDayTankReplenishment);
Task activateUnderGroundTank02Replenishment(100, TASK_FOREVER, &replenishUnderGroundTank02);


struct UndergroundTankControl {
    int tank01FuelGuage;
    int tank02FuelGuage;
    int previousTank02FuelGuage;
    int tank01FuelGuageResult;
    int tank02FuelGuageResult;
    int tank02FuelLevelReport;
    int pump01AuxSenseAnlaysis;
    int pump02AuxSenseAnlaysis;
    int previousPump01AuxSenseAnlaysis;
    int previousPump02AuxSenseAnlaysis;
    int tank01;
    int tank02;
    int tank01CurrentLevel; 
    int tank02CurrentLevel;
    int minCount;
    int maxCount;
    int badFuelQualitySignalCount;
    int stopDayTankReplenishmentSignalCount;
    volatile unsigned long start_time_micros;
    volatile unsigned long delayTime;
}undergroundControl;


class UnderGroundFuelAnalysis{
    private:
        int Tank02Vol;

    public: 
      void processTankLevel();
      void replenishUnderGroundTank02();
      void stopReplenishmentProcess();
      void activateFlowControlValve(int);
      void deactivateFlowControlValve(int);

      void setTank02FuelLevel(int tank02Vol){
          Tank02Vol = tank02Vol;
      }
      int getTank02FuelLevel(){
          return Tank02Vol;
      }
};


void UnderGroundFuelAnalysis :: processTankLevel() {
    undergroundControl.tank02FuelLevelReport = getTank02FuelLevel();
    if (undergroundControl.tank02FuelLevelReport >= MAXIMUM_FUEL_LEVEL) {
        undergroundControl.maxCount++;
        undergroundControl.minCount = 0;
        if (undergroundControl.maxCount == 1){
            stopReplenishmentProcess();
            Serial.println("Tank02 Volume report: "+ (String)undergroundControl.tank02FuelLevelReport);
        }
    }else if(undergroundControl.tank02FuelLevelReport >= MINIMUM_FUEL_LEVEL){
        undergroundControl.minCount++;
        undergroundControl.maxCount = 0;
        if (undergroundControl.minCount == 1){
            activateUnderGroundTank02Replenishment.enable();
            Serial.println("Tank02 Volume is Below average, report: "+ (String)undergroundControl.tank02FuelLevelReport);
        }
    }
}

void UnderGroundFuelAnalysis :: replenishUnderGroundTank02() {
    //publish to Replenish Controller
    activateFlowControlValve(UNDER_GROUND_TANK_02_VALVE);
    // Serial.println("Tank02 Flow Control Valve: Activated");
}

void UnderGroundFuelAnalysis :: activateFlowControlValve(int pinNumber) {
    digitalWrite(pinNumber, ACTIVATE);
}


void UnderGroundFuelAnalysis :: deactivateFlowControlValve(int pinNumber) {
    digitalWrite(pinNumber, DEACTIVATE);
}

void UnderGroundFuelAnalysis :: stopReplenishmentProcess() {
    deactivateFlowControlValve(UNDER_GROUND_TANK_02_VALVE);
    fuelManagementControl.deleteTask(activateUnderGroundTank02Replenishment);
    Serial.println("Tank02 Flow Control Valve: Deactivated");
}

void replenishUnderGroundTank02(){
    UnderGroundFuelAnalysis underGroundFuelAnalysis;
    underGroundFuelAnalysis.activateFlowControlValve(UNDER_GROUND_TANK_02_VALVE);
    activateUnderGroundTank02Replenishment.disable();
}

void processUndergroundTank02() {
    char data[1050];
    StaticJsonDocument<750> doc;
    UnderGroundFuelAnalysis underGroundFuelAnalysis;
    //process the tank guage for Tank02, Ultrasonic Level sensor Reading
    undergroundControl.tank02FuelGuage = 96;
    delay(2000);
    if(undergroundControl.tank02FuelGuage != undergroundControl.previousTank02FuelGuage){
      doc["sensor"] = "UTank02";
      doc["volume"] = undergroundControl.tank02FuelGuage;

      String payload = doc.as<String>();
      payload.toCharArray(data, (payload.length() + 1));
      mqttClient.publish("FM-report", data);
      underGroundFuelAnalysis.setTank02FuelLevel(undergroundControl.tank02FuelGuage);
      underGroundFuelAnalysis.processTankLevel();
    }
    undergroundControl.previousTank02FuelGuage = undergroundControl.tank02FuelGuage;
}
#endif