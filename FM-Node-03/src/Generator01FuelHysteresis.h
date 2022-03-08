

#ifndef _GENERATOR_01_FUEL_TANK_
#define _GENERATOR_01_FUEL_TANK_


class Generator01FuelHysteresis : DayTankFuelAnalysis{
    private:
      int DayTankFuelGuage;

    public:
     void activateSolenoidValve(int);
     void deactivateSolenoidValve(int);
     void startGen01FuelHysteresis();
     void startGen01TankFuelReplenishment();
};


void Generator01FuelHysteresis :: activateSolenoidValve(int sensor) {
    digitalWrite(sensor, ACTIVATE);
}

void Generator01FuelHysteresis :: deactivateSolenoidValve(int sensor) {
    digitalWrite(sensor, DEACTIVATE);
}


void Generator01FuelHysteresis :: startGen01TankFuelReplenishment() {
  char data1[1050];  char data[750];
  StaticJsonDocument<1024> doc;
  gen01FuelTankState = FUEL_REFILL_IN_PROGRESS;
  generator01FuelTankControl.currentMillis = millis();
  if (generator01FuelTankControl.currentMillis - generator01FuelTankControl.previousMillis > generator01FuelTankControl.flowInterval) {
    generator01FuelTankControl.incomingFuelPublishCount++;
    generator01FuelTankControl.pulse1Sec = generator01FuelTankControl.pulseCount;
    generator01FuelTankControl.pulseCount = 0;
    generator01FuelTankControl.flowRate = ((1000.0 / (millis() - generator01FuelTankControl.previousMillis)) * generator01FuelTankControl.pulse1Sec) / generator01FuelTankControl.calibrationFactor;
    generator01FuelTankControl.previousMillis = millis();
    generator01FuelTankControl.flowMilliLitres = (generator01FuelTankControl.flowRate / 60) * 1000;
    generator01FuelTankControl.totalMilliLitres += generator01FuelTankControl.flowMilliLitres;
    generator01FuelTankControl.flowVolume = generator01FuelTankControl.totalMilliLitres / 1000;
    Serial.print("Generator01======>>>Flow rate: ");
    Serial.print(int(generator01FuelTankControl.flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");

    // Print the cumulative total of litres flowed since starting
    Serial.print("Liquid Quantity: ");
    Serial.print(generator01FuelTankControl.totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(generator01FuelTankControl.totalMilliLitres / 1000);
    Serial.println("L");

    doc["source"] = "Gen01TankReplenishment";
    doc["rate"] = generator01FuelTankControl.flowRate;
    doc["vol"] = generator01FuelTankControl.totalMilliLitres / 1000;
    String payload = doc.as<String>();
    payload.toCharArray(data1, (payload.length() + 1));
    mqttClient.publish("FM-Report", data1);

    if (generator01FuelTankControl.incomingFuelPublishCount == 10) {
      String payload1 = "G01"+(String)generator01FuelTankControl.flowVolume;
      payload1.toCharArray(data, (payload1.length() + 1));
      mqttClient.publish("FMPIN", data, 2);
      generator01FuelTankControl.incomingFuelPublishCount = 0;
    }
  }
}


void beginProcessToReplenishGen01FuelTank() {
  Generator01FuelHysteresis generator01FuelHysteresis;
  generator01FuelHysteresis.activateSolenoidValve(CAT_GEN_01_SOLENOID_VALVE);
  ReplenishGenerator01.enable();   
  ProcessOutputFromDayTank.enable();
  StartProcessToReplenishGen01.disable();
}


void refillGen01Tank(){
    Generator01FuelHysteresis generator01FuelHysteresis;
    generator01FuelHysteresis.startGen01TankFuelReplenishment();
}

void stopGen01Replenishment(){
    Generator01FuelHysteresis generator01FuelHysteresis;
    generator01FuelHysteresis.deactivateSolenoidValve(CAT_GEN_01_SOLENOID_VALVE);
    ReplenishGenerator01.disable();
    ProcessOutputFromDayTank.disable();
    //reset replenishment parameters
    resetGen01ReplenishmentParameter();
    DactivateGen01Replenishment.disable();
}





#endif