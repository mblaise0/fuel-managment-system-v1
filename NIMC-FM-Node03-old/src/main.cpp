#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <AsyncDelay.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define _TASK_SLEEP_ON_IDLE_RUN
#define _TASK_STATUS_REQUEST
#include <TaskScheduler.h>

#define FLOWMETER_DISCHARGE_PUMP01 33
#define FLOWMETER_DISCHARGE_PUMP02 34
#define TEMPERATURE_SENSOR 35
#define INPUT_TO_DAY_TANKFLOW_METER 32
#define OUTPUT_FROM_DAY_TANK_FLOWMETER 27 
#define OUTPUT_FROM_DAY_TANK_FLOW_SWITCH 26
#define ELECTRIC_PUMP 25

#define CAT_GEN_01_SOLENOID_VALVE 23
#define CAT_GEN_01_FLOW_METER 21
#define CAT_GEN_01_TANK_GUAGE 19

#define CAT_GEN_02_SOLENOID_VALVE 5
#define CAT_GEN_02_FLOW_METER 4
#define CAT_GEN_02_TANK_GUAGE 2

#define CAT_GEN_03_SOLENOID_VALVE 22
#define CAT_GEN_03_FLOW_METER 28
#define CAT_GEN_03_TANK_GUAGE 36

OneWire oneWire(TEMPERATURE_SENSOR);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature flowTemperature(&oneWire);

byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 2, 59);
IPAddress mqttServerIp(192, 168, 2, 44);

const short mqttServerPort = 11883; // IP port of the MQTT broker
const char* mqttClientName = "FM-Node03";
 
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

Scheduler userScheduler; 

long lastReconnectAttempt = 0;

#include "./Declaration.h"
#include "./GeneratorFuelTankControl.h"
#include "./DayTankFuelAnalysis.h"
#include "./Generator01FuelHysteresis.h"
#include "./Generator02FuelHysteresis.h"
#include "./Generator03FuelHysteresis.h"

void callback(char* topic, byte* payload, unsigned int length) {
  String inValue;
  String topicStr = topic;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  char payloadString[length+1];
  memcpy(payloadString, payload, length);
  payloadString[length] = '\0';
  inValue = payloadString;
  Serial.println();  
  reciever(inValue, topicStr);
}


boolean reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(mqttClientName)) {
      Serial.println("connected");
      mqttClient.publish("outTopic","FM-Node03 connected");
      mqttClient.subscribe("FM-Node03", 1);

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


void setup()
{
  Serial.begin(115200);
  flowTemperature.begin();
  mqttClient.setServer(mqttServerIp, mqttServerPort);
  mqttClient.setCallback(callback);
  Ethernet.begin(mac, ip);

  int actuators[] = {FLOWMETER_DISCHARGE_PUMP01, FLOWMETER_DISCHARGE_PUMP02, ELECTRIC_PUMP, CAT_GEN_01_SOLENOID_VALVE, CAT_GEN_02_SOLENOID_VALVE, CAT_GEN_03_SOLENOID_VALVE};
  for (int i = 0; i < 6; i++){
    pinMode(actuators[i], OUTPUT);
  }
    
  int interrupts[] = {TEMPERATURE_SENSOR, INPUT_TO_DAY_TANKFLOW_METER, OUTPUT_FROM_DAY_TANK_FLOWMETER, OUTPUT_FROM_DAY_TANK_FLOW_SWITCH, CAT_GEN_01_FLOW_METER, CAT_GEN_02_FLOW_METER, CAT_GEN_03_FLOW_METER, CAT_GEN_01_TANK_GUAGE, CAT_GEN_02_TANK_GUAGE, CAT_GEN_03_TANK_GUAGE};
  for (int i = 0; i < 7; i++){
    pinMode(interrupts[i], INPUT);
  } 
  
  userScheduler.addTask(detectFuelFlowFromDayTank);
  detectFuelFlowFromDayTank.enable();
  userScheduler.addTask(StopInputFlowComputationToDayTank);
  userScheduler.addTask(StartInputFlowComputationToDayTank);
  userScheduler.addTask(ProcessInputToDayTank);
  userScheduler.addTask(ProcessOutputToDayTank);
  userScheduler.addTask(ProcessPump01FlowRate);
  userScheduler.addTask(ProcessPump02FlowRate);
  userScheduler.addTask(ReplenishGenerator01);
  userScheduler.addTask(ReplenishGenerator02);
  userScheduler.addTask(ReplenishGenerator03);

  userScheduler.addTask(Generator01FuelGaugeRuntime);
  Generator01FuelGaugeRuntime.enable();
  
  userScheduler.addTask(Generator02FuelGaugeRuntime);
  Generator02FuelGaugeRuntime.enable();
  
  userScheduler.addTask(Generator03FuelGaugeRuntime);
  Generator03FuelGaugeRuntime.enable();

  userScheduler.addTask(monitorDayTankFuelLevel);
  monitorDayTankFuelLevel.enable();

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
  attachInterrupt(digitalPinToInterrupt(INPUT_TO_DAY_TANKFLOW_METER), inputFlowMeterPulseCounter, FALLING);

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
  attachInterrupt(digitalPinToInterrupt(OUTPUT_FROM_DAY_TANK_FLOWMETER), outputFlowMeterPulseCounter, FALLING);


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
  attachInterrupt(digitalPinToInterrupt(FLOWMETER_DISCHARGE_PUMP01), pump01FlowMeterPulseCounter, FALLING);

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
  attachInterrupt(digitalPinToInterrupt(FLOWMETER_DISCHARGE_PUMP02), pump02FlowMeterPulseCounter, FALLING);

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
  attachInterrupt(digitalPinToInterrupt(CAT_GEN_01_FLOW_METER), gen01pulseCounter, FALLING);

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
  attachInterrupt(digitalPinToInterrupt(CAT_GEN_02_FLOW_METER), gen02pulseCounter, FALLING);

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
  attachInterrupt(digitalPinToInterrupt(CAT_GEN_03_FLOW_METER), gen03pulseCounter, FALLING);

  dayTankFuelTank.flowSwitchState = 0;
  dayTankFuelTank.previousFlowSwitchState = 0;
  generator01FuelTankControl.fuelMaxCount = 0;
  generator01FuelTankControl.fuelMinCount = 0;
  generator01FuelTankControl.previousGenFuelLevelReading = 0;

  generator02FuelTankControl.fuelMaxCount = 0;
  generator02FuelTankControl.fuelMinCount = 0;
  generator02FuelTankControl.previousGenFuelLevelReading = 0;

  generator03FuelTankControl.fuelMaxCount = 0;
  generator03FuelTankControl.fuelMinCount = 0;
  generator03FuelTankControl.previousGenFuelLevelReading = 0;

  dayTankFuelTank.fuelLevelResult = 0;
  dayTankFuelTank.previousFuelLevelResult = 0;
  dayTankFuelTank.DTMAXCount = 0;
  dayTankFuelTank.DTMINCount = 0;
  dayTankFuelTank.DTACount = 0;
  lastReconnectAttempt = 0;
}

void loop() {
  if (!mqttClient.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
    reconnect();
  } else {
      mqttClient.loop();
      userScheduler.execute();
  }
}