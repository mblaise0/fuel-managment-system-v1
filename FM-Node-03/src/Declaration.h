
#ifndef _DECLARATIONS_
#define _DECLARATIONS_

#define ACTIVATE HIGH
#define DEACTIVATE LOW
#define ACTIVATED HIGH

#define MAXIMUM_FUEL_LEVEL 92
#define MINIMUM_FUEL_LEVEL 50

void ReportFlowDetected();
void computeFuelInputToDayTank();
void computeFuelOutputFromDayTank();
void measureFuelFlowFromPump01();
void measureFuelFlowFromPump02();
void beginFlowComputationToDayTank(); 
void abortFlowComputationToDayTank();
void refillGen01Tank();
void refillGen02Tank();
void refillGen03Tank();
void stopGen01Replenishment();
void stopGen02Replenishment();
void stopGen03Replenishment();
void replenish335KvaGen01();
void abort335kvaGen01Replenishment();
void replenish335KvaGen02();
void abort335kvaGen02Replenishment();
void beginProcessToReplenish335KvaGen01();
void beginProcessToReplenish335KvaGen02();
void beginProcessToReplenishGen01FuelTank();
void beginProcessToReplenishGen02FuelTank();
void beginProcessToReplenishGen03FuelTank();

Task StartProcessToReplenishGen01(100, TASK_FOREVER, &beginProcessToReplenishGen01FuelTank);
Task StartProcessToReplenishGen02(100, TASK_FOREVER, &beginProcessToReplenishGen02FuelTank);
Task StartProcessToReplenishGen03(100, TASK_FOREVER, &beginProcessToReplenishGen03FuelTank);
Task StartProcessToReplenish335KvaGen01(1000, TASK_FOREVER, &beginProcessToReplenish335KvaGen01);
Task StartProcessToReplenish335KvaGen02(1000, TASK_FOREVER, &beginProcessToReplenish335KvaGen02);
Task Replenish335KvaGen01(100, TASK_FOREVER, &replenish335KvaGen01);
Task Stop335kvaGen01ReplenishmentProcess(1000, TASK_FOREVER, &abort335kvaGen01Replenishment);
Task Replenish335KvaGen02(100, TASK_FOREVER, &replenish335KvaGen02);
Task Stop335kvaGen02ReplenishmentProcess(1000, TASK_FOREVER, &abort335kvaGen02Replenishment);

Task ReplenishGenerator01(1000, TASK_FOREVER, &refillGen01Tank);
Task ReplenishGenerator02(1000, TASK_FOREVER, &refillGen02Tank);
Task ReplenishGenerator03(1000, TASK_FOREVER, &refillGen03Tank);
Task DetectFuelFlowFromDayTank(100, TASK_FOREVER, &ReportFlowDetected);
Task ProcessInputToDayTank(100, TASK_FOREVER, &computeFuelInputToDayTank);
Task ProcessOutputFromDayTank(100, TASK_FOREVER, &computeFuelOutputFromDayTank);
Task ProcessPump01FlowRate(100, TASK_FOREVER, &measureFuelFlowFromPump01);
Task ProcessPump02FlowRate(100, TASK_FOREVER, &measureFuelFlowFromPump02);
Task StartInputFlowComputationToDayTank(100, TASK_FOREVER, &beginFlowComputationToDayTank);
Task StopInputFlowComputationToDayTank(100, TASK_FOREVER, &abortFlowComputationToDayTank);
Task DactivateGen01Replenishment(1000, TASK_FOREVER, &stopGen01Replenishment);
Task DactivateGen02Replenishment(1000, TASK_FOREVER, &stopGen02Replenishment);
Task DactivateGen03Replenishment(1000, TASK_FOREVER, &stopGen03Replenishment);

enum GeneratorFuelTankRefillState {INIT_STATE = 0,
                                   FUEL_REFILL_STARTED = 1,
                                   FUEL_REFILL_IN_PROGRESS = 2, 
                                   FUEL_REFILL_STOPPED = 3,
                                   FUEL_REFILL_COMPLETED = 4,
                                   FUEL_TANK_FULL = 5,
                                   FUEL_REFILL_NOT_IN_PROGRESS = 6
                                  } gen01FuelTankState, gen02FuelTankState, gen03FuelTankState, dayTankState;

struct FuelManagementControl{
    long currentMillis;
    long previousMillis; 
    int flowInterval;
    float calibrationFactor;
    volatile byte pulseCount;
    byte pulse1Sec;
    float flowRate;
    unsigned int flowMilliLitres;
    unsigned long totalMilliLitres;
    int flowSwitchState;
    int previousFlowSwitchState;
    float flowTemp;
    volatile unsigned long start_time_micros;
    volatile unsigned long delayTime;
    int flowVolume;
    int outgoingFlowVolume;
    int outgoingFuelPublishCount;
    int incomingFuelPublishCount;
    int deactivateSignalCount;
    int activateSignalCount;
    int aSignalCount;
    int dSignalCount;
    int maxCount = 2;
}dayTankFuelTank, generator01FuelTankControl, generator02FuelTankControl, generator03FuelTankControl, 
inputFlowMeterControl, outputFlowMeterControl, pump01FlowMeterControl, pump02FlowMeterControl, ps, generator01335kva, generator02335kva, generator335kvaControl;

void inputFlowMeterPulseCounter()
{
  inputFlowMeterControl.pulseCount++;
}

void outputFlowMeterPulseCounter()
{
  outputFlowMeterControl.pulseCount++;
}

void pump01FlowMeterPulseCounter()
{
  pump01FlowMeterControl.pulseCount++;
}

void pump02FlowMeterPulseCounter()
{
  pump02FlowMeterControl.pulseCount++;
}

void gen01pulseCounter()
{
  generator01FuelTankControl.pulseCount++;
}

void gen02pulseCounter()
{
  generator02FuelTankControl.pulseCount++;
}

void gen03pulseCounter()
{
  generator03FuelTankControl.pulseCount++;
}


void gen01335pulseCounter()
{
  generator01335kva.pulseCount++;
}

void gen02335pulseCounter()
{
  generator02335kva.pulseCount++;
}

void resetInputParameters() {
  inputFlowMeterControl.currentMillis = 0;
  inputFlowMeterControl.previousMillis = 0;
  inputFlowMeterControl.flowInterval = 1000;
  inputFlowMeterControl.calibrationFactor = 4.5;
  inputFlowMeterControl.pulse1Sec = 0;
  inputFlowMeterControl.pulseCount = 0;
  inputFlowMeterControl.flowRate = 0.0;
  inputFlowMeterControl.flowMilliLitres = 0;
  inputFlowMeterControl.totalMilliLitres = 0;
  inputFlowMeterControl.previousMillis = 0;

  pump01FlowMeterControl.currentMillis = 0;
  pump01FlowMeterControl.previousMillis = 0;
  pump01FlowMeterControl.flowInterval = 1000;
  pump01FlowMeterControl.calibrationFactor = 4.5;
  pump01FlowMeterControl.pulse1Sec = 0;
  pump01FlowMeterControl.pulseCount = 0;
  pump01FlowMeterControl.flowRate = 0.0;
  pump01FlowMeterControl.flowMilliLitres = 0;
  pump01FlowMeterControl.totalMilliLitres = 0;
  pump01FlowMeterControl.previousMillis = 0;

  pump02FlowMeterControl.currentMillis = 0;
  pump02FlowMeterControl.previousMillis = 0;
  pump02FlowMeterControl.flowInterval = 1000;
  pump02FlowMeterControl.calibrationFactor = 4.5;
  pump02FlowMeterControl.pulse1Sec = 0;
  pump02FlowMeterControl.pulseCount = 0;
  pump02FlowMeterControl.flowRate = 0.0;
  pump02FlowMeterControl.flowMilliLitres = 0;
  pump02FlowMeterControl.totalMilliLitres = 0;
  pump02FlowMeterControl.previousMillis = 0;
}


void resetOutputParameters() {
  outputFlowMeterControl.currentMillis = 0;
  outputFlowMeterControl.previousMillis = 0;
  outputFlowMeterControl.flowInterval = 1000;
  outputFlowMeterControl.calibrationFactor = 4.5;
  outputFlowMeterControl.pulse1Sec = 0;
  outputFlowMeterControl.pulseCount = 0;
  outputFlowMeterControl.flowRate = 0.0;
  outputFlowMeterControl.flowMilliLitres = 0;
  outputFlowMeterControl.totalMilliLitres = 0;
  outputFlowMeterControl.previousMillis = 0;
}

void resetGen01ReplenishmentParameter() {
  generator01FuelTankControl.currentMillis = 0;
  generator01FuelTankControl.previousMillis = 0;
  generator01FuelTankControl.flowInterval = 1000;
  generator01FuelTankControl.calibrationFactor = 4.5;
  generator01FuelTankControl.pulse1Sec = 0;
  generator01FuelTankControl.pulseCount = 0;
  generator01FuelTankControl.flowRate = 0.0;
  generator01FuelTankControl.flowMilliLitres = 0;
  generator01FuelTankControl.totalMilliLitres = 0;
  generator01FuelTankControl.previousMillis = 0;
}


void resetGen02ReplenishmentParameter() {
  generator02FuelTankControl.currentMillis = 0;
  generator02FuelTankControl.previousMillis = 0;
  generator02FuelTankControl.flowInterval = 1000;
  generator02FuelTankControl.calibrationFactor = 4.5;
  generator02FuelTankControl.pulse1Sec = 0;
  generator02FuelTankControl.pulseCount = 0;
  generator02FuelTankControl.flowRate = 0.0;
  generator02FuelTankControl.flowMilliLitres = 0;
  generator02FuelTankControl.totalMilliLitres = 0;
  generator02FuelTankControl.previousMillis = 0;
}

void resetGen03ReplenishmentParameter() {
  generator03FuelTankControl.currentMillis = 0;
  generator03FuelTankControl.previousMillis = 0;
  generator03FuelTankControl.flowInterval = 1000;
  generator03FuelTankControl.calibrationFactor = 4.5;
  generator03FuelTankControl.pulse1Sec = 0;
  generator03FuelTankControl.pulseCount = 0;
  generator03FuelTankControl.flowRate = 0.0;
  generator03FuelTankControl.flowMilliLitres = 0;
  generator03FuelTankControl.totalMilliLitres = 0;
  generator03FuelTankControl.previousMillis = 0;
}
#endif //_DECLARATIONS_