
#ifndef _INTERRUPTS
#define _INTERRUPTS

#define ACTIVATE HIGH
#define DEACTIVATE LOW

int pump01AuxState;
int pump02AuxState;


class NodeInterrupts{
    private:
     int Pump01Aux;
     int Pump02Aux;

    public:
     void pump01Aux(int);
     void pump02Aux(int);    
};

void NodeInterrupts :: pump01Aux(int sensor){ 
    char data[1050];
    StaticJsonDocument<750> doc;
    doc["sensor"] = "pump01Aux";
    undergroundControl.pump01AuxSenseAnlaysis = digitalRead(sensor);
    if (undergroundControl.pump01AuxSenseAnlaysis != undergroundControl.previousPump01AuxSenseAnlaysis){
        if (undergroundControl.pump01AuxSenseAnlaysis == ACTIVATE) {
           doc["state"] = undergroundControl.pump01AuxSenseAnlaysis;
        }else{
           doc["state"] = undergroundControl.pump01AuxSenseAnlaysis;
        }
        String payload = doc.as<String>();
        payload.toCharArray(data, (payload.length() + 1));
        mqttClient.publish("FM-report", data);
    }
    undergroundControl.previousPump01AuxSenseAnlaysis = undergroundControl.pump01AuxSenseAnlaysis;
}

void NodeInterrupts :: pump02Aux(int sensor){
    char data[1050];
    StaticJsonDocument<750> doc;
    doc["sensor"] = "pump02Aux";
    undergroundControl.pump02AuxSenseAnlaysis = digitalRead(sensor);
    if (undergroundControl.pump02AuxSenseAnlaysis != undergroundControl.previousPump02AuxSenseAnlaysis){
        if (undergroundControl.pump02AuxSenseAnlaysis == ACTIVATE) {
           doc["state"] = undergroundControl.pump02AuxSenseAnlaysis;
        }else{
           doc["state"] = undergroundControl.pump02AuxSenseAnlaysis;
        }
        String payload = doc.as<String>();
        payload.toCharArray(data, (payload.length() + 1));
        mqttClient.publish("FM-report", data);
    }
    undergroundControl.previousPump02AuxSenseAnlaysis = undergroundControl.pump02AuxSenseAnlaysis;
}


void moinitorPumpState(){
    NodeInterrupts nodeInterrupts;
    nodeInterrupts.pump01Aux(UNDER_GROUND_TANK_PUMP_01_AUX);
    nodeInterrupts.pump02Aux(UNDER_GROUND_TANK_PUMP_02_AUX);
}

#endif