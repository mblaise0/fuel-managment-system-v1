
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
void processGenerator01FuelHysteresis();
void processGenerator02FuelHysteresis();
void processGenerator03FuelHysteresis();
void measureDayTankFuelLevel();

Task monitorDayTankFuelLevel(1000, TASK_FOREVER, &measureDayTankFuelLevel);
Task Generator01FuelGaugeRuntime(100, TASK_FOREVER, &processGenerator01FuelHysteresis);
Task Generator02FuelGaugeRuntime(100, TASK_FOREVER, &processGenerator02FuelHysteresis);
Task Generator03FuelGaugeRuntime(100, TASK_FOREVER, &processGenerator03FuelHysteresis);
Task ReplenishGenerator01(1000, TASK_FOREVER, &refillGen01Tank);
Task ReplenishGenerator02(1000, TASK_FOREVER, &refillGen02Tank);
Task ReplenishGenerator03(1000, TASK_FOREVER, &refillGen03Tank);
Task detectFuelFlowFromDayTank(100, TASK_FOREVER, &ReportFlowDetected);
Task ProcessInputToDayTank(100, TASK_FOREVER, &computeFuelInputToDayTank);
Task ProcessOutputToDayTank(100, TASK_FOREVER, &computeFuelOutputFromDayTank);
Task ProcessPump01FlowRate(100, TASK_FOREVER, &measureFuelFlowFromPump01);
Task ProcessPump02FlowRate(100, TASK_FOREVER, &measureFuelFlowFromPump02);
Task StartInputFlowComputationToDayTank(100, TASK_FOREVER, &beginFlowComputationToDayTank);
Task StopInputFlowComputationToDayTank(100, TASK_FOREVER, &abortFlowComputationToDayTank);

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
    int genfuelLevelReport;
    int genFuelLevelReading;
    int previousGenFuelLevelReading;
    int fuelMaxCount;
    int fuelMinCount;
    int fuelLevelResult;
    int fuelLevelReport;
    int previousFuelLevelResult;
    int DTMAXCount;
    int DTMINCount;
    int DTACount;
    float flowTemp;
    volatile unsigned long start_time_micros;
    volatile unsigned long delayTime;
}dayTankFuelTank, generator01FuelTankControl, generator02FuelTankControl, generator03FuelTankControl, 
inputFlowMeterControl, outputFlowMeterControl, pump01FlowMeterControl, pump02FlowMeterControl;

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


#endif //_DECLARATIONS_