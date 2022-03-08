
#ifndef _GENERATOR_FUEL_TANK_CONTROL
#define _GENERATOR_FUEL_TANK_CONTROL

#define ACTIVATE HIGH
#define DEACTIVATE LOW
#define FLOW_DETECTED HIGH

// struct NodeControl{
    volatile int flow_frequency;
    unsigned int l_hour; 
    unsigned long currentTime;
    unsigned long cloopTime;
// }FmNode01;

void flow () // Interrupt function
{
   flow_frequency++;
}
class GeneratorFuelTankControl
{
private:
    int FlowMeterSensor;
    int FlowSwitchSensor;
    int SolenoidValveSensor;

public:
    GeneratorFuelTankControl(int flowMeterSensor, int flowSwitchSensor, int solenoidValveSensor){
        FlowMeterSensor = flowMeterSensor;
        FlowSwitchSensor = flowSwitchSensor;
        SolenoidValveSensor = solenoidValveSensor;
    }

    void activateValve(int);
    void deactivateValve(int);
    void computeReplenishmentVolume();
    void monitorFuelInputToGeneratorTank(int);
};


void GeneratorFuelTankControl :: activateValve(int sensorPin){
    digitalWrite(sensorPin, ACTIVATE);
}

void GeneratorFuelTankControl :: deactivateValve(int sensorPin) {
    digitalWrite(sensorPin, DEACTIVATE);
}

void GeneratorFuelTankControl :: computeReplenishmentVolume() {
    currentTime = millis();
   // Every second, calculate and print litres/hour
   if(currentTime >= (cloopTime + 1000))
   {
      cloopTime = currentTime; // Updates cloopTime
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
      l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
      flow_frequency = 0; // Reset Counter
      Serial.print(l_hour, DEC); // Print litres/hour
      Serial.println(" L/hour");
   }
}

int flowStatus = 0;
int previousFlowStatus = 0;

void GeneratorFuelTankControl::monitorFuelInputToGeneratorTank(int sensor) {
    int flowStatus = digitalRead(sensor);
    if (flowStatus != previousFlowStatus) {
        if (flowStatus == FLOW_DETECTED) {
            Serial.println("Flow Detected!!");
        }else{
            Serial.println("Flow Stopped!!");
        }
    }
    previousFlowStatus = flowStatus;

}

 char reciever(String topic, String inValue){

 }
 
#endif   //_GENERATOR_FUEL_TANK_CONTROL
