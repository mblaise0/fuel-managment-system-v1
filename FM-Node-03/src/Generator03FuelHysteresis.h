

#ifndef _GENERATOR_03_FUEL_TANK_
#define _GENERATOR_03_FUEL_TANK_


class Generator03FuelHysteresis : DayTankFuelAnalysis{
    private:
      int DayTankFuelGuage;

    public:
     void activateSolenoidValve(int);
     void deactivateSolenoidValve(int);
     void startGen03FuelHysteresis();
     void startGen03TankFuelReplenishment();

};

void Generator03FuelHysteresis :: activateSolenoidValve(int sensor) {
    digitalWrite(sensor, ACTIVATE);
}

void Generator03FuelHysteresis :: deactivateSolenoidValve(int sensor) {
    digitalWrite(sensor, DEACTIVATE);
}


void Generator03FuelHysteresis :: startGen03TankFuelReplenishment() {
  char data1[750];   char data[750];
  StaticJsonDocument<1024> doc;
  gen03FuelTankState = FUEL_REFILL_IN_PROGRESS;
  generator03FuelTankControl.currentMillis = millis();
  if (generator03FuelTankControl.currentMillis - generator03FuelTankControl.previousMillis > generator03FuelTankControl.flowInterval) {
    generator03FuelTankControl.incomingFuelPublishCount++;
    generator03FuelTankControl.pulse1Sec = generator03FuelTankControl.pulseCount;
    generator03FuelTankControl.pulseCount = 0;
    generator03FuelTankControl.flowRate = ((1000.0 / (millis() - generator03FuelTankControl.previousMillis)) * generator03FuelTankControl.pulse1Sec) / generator03FuelTankControl.calibrationFactor;
    generator03FuelTankControl.previousMillis = millis();
    generator03FuelTankControl.flowMilliLitres = (generator03FuelTankControl.flowRate / 60) * 1000;
    generator03FuelTankControl.totalMilliLitres += generator03FuelTankControl.flowMilliLitres;
    generator03FuelTankControl.flowVolume = generator03FuelTankControl.totalMilliLitres / 1000;

    doc["source"] = "Gen03TankReplenishment";
    doc["rate"] = generator03FuelTankControl.flowRate;
    doc["vol"] = generator03FuelTankControl.totalMilliLitres / 1000;
    String payload = doc.as<String>();
    payload.toCharArray(data1, (payload.length() + 1));
    // mqttClient.publish("FM-Report", data1);  

    if (generator03FuelTankControl.incomingFuelPublishCount == 10) {
      Serial.println("No of count: "+ (String)generator03FuelTankControl.incomingFuelPublishCount);
      String payload1 = "G03"+(String)generator03FuelTankControl.flowVolume;
      payload1.toCharArray(data, (payload1.length() + 1));
      mqttClient.publish("FMPIN", data, 2);
      generator03FuelTankControl.incomingFuelPublishCount = 0;
    }

    Serial.print("Generator03======>>>Flow rate: ");
    Serial.print(int(generator03FuelTankControl.flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Liquid Quantity: ");
    Serial.print(generator03FuelTankControl.totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(generator03FuelTankControl.totalMilliLitres / 1000);
    Serial.println("L");  
  }
}

void beginProcessToReplenishGen03FuelTank() {
  Generator03FuelHysteresis generator03FuelHysteresis;
  generator03FuelHysteresis.activateSolenoidValve(CAT_GEN_03_SOLENOID_VALVE);
  ReplenishGenerator03.enable();   
  ProcessOutputFromDayTank.enable();
  StartProcessToReplenishGen03.disable();
}

void refillGen03Tank(){
    Generator03FuelHysteresis generator03FuelHysteresis;
    generator03FuelHysteresis.startGen03TankFuelReplenishment();
}


void stopGen03Replenishment(){
    Generator03FuelHysteresis generator03FuelHysteresis;
    generator03FuelHysteresis.deactivateSolenoidValve(CAT_GEN_03_SOLENOID_VALVE);
    ReplenishGenerator03.disable();
    //reset replenishment parameters
    resetGen03ReplenishmentParameter();
    ProcessOutputFromDayTank.disable();
    DactivateGen03Replenishment.disable();
}

#endif