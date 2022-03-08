
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
void windUpDayTankReplenishment();
void replenishUnderGroundTank02();
void closeUndergroundTank02Valve();

Task TerminateUndergroundTank02Valve(1000, TASK_FOREVER, closeUndergroundTank02Valve);
Task ReplenishDayTankWithUndergroundTank01(1000, TASK_FOREVER, &selectTank01);
Task ReplenishDayTankWithUndergroundTank02(1000, TASK_FOREVER, &selectTank02);
Task EvacuateFuel(1000, TASK_FOREVER, &flushBadQualityFuel);
Task ReportPumpState(500, TASK_FOREVER, &moinitorPumpState);
Task TerminateDayTankReplenishment(3000, TASK_FOREVER, &windUpDayTankReplenishment);
Task ActivateUnderGroundTank02Replenishment(100, TASK_FOREVER, &replenishUnderGroundTank02);


struct UndergroundTankControl {
    int tank02FuelLevelReport;
    int pump01AuxSenseAnlaysis;
    int pump02AuxSenseAnlaysis;
    int previousPump01AuxSenseAnlaysis;
    int previousPump02AuxSenseAnlaysis;
    int tank01;
    int tank02;
    int badFuelQualitySignalCount;
    int stopDayTankReplenishmentSignalCount;
    volatile unsigned long start_time_micros;
    volatile unsigned long delayTime;
    int replenishCount;
    int stopReplenishmentCount;
}undergroundControl, dayTankControl;


class UnderGroundFuelAnalysis{

    public: 
        void replenishUnderGroundTank02() {
            //publish to Replenish Controller
            activateFlowControlValve(UNDER_GROUND_TANK_02_VALVE);
            // Serial.println("Tank02 Flow Control Valve: Activated");
        }

        void activateFlowControlValve(int pinNumber) {
            digitalWrite(pinNumber, ACTIVATE);
        }


        void deactivateFlowControlValve(int pinNumber) {
            digitalWrite(pinNumber, DEACTIVATE);
        }

        void stopReplenishmentProcess() {
            deactivateFlowControlValve(UNDER_GROUND_TANK_02_VALVE);
            Serial.println("Tank02 Flow Control Valve: Deactivated");
        }
};




void replenishUnderGroundTank02(){
    UnderGroundFuelAnalysis underGroundFuelAnalysis;
    underGroundFuelAnalysis.activateFlowControlValve(UNDER_GROUND_TANK_02_VALVE);
    Serial.println("UndergroundTank02Valve: Activated");
    ActivateUnderGroundTank02Replenishment.disable();
} 



void closeUndergroundTank02Valve() {
    UnderGroundFuelAnalysis underGroundFuelAnalysis;
    underGroundFuelAnalysis.stopReplenishmentProcess();
    Serial.println("UndergroundTank02Valve: Deactivated");
    TerminateUndergroundTank02Valve.disable();
}

#endif