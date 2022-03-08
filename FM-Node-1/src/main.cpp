// Arduino Version - NIMC FUEL Management System Underground TankFuel 
//Replenishment Control (FM-Node01)

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <AsyncDelay.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define _TASK_SLEEP_ON_IDLE_RUN
#define _TASK_STATUS_REQUEST 
#include <TaskScheduler.h>

#define INCOMER_UNDERGROUND_TANK_FLOW_SWITCH 6
#define INCOMER_PUMP_TO_UNDERGROUND_TANK 4
#define INCOMER_SOLENOID_VAVLE 8
#define INCOMER_TEMPERATURE_SENSOR 12
#define UNDERGROUND_TANK_FLOW_METER 13 
#define FUEL_QUALITY_SENSOR  19
#define TEMPERATURE_SENSOR_AFTER_APPROVAL 23
#define UNDERGROUND_TANK01_SOLENOID_VALVE 25
#define UNDERGROUND_TANK01_FUEL_GUAGE 27
#define EVACUATION_SOLENOID_VALVE 32

byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 2, 60);
IPAddress mqttServerIp(192, 168, 2, 44);

const short mqttServerPort = 11883; // IP port of the MQTT broker
const char* mqttClientName = "FM-Node01";

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

Scheduler userScheduler; 
#define DEBUG 1

OneWire underGroundTemp01(INCOMER_TEMPERATURE_SENSOR);
OneWire underGroundTemp02(TEMPERATURE_SENSOR_AFTER_APPROVAL);

DallasTemperature underGroundTankTemperature01(&underGroundTemp01);
DallasTemperature underGroundTankTemperature02(&underGroundTemp02);

enum FuelTankRefillState {INIT_STATE = 0,
                          FUEL_REFILL_STARTED = 1,
                          FUEL_REFILL_IN_PROGRESS = 2,
                          FUEL_REFILL_STOPPED = 3,
                          FUEL_REFILL_COMPLETED = 4,
                          FUEL_TANK_FULL = 5,
                          FUEL_REFILL_NOT_IN_PROGRESS = 6
                        } undergroundTank01FuelTankState, undergroundTank02FuelTankState;

long lastReconnectAttempt = 0;

struct UnderGroundTankFuelControl {
  int incomerSolenoidValve;
  int outGoingSolenoidVave;
  int tankReplenishmentReport;
  int previousTankReplenishmentReport;
  int replenishCount;
  int stopReplenishCount;
  float tank01Temp;
  float tank02Temp;
  long currentMillis;
  long previousMillis;
  float calibrationFactor;
  volatile byte pulseCount;
  byte pulse1Sec;
  float flowRate;
  unsigned int flowMilliLitres;
  unsigned long totalMilliLitres;
  unsigned long currentRefillVolume;
  int flowInterval;
  int fuelLevelReport;
  int previousFuelLevelReport;
  int fuelLevelReading;
  int qualityReport;
  int fuelQualityTest;
  int fuelQualityTestResult;
  int incomingValves;
  int flowControl;
  int goodFuelCount;
  float postFuelTemp;
  int activateCount;
  int updateServerCount;
  int tank01;
  int tank02;
  volatile unsigned long start_time_micros;
  volatile unsigned long delayTime;
}SolenoidValvesControl, underGroundTankAnalysis;


void undergroundTankPulseCounter()
{
  underGroundTankAnalysis.pulseCount++;
}


#include "./UnderGroundReplenishmentControl.h"

char reciever(String v, String r);

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
      mqttClient.publish("outTopic","FM-Node01 connected");
      mqttClient.subscribe("FM-Node01");
      mqttClient.subscribe("DTK02");
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
  mqttClient.setServer(mqttServerIp, mqttServerPort);
  mqttClient.setCallback(callback);
  Ethernet.begin(mac, ip);

  underGroundTankTemperature01.begin();
  underGroundTankTemperature02.begin();

  int actuatorSensorPins[] = {INCOMER_PUMP_TO_UNDERGROUND_TANK, UNDERGROUND_TANK01_SOLENOID_VALVE, INCOMER_SOLENOID_VAVLE};
  for (int i = 0; i < 3; i++){
    pinMode(actuatorSensorPins[i], OUTPUT);
  }

  int plantInputPins[] = {INCOMER_UNDERGROUND_TANK_FLOW_SWITCH, INCOMER_TEMPERATURE_SENSOR, FUEL_QUALITY_SENSOR, TEMPERATURE_SENSOR_AFTER_APPROVAL, UNDERGROUND_TANK01_FUEL_GUAGE};
  for (int i = 0; i < 5; i++){
     pinMode(plantInputPins[i], INPUT);
  }
  
  pinMode(UNDERGROUND_TANK_FLOW_METER, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(UNDERGROUND_TANK_FLOW_METER), undergroundTankPulseCounter, FALLING);

  underGroundTankAnalysis.replenishCount = 0;
  underGroundTankAnalysis.stopReplenishCount = 0;
  underGroundTankAnalysis.currentMillis = 0;
  underGroundTankAnalysis.previousMillis = 0;
  underGroundTankAnalysis.flowInterval = 1000;
  underGroundTankAnalysis.calibrationFactor = 4.5;
  underGroundTankAnalysis.pulse1Sec = 0;
  underGroundTankAnalysis.pulseCount = 0;
  underGroundTankAnalysis.flowRate = 0.0;
  underGroundTankAnalysis.flowMilliLitres = 0;
  underGroundTankAnalysis.totalMilliLitres = 0;
  underGroundTankAnalysis.previousMillis = 0;
  underGroundTankAnalysis.previousFuelLevelReport = 0;
  underGroundTankAnalysis.previousTankReplenishmentReport = 0;

  userScheduler.addTask(DetectFlow);
  DetectFlow.enable();
  
  userScheduler.addTask(activateElectricPump);
  userScheduler.addTask(activateIncomerSolenoidValve);
  userScheduler.addTask(stopReplenishmentProcess);
  userScheduler.addTask(monitorPostFuelTemperature);
  userScheduler.addTask(StartSupplyFlowMeterAnalysis);
  userScheduler.addTask(TerminateAllUndergroundTanksReplenishMent);
  userScheduler.addTask(StartTank02FlowMeterAnalysis);

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
