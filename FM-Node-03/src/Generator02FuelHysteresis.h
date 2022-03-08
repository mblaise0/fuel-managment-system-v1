

#ifndef _GENERATOR_02_FUEL_TANK_
#define _GENERATOR_02_FUEL_TANK_



class Generator02FuelHysteresis : DayTankFuelAnalysis{
    private:
      int DayTankFuelGuage;

    public:
     void activateSolenoidValve(int);
     void deactivateSolenoidValve(int);
     void startGen02FuelHysteresis();
     void startGen02TankFuelReplenishment();

};


void Generator02FuelHysteresis :: activateSolenoidValve(int sensor) {
    digitalWrite(sensor, ACTIVATE);
}

void Generator02FuelHysteresis :: deactivateSolenoidValve(int sensor) {
    digitalWrite(sensor, DEACTIVATE);
}

void Generator02FuelHysteresis :: startGen02TankFuelReplenishment() {
  char data1[1050];   char data[750];
  StaticJsonDocument<1024> doc;
  gen02FuelTankState = FUEL_REFILL_IN_PROGRESS;
  generator02FuelTankControl.currentMillis = millis();
  if (generator02FuelTankControl.currentMillis - generator02FuelTankControl.previousMillis > generator02FuelTankControl.flowInterval) {
    generator02FuelTankControl.incomingFuelPublishCount++;
    generator02FuelTankControl.pulse1Sec = generator02FuelTankControl.pulseCount;
    generator02FuelTankControl.pulseCount = 0;
    generator02FuelTankControl.flowRate = ((1000.0 / (millis() - generator02FuelTankControl.previousMillis)) * generator02FuelTankControl.pulse1Sec) / generator02FuelTankControl.calibrationFactor;
    generator02FuelTankControl.previousMillis = millis();
    generator02FuelTankControl.flowMilliLitres = (generator02FuelTankControl.flowRate / 60) * 1000;
    generator02FuelTankControl.totalMilliLitres += generator02FuelTankControl.flowMilliLitres;
    generator01FuelTankControl.flowVolume = generator02FuelTankControl.totalMilliLitres / 1000;
   
    doc["source"] = "Gen02TankReplenishment";
    doc["rate"] = generator02FuelTankControl.flowRate;
    doc["vol"] = generator02FuelTankControl.totalMilliLitres / 1000;
    String payload = doc.as<String>();
    payload.toCharArray(data1, (payload.length() + 1));
    mqttClient.publish("FM-Report", data1);

    if (generator02FuelTankControl.incomingFuelPublishCount == 10) {
      String payload1 = "G02"+(String)generator01FuelTankControl.flowVolume;
      payload1.toCharArray(data, (payload1.length() + 1));
      mqttClient.publish("FMPIN", data, 2);
      generator02FuelTankControl.incomingFuelPublishCount = 0;
    }

    Serial.print("Generator02======>>>Flow rate: ");
    Serial.print(int(generator02FuelTankControl.flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Liquid Quantity: ");
    Serial.print(generator02FuelTankControl.totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(generator02FuelTankControl.totalMilliLitres / 1000);
    Serial.println("L");
  }
}


void beginProcessToReplenishGen02FuelTank() {
  Generator02FuelHysteresis generator02FuelHysteresis;
  generator02FuelHysteresis.activateSolenoidValve(CAT_GEN_02_SOLENOID_VALVE);
  ReplenishGenerator02.enable(); 
  ProcessOutputFromDayTank.enable();  
  StartProcessToReplenishGen02.disable();
}

void refillGen02Tank(){
    Generator02FuelHysteresis generator02FuelHysteresis;
    generator02FuelHysteresis.startGen02TankFuelReplenishment();
}


void stopGen02Replenishment(){
    Generator02FuelHysteresis generator02FuelHysteresis;
    generator02FuelHysteresis.deactivateSolenoidValve(CAT_GEN_02_SOLENOID_VALVE);
    ReplenishGenerator02.disable();
    //reset replenishment parameters
    ProcessOutputFromDayTank.disable();
    resetGen02ReplenishmentParameter();
    DactivateGen02Replenishment.disable();
}

#endif