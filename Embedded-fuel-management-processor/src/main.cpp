#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <SPI.h>
#define _TASK_SLEEP_ON_IDLE_RUN
#define _TASK_STATUS_REQUEST
#include <TaskScheduler.h>

const char* ssid = "iomariTPLink"; // Wifi SSID
const char* password = "#InspireD#"; // Wifi Password

const char* clientName = "FM-Processor";

long lastReconnectAttempt = 0;

const char* mqtt_server = "192.168.2.44";
IPAddress local_IP(192, 168, 2, 61);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 2);
 
IPAddress subnet(255, 255, 252, 0);

WiFiClient espClient10;
PubSubClient client(espClient10);

Scheduler userScheduler;

char recieveDirective(String value, String topic);

//Global declarations
int _available_fuel_volume_underground_tank01;
int _available_fuel_volume_underground_tank02;
int _available_fuel_volume_day_tank;
int _available_fuel_volume_gen01;
int _available_fuel_volume_gen02;
int _available_fuel_volume_gen03;
int _available_fuel_volume_335kva_01;
int _available_fuel_volume_335kva_02;


struct FuelManagmentController{
    int availableFuelVolume;
    int previousFuelVolume;
    int totalAvailableFuelVolume;
    int currentVolume;
    int totalSupplyFuelVolume;
    int incomingFuelVolume;
    int outgoingFuelVolume;
    int updateTankVolume;
    int genActiveTimeInHours;
    int consumedFuelVolume;
    int totalFuelConsumption;
    String tankName;
    String payload;
    int tank01;
    int tank02;
}underground_tank01, underground_tank02, underground_tank_analysis, day_tank_volumetric_analysis,
 gen01_volumetric_analysis, gen02_volumetric_analysis, gen03_volumetric_analysis, generator_volumetric_analysis, gen01_335kva_volumetric_analysis, gen02_335kva_volumetric_analysis;

#include "./CaliberateTankStartVolume.h"
#include "./ProcessorResponse.h"
#include "./UndergroundTankVolumetricAnalysis.h"
#include "./DayTankVolumetricAnalysis.h"
#include "./GeneratorTankVolumetricAnalysis.h"

UndergroundTankVolumetricAnalysis undergroundTankVolumetricAnalysis;
DayTankVolumetricAnalysis dayTankVolumetricAnalysis;
GeneratorTankVolumetricAnalysis generator01TankVolumetricAnalysis;
GeneratorTankVolumetricAnalysis generator02TankVolumetricAnalysis;
GeneratorTankVolumetricAnalysis generator03TankVolumetricAnalysis;
GeneratorTankVolumetricAnalysis generator335KVA01TankVolumetricAnalysis;
GeneratorTankVolumetricAnalysis generator335KVA02TankVolumetricAnalysis;

// CaliberateTankStartVolume caliberateTankStartVolume;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String inValue;
  String topicStr = topic;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] "); 
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  char payloadString[length + 1];
  memcpy(payloadString, payload, length);
  payloadString[length] = '\0';
  inValue = payloadString;
  Serial.println();
  recieveDirective(inValue, topicStr);
}


boolean reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientName)) {
      Serial.println("connected");
      client.publish("outTopic", "FM-Processor connected");

      client.subscribe("UTank01", 1);
      client.loop();
      client.subscribe("UTank02", 1);
      client.loop();
      client.subscribe("DTank", 1);
      client.loop();
      client.subscribe("GEN01", 1);
      client.loop();
      client.subscribe("GEN02", 1);
      client.loop();
      client.subscribe("335kva01", 1);
      client.loop();
      client.subscribe("335kva02", 1);
      client.loop();
      client.subscribe("FMPOUT", 1);
      client.loop();
      client.subscribe("FMPIN", 1);


    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      vTaskDelay(5000);
    }
  }
}

void setup() { 
  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, 11883);
  client.setCallback(callback);

  underground_tank01.currentVolume = 0;
  underground_tank02.currentVolume = 0;
  day_tank_volumetric_analysis.currentVolume = 0;
  underground_tank01.previousFuelVolume = 0;

  gen01_volumetric_analysis.previousFuelVolume = 0;
  gen02_volumetric_analysis.previousFuelVolume = 0;
  gen03_volumetric_analysis.previousFuelVolume = 0;

  generator_volumetric_analysis.previousFuelVolume = 0;

  userScheduler.addTask(ReportUndergroundTank01VolumetricAnalysis);
  ReportUndergroundTank01VolumetricAnalysis.enable();

  userScheduler.addTask(ReportUndergroundTank02VolumetricAnalysis);
  ReportUndergroundTank02VolumetricAnalysis.enable();

  userScheduler.addTask(ReportDayTankVolumetricAnalysis);
  ReportDayTankVolumetricAnalysis.enable();

  userScheduler.addTask(ReportGenerator01FuelVolume);
  ReportGenerator01FuelVolume.enable();

  userScheduler.addTask(ReportGenerator02FuelVolume);
  ReportGenerator02FuelVolume.enable();

  userScheduler.addTask(ReportGenerator03FuelVolume);
  ReportGenerator03FuelVolume.enable();

  userScheduler.addTask(ParseCommand);
  ParseCommand.enable();

  userScheduler.addTask(UnderTank01ReplenishmentProcess);
  userScheduler.addTask(UnderTank02ReplenishmentProcess);
  userScheduler.addTask(DayTankReplenishmentProcess);
  userScheduler.addTask(Gen01TankReplenishmentProcess);
  userScheduler.addTask(Gen02TankReplenishmentProcess);
  userScheduler.addTask(Gen03TankReplenishmentProcess);
  userScheduler.addTask(TerminateUnderTank01ReplenishmentProcess);
  userScheduler.addTask(TerminateUnderTank02ReplenishmentProcess);
  userScheduler.addTask(TerminateDayTankReplenishmentProcess);
  userScheduler.addTask(TerminateGen01TankReplenishmentProcess);
  userScheduler.addTask(TerminateGen02TankReplenishmentProcess);
  userScheduler.addTask(TerminateGen03TankReplenishmentProcess);
  userScheduler.addTask(TankSelectionForUnderGroundReplenishment);

  lastReconnectAttempt = 0;
}

void loop() {
if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  }else {
    client.loop();
    userScheduler.execute();
  }
 
}

char recieveDirective(String value, String topic) {

    if (topic == "FMPIN")
     {
      if (value.startsWith("UT01R")){
        underground_tank01.incomingFuelVolume = value.substring(5).toInt();
        undergroundTankVolumetricAnalysis.computeIncomingFuelVolumeTank01(underground_tank01.incomingFuelVolume);
      }else if(value.startsWith("UT02R")){ 
        underground_tank02.incomingFuelVolume = value.substring(5).toInt();
        undergroundTankVolumetricAnalysis.computeIncomingFuelVolumeTank02(underground_tank02.incomingFuelVolume);        
      }else if(value.startsWith("DTKR")) {
        day_tank_volumetric_analysis.incomingFuelVolume = value.substring(4).toInt();
        dayTankVolumetricAnalysis.computeIncomingFuelVolume(day_tank_volumetric_analysis.incomingFuelVolume);
      }else if (value.startsWith("G01")) {
        gen01_volumetric_analysis.incomingFuelVolume = value.substring(3).toInt();
        generator01TankVolumetricAnalysis.computeFuelInputToGenerator(gen01_volumetric_analysis.incomingFuelVolume, _available_fuel_volume_gen01, "GEN01");
      }else if (value.startsWith("G02")) {
        gen02_volumetric_analysis.incomingFuelVolume = value.substring(3).toInt();
        generator02TankVolumetricAnalysis.computeFuelInputToGenerator(gen02_volumetric_analysis.incomingFuelVolume, _available_fuel_volume_gen02, "GEN02");
      }else if (value.startsWith("G03")) {
        gen03_volumetric_analysis.incomingFuelVolume = value.substring(3).toInt();
        generator03TankVolumetricAnalysis.computeFuelInputToGenerator(gen03_volumetric_analysis.incomingFuelVolume, _available_fuel_volume_gen03,"GEN03");
      }else if (value.startsWith("335G01")) {
        gen01_335kva_volumetric_analysis.incomingFuelVolume = value.substring(3).toInt();
        generator335KVA01TankVolumetricAnalysis.computeFuelInputToGenerator(gen01_335kva_volumetric_analysis.incomingFuelVolume, _available_fuel_volume_335kva_01,"335KVA01");
      }else if (value.startsWith("335G02")) {
        gen03_volumetric_analysis.incomingFuelVolume = value.substring(3).toInt();
        generator335KVA02TankVolumetricAnalysis.computeFuelInputToGenerator(gen02_335kva_volumetric_analysis.incomingFuelVolume, _available_fuel_volume_335kva_02,"335KVA02");
      }else if (value.startsWith("FDQG1")) {
        // check both underground Tanks and decide which should be replenished ,publish a "S.FVA" to FM-Node01 as response if undergroundTank01 needs replenishment 
        TankSelectionForUnderGroundReplenishment.enable();
      }else if (value.startsWith("FDQB1")){
        
      }else if (value.startsWith("FND0")){
          //publish message Replenishment stopped flow not detected
      }  

    }
    else if (topic == "FMPOUT")
     {
      if(value.startsWith("UT01S")){
        underground_tank01.outgoingFuelVolume = value.substring(5).toInt();
        undergroundTankVolumetricAnalysis.computeOutputFuelVolumeTank01(underground_tank01.outgoingFuelVolume);        
      }else if(value.startsWith("UT02S")){
        underground_tank02.outgoingFuelVolume = value.substring(5).toInt();
        undergroundTankVolumetricAnalysis.computeOutputFuelVolumeTank02(underground_tank02.outgoingFuelVolume); 
      }else if(value.startsWith("DTSR")){
        day_tank_volumetric_analysis.outgoingFuelVolume = value.substring(4).toInt();
        dayTankVolumetricAnalysis.computeOutputFuelVolume(day_tank_volumetric_analysis.outgoingFuelVolume);
      }else if (value.startsWith("OG01")){
        gen01_volumetric_analysis.outgoingFuelVolume = value.substring(4).toInt();
        generator01TankVolumetricAnalysis.computFuelConsumedByGenerator(gen01_volumetric_analysis.outgoingFuelVolume, _available_fuel_volume_gen01, "GTank");
      }else if (value.startsWith("OG02")){
        gen02_volumetric_analysis.outgoingFuelVolume = value.substring(4).toInt();
        generator02TankVolumetricAnalysis.computFuelConsumedByGenerator(gen02_volumetric_analysis.outgoingFuelVolume, _available_fuel_volume_gen02, "UGen02");
      }else if (value.startsWith("OG03")){
        gen03_volumetric_analysis.outgoingFuelVolume = value.substring(4).toInt();
        generator03TankVolumetricAnalysis.computFuelConsumedByGenerator(gen03_volumetric_analysis.outgoingFuelVolume, _available_fuel_volume_gen03, "OGTank");
      }else if (value.startsWith("335KVA01")){
        gen01_335kva_volumetric_analysis.outgoingFuelVolume = value.substring(9).toInt();
        generator335KVA01TankVolumetricAnalysis.computFuelConsumedByGenerator(gen01_335kva_volumetric_analysis.outgoingFuelVolume, _available_fuel_volume_335kva_01, "335KvaG01");
      }else if (value.startsWith("335KVA02")){
        gen01_335kva_volumetric_analysis.outgoingFuelVolume = value.substring(9).toInt();
        generator335KVA02TankVolumetricAnalysis.computFuelConsumedByGenerator(gen02_335kva_volumetric_analysis.outgoingFuelVolume, _available_fuel_volume_335kva_02, "335KvaG02");
      }  
    }
    else if (topic == "UTank01"){
      _available_fuel_volume_underground_tank01 = value.toInt();
    }else if (topic == "UTank02"){
      _available_fuel_volume_underground_tank02 = value.toInt();
    }else if (topic == "DTank") {
      _available_fuel_volume_day_tank = value.toInt();
    }else if (topic == "GEN01") {
      _available_fuel_volume_gen01 = value.toInt();
    }else if (topic == "GEN02"){
      _available_fuel_volume_gen02 = value.toInt();      
    }else if (topic == "GEN03") {
      _available_fuel_volume_gen03 = value.toInt();      
    }else if (topic == "335kva01") {
      _available_fuel_volume_335kva_01 = value.toInt();      
    }else if (topic == "335kva02") {
      _available_fuel_volume_335kva_02 = value.toInt();      
    }
}

