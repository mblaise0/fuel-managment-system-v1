

#ifndef _GENERATOR_03_FUEL_TANK_
#define _GENERATOR_03_FUEL_TANK_


class Generator03FuelHysteresis : DayTankFuelAnalysis{
    private:
      int DayTankFuelGuage;

    public:
     void computeGenerator03FuelTankLevel();
     void activateSolenoidValve(int);
     void deactivateSolenoidValve(int);
     void startGen03FuelHysteresis();
     void startGen03TankFuelReplenishment();
     void setDayTankFuelGaugeLevel(int dayTankFuelGuage) {
         DayTankFuelGuage = dayTankFuelGuage;
     }

     int getDayTankFuelGuageLevel() {
         return DayTankFuelGuage;
     }

};

void Generator03FuelHysteresis :: computeGenerator03FuelTankLevel() {
  char data[1050];
  StaticJsonDocument<1050> doc;
  
    generator03FuelTankControl.genfuelLevelReport = getDayTankFuelGuageLevel();
    if (generator03FuelTankControl.genfuelLevelReport >= MAXIMUM_FUEL_LEVEL) {
        gen03FuelTankState = FUEL_TANK_FULL;
        generator03FuelTankControl.fuelMaxCount++;
        if (generator03FuelTankControl.fuelMaxCount == 1){
            deactivateOutputPumpFromDayTank(ELECTRIC_PUMP);
            deactivateSolenoidValve(CAT_GEN_03_SOLENOID_VALVE);
            String payload = "Gen03 Tank: 100%";       //inform FM-node03
            payload.toCharArray(data, (payload.length() + 1));
            mqttClient.publish("FM-SYS", data);
            ReplenishGenerator03.disable();
            generator03FuelTankControl.fuelMinCount = 0;
            Serial.println("Gen03FuelLevel: Maximum...");
        }
    }else {
        gen03FuelTankState = FUEL_REFILL_STARTED;
        if(generator03FuelTankControl.genfuelLevelReport <= 60){
            generator03FuelTankControl.fuelMinCount++;
            if (generator03FuelTankControl.fuelMinCount == 1){
                Serial.println("Gen03FuelLevel: Minimum, Replenishment");
                activateSolenoidValve(CAT_GEN_03_SOLENOID_VALVE);
                activateOutputPumpFromDayTank(ELECTRIC_PUMP);
                ReplenishGenerator03.enable();
                String payload = "Gen03 Tank is Low; Replenishment: Activated";       //inform FM-node03
                payload.toCharArray(data, (payload.length() + 1));
                mqttClient.publish("FM-SYS", data);
                generator03FuelTankControl.fuelMaxCount = 0;
            } 
        }       
    }
}

void Generator03FuelHysteresis :: activateSolenoidValve(int sensor) {
    digitalWrite(sensor, ACTIVATE);
}

void Generator03FuelHysteresis :: deactivateSolenoidValve(int sensor) {
    digitalWrite(sensor, DEACTIVATE);
}


void refillGen03Tank(){
    Generator03FuelHysteresis Generator03FuelHysteresis;
    Generator03FuelHysteresis.startGen03TankFuelReplenishment();
}

void Generator03FuelHysteresis :: startGen03TankFuelReplenishment() {
  char data1[750];
  StaticJsonDocument<1024> doc;
  gen03FuelTankState = FUEL_REFILL_IN_PROGRESS;
  generator03FuelTankControl.currentMillis = millis();
  if (generator03FuelTankControl.currentMillis - generator03FuelTankControl.previousMillis > generator03FuelTankControl.flowInterval) {
    
    generator03FuelTankControl.pulse1Sec = generator03FuelTankControl.pulseCount;
    generator03FuelTankControl.pulseCount = 0;
    generator03FuelTankControl.flowRate = ((1000.0 / (millis() - generator03FuelTankControl.previousMillis)) * generator03FuelTankControl.pulse1Sec) / generator03FuelTankControl.calibrationFactor;
    generator03FuelTankControl.previousMillis = millis();
    generator03FuelTankControl.flowMilliLitres = (generator03FuelTankControl.flowRate / 60) * 1000;
    generator03FuelTankControl.totalMilliLitres += generator03FuelTankControl.flowMilliLitres;
    
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

    doc["source"] = "Gen03TankReplenishment";
    doc["rate"] = generator03FuelTankControl.flowRate;
    doc["vol"] = generator03FuelTankControl.totalMilliLitres / 1000;
    String payload = doc.as<String>();
    payload.toCharArray(data1, (payload.length() + 1));
    mqttClient.publish("FM-SYS", data1);    
  }
}

void processGenerator03FuelHysteresis() {
    Generator03FuelHysteresis Generator03FuelHysteresis;
    char data[750];
    StaticJsonDocument<750> doc;

    generator03FuelTankControl.genFuelLevelReading = 95;
    if (generator03FuelTankControl.genFuelLevelReading != generator03FuelTankControl.previousGenFuelLevelReading) {
        gen01FuelTankState = FUEL_REFILL_NOT_IN_PROGRESS;
        Serial.println("Gen03FuelTankGauge: " +(String)generator03FuelTankControl.genFuelLevelReading);
        doc["sensor"] = "Gen03TankGauge";
        doc["vol"] = generator03FuelTankControl.genFuelLevelReading;

        String payload = doc.as<String>();
        payload.toCharArray(data, (payload.length() + 1));
        mqttClient.publish("FM-gen03", data); 
        Generator03FuelHysteresis.setDayTankFuelGaugeLevel(generator03FuelTankControl.genFuelLevelReading);
        Generator03FuelHysteresis.computeGenerator03FuelTankLevel();
        generator03FuelTankControl.delayTime = 2000;
        generator03FuelTankControl.start_time_micros = millis();  
        while ((millis() - generator03FuelTankControl.start_time_micros) < generator03FuelTankControl.delayTime){
            ;
        }
    }
    generator03FuelTankControl.previousGenFuelLevelReading = generator03FuelTankControl.genFuelLevelReading;
}





#endif