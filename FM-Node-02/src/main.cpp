#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <AsyncDelay.h>
#include <ArduinoJson.h>
#include <Wire.h>

#define _TASK_SLEEP_ON_IDLE_RUN
#define _TASK_STATUS_REQUEST
#include <TaskScheduler.h>

#define UNDER_GROUND_TANK01_VALVE_1A 2
#define UNDER_GROUND_TANK01_VALVE_1B 4
#define UNDER_GROUND_TANK02_VALVE_2A 5
#define UNDER_GROUND_TANK02_VALVE_2B 18

#define UNDER_GROUND_TANK_PUMP01 19
#define UNDER_GROUND_TANK_PUMP02 21

#define DISCHARGE_VALVE_FROM_PUMP_01 25 
#define DISCHARGE_VALVE_FROM_PUMP_02 26

#define UNDER_GROUND_TANK02_FUEL_GUAGE 22
#define UNDER_GROUND_TANK_PUMP_01_AUX 33
#define UNDER_GROUND_TANK_PUMP_02_AUX 34

#define EVACUATION_VALVE 27
#define UNDER_GROUND_TANK_02_VALVE 32

Scheduler fuelManagementControl;

byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 2, 58);
IPAddress mqttServerIp(192, 168, 2, 44);

const short mqttServerPort = 11883; // IP port of the MQTT broker
const char* mqttClientName = "FM-Node02";

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

#define DEBUG 1

long lastReconnectAttempt = 0;

#include "./UndergroundTankFuelAnalysis.h"
#include "./PumpSelectionToDayTank.h"
#include "./Interrupt.h"

char parseIncomingCommand(String v, String r);

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
  parseIncomingCommand(inValue, topicStr);
}



boolean reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(mqttClientName)) {
      Serial.println("connected");
      mqttClient.subscribe("FM-Node02", 1);
      mqttClient.subscribe("ADTKR", 1);
      mqttClient.subscribe("DTKR", 1);
      mqttClient.publish("outTopic","FM-Node02 connected");
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

  int actuators[] = {UNDER_GROUND_TANK01_VALVE_1A, UNDER_GROUND_TANK01_VALVE_1B, UNDER_GROUND_TANK02_VALVE_2A, UNDER_GROUND_TANK02_VALVE_2B, EVACUATION_VALVE, UNDER_GROUND_TANK_02_VALVE, UNDER_GROUND_TANK_PUMP01, UNDER_GROUND_TANK_PUMP02, DISCHARGE_VALVE_FROM_PUMP_01, DISCHARGE_VALVE_FROM_PUMP_02};
  for (int i = 0; i < 10; i++){
    pinMode(actuators[i], OUTPUT);
    digitalWrite(actuators[i], LOW);
  }
  
  int interrupts[] = {UNDER_GROUND_TANK02_FUEL_GUAGE, UNDER_GROUND_TANK_PUMP_01_AUX, UNDER_GROUND_TANK_PUMP_02_AUX};
  for (int i = 0; i < 3; i++){
    pinMode(interrupts[i], INPUT);
  }

  fuelManagementControl.addTask(EvacuateFuel);
  fuelManagementControl.addTask(ReportPumpState);
  ReportPumpState.enable();
  fuelManagementControl.addTask(TerminateDayTankReplenishment);
  fuelManagementControl.addTask(ReplenishDayTankWithUndergroundTank01); 
  fuelManagementControl.addTask(ReplenishDayTankWithUndergroundTank02); 
  fuelManagementControl.addTask(ActivateUnderGroundTank02Replenishment);
  fuelManagementControl.addTask(TerminateUndergroundTank02Valve);

  undergroundControl.badFuelQualitySignalCount = 0;
  undergroundControl.stopDayTankReplenishmentSignalCount = 0;
  undergroundControl.stopReplenishmentCount = 0;
  undergroundControl.replenishCount = 0;
  dayTankControl.replenishCount = 0;
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
    fuelManagementControl.execute();
  }
}