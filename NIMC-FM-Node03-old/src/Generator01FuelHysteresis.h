

#ifndef _GENERATOR_01_FUEL_TANK_
#define _GENERATOR_01_FUEL_TANK_


class Generator01FuelHysteresis : DayTankFuelAnalysis{
    private:
      int DayTankFuelGuage;

    public:
     void computeGenerator01FuelTankLevel();
     void activateSolenoidValve(int);
     void deactivateSolenoidValve(int);
     void startGen01FuelHysteresis();
     void startGen01TankFuelReplenishment();
     void setDayTankFuelGaugeLevel(int dayTankFuelGuage) {
         DayTankFuelGuage = dayTankFuelGuage;
     }

     int getDayTankFuelGuageLevel() {
         return DayTankFuelGuage;
     }

};

void Generator01FuelHysteresis :: computeGenerator01FuelTankLevel() {
    char data[1050];
    StaticJsonDocument<1050> doc;
    generator01FuelTankControl.genfuelLevelReport = getDayTankFuelGuageLevel();
    if (generator01FuelTankControl.genfuelLevelReport >= MAXIMUM_FUEL_LEVEL) {
        gen01FuelTankState = FUEL_TANK_FULL;
        generator01FuelTankControl.fuelMaxCount++;
        if (generator01FuelTankControl.fuelMaxCount == 1){
            deactivateOutputPumpFromDayTank(ELECTRIC_PUMP);
            deactivateSolenoidValve(CAT_GEN_01_SOLENOID_VALVE);
            String payload = "Gen01 Tank: 100%";       //inform FM-node03
            payload.toCharArray(data, (payload.length() + 1));
            mqttClient.publish("FM-SYS", data);
            ReplenishGenerator01.disable();
            generator01FuelTankControl.fuelMinCount = 0;
            Serial.println("Gen01FuelLevel: Maximum...");
        }
    }else {
        gen01FuelTankState = FUEL_REFILL_STARTED;
        if(generator01FuelTankControl.genfuelLevelReport <= 60){
            generator01FuelTankControl.fuelMinCount++;
            if (generator01FuelTankControl.fuelMinCount == 1){
                Serial.println("Gen01FuelLevel: Minimum, Replenishment");
                activateSolenoidValve(CAT_GEN_01_SOLENOID_VALVE);
                activateOutputPumpFromDayTank(ELECTRIC_PUMP);
                ReplenishGenerator01.enable();
                String payload = "Gen01 Tank is Low; Replenishment: Activated";       //inform FM-node03
                payload.toCharArray(data, (payload.length() + 1));
                mqttClient.publish("FM-SYS", data);
                generator01FuelTankControl.fuelMaxCount = 0;
            } 
        }       
    }
}

void Generator01FuelHysteresis :: activateSolenoidValve(int sensor) {
    digitalWrite(sensor, ACTIVATE);
}

void Generator01FuelHysteresis :: deactivateSolenoidValve(int sensor) {
    digitalWrite(sensor, DEACTIVATE);
}


void refillGen01Tank(){
    Generator01FuelHysteresis Generator01FuelHysteresis;
    Generator01FuelHysteresis.startGen01TankFuelReplenishment();
}

void Generator01FuelHysteresis :: startGen01TankFuelReplenishment() {
  char data1[750];
  StaticJsonDocument<1024> doc;
  gen01FuelTankState = FUEL_REFILL_IN_PROGRESS;
  generator01FuelTankControl.currentMillis = millis();
  if (generator01FuelTankControl.currentMillis - generator01FuelTankControl.previousMillis > generator01FuelTankControl.flowInterval) {
    
    generator01FuelTankControl.pulse1Sec = generator01FuelTankControl.pulseCount;
    generator01FuelTankControl.pulseCount = 0;
    generator01FuelTankControl.flowRate = ((1000.0 / (millis() - generator01FuelTankControl.previousMillis)) * generator01FuelTankControl.pulse1Sec) / generator01FuelTankControl.calibrationFactor;
    generator01FuelTankControl.previousMillis = millis();
    generator01FuelTankControl.flowMilliLitres = (generator01FuelTankControl.flowRate / 60) * 1000;
    generator01FuelTankControl.totalMilliLitres += generator01FuelTankControl.flowMilliLitres;
    
    Serial.print("Generator01======>>>Flow rate: ");
    Serial.print(int(generator01FuelTankControl.flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

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
    mqttClient.publish("FM-SYS", data1);
  }
}

void processGenerator01FuelHysteresis() {
    Generator01FuelHysteresis Generator01FuelHysteresis;
    char data[750];
    StaticJsonDocument<750> doc;

    generator01FuelTankControl.genFuelLevelReading = 94;
    if (generator01FuelTankControl.genFuelLevelReading != generator01FuelTankControl.previousGenFuelLevelReading) {
        gen01FuelTankState = FUEL_REFILL_NOT_IN_PROGRESS;
        Serial.println("Gen01FuelTankGauge: " +(String)generator01FuelTankControl.genFuelLevelReading);
        doc["sensor"] = "Gen01TankGauge";
        doc["vol"] = generator01FuelTankControl.genFuelLevelReading;

        String payload = doc.as<String>();
        payload.toCharArray(data, (payload.length() + 1));
        mqttClient.publish("FM-gen01", data); 
        Generator01FuelHysteresis.setDayTankFuelGaugeLevel(generator01FuelTankControl.genFuelLevelReading);
        Generator01FuelHysteresis.computeGenerator01FuelTankLevel();
        generator01FuelTankControl.delayTime = 2000;
        generator01FuelTankControl.start_time_micros = millis();  
        while ((millis() - generator01FuelTankControl.start_time_micros) < generator01FuelTankControl.delayTime){
            ;
        }
    }
    generator01FuelTankControl.previousGenFuelLevelReading = generator01FuelTankControl.genFuelLevelReading;
}


#endif