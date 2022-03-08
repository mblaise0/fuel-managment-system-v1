

#ifndef _GENERATOR_02_FUEL_TANK_
#define _GENERATOR_02_FUEL_TANK_



class Generator02FuelHysteresis : DayTankFuelAnalysis{
    private:
      int DayTankFuelGuage;

    public:
     void computeGenerator02FuelTankLevel();
     void activateSolenoidValve(int);
     void deactivateSolenoidValve(int);
     void startGen02FuelHysteresis();
     void startGen02TankFuelReplenishment();
     void setDayTankFuelGaugeLevel(int dayTankFuelGuage) {
         DayTankFuelGuage = dayTankFuelGuage;
     }

     int getDayTankFuelGuageLevel() {
         return DayTankFuelGuage;
     }

};

void Generator02FuelHysteresis :: computeGenerator02FuelTankLevel() {
  char data[1050];
  StaticJsonDocument<1050> doc;
  
    generator02FuelTankControl.genfuelLevelReport = getDayTankFuelGuageLevel();
    if (generator02FuelTankControl.genfuelLevelReport >= MAXIMUM_FUEL_LEVEL) {
        gen02FuelTankState = FUEL_TANK_FULL;
        generator02FuelTankControl.fuelMaxCount++;
        if (generator02FuelTankControl.fuelMaxCount == 1){
            deactivateOutputPumpFromDayTank(ELECTRIC_PUMP);
            deactivateSolenoidValve(CAT_GEN_02_SOLENOID_VALVE);
            String payload = "Gen02 Tank: 100%";       //inform FM-node03
            payload.toCharArray(data, (payload.length() + 1));
            mqttClient.publish("FM-SYS", data);
            ReplenishGenerator02.disable();
            generator02FuelTankControl.fuelMinCount = 0;
            Serial.println("Gen02FuelLevel: Maximum...");
        }
    }else {
        gen02FuelTankState = FUEL_REFILL_STARTED;
        if(generator02FuelTankControl.genfuelLevelReport <= 60){
            generator02FuelTankControl.fuelMinCount++;
            if (generator02FuelTankControl.fuelMinCount == 1){
                Serial.println("Gen02FuelLevel: Minimum, Replenishment");
                activateSolenoidValve(CAT_GEN_02_SOLENOID_VALVE);
                activateOutputPumpFromDayTank(ELECTRIC_PUMP);
                ReplenishGenerator02.enable();
                String payload = "Gen02 Tank is Low; Replenishment: Activated";       //inform other Nodes
                payload.toCharArray(data, (payload.length() + 1));
                mqttClient.publish("FM-SYS", data);
                generator02FuelTankControl.fuelMaxCount = 0;
            } 
        }       
    }
}

void Generator02FuelHysteresis :: activateSolenoidValve(int sensor) {
    digitalWrite(sensor, ACTIVATE);
}

void Generator02FuelHysteresis :: deactivateSolenoidValve(int sensor) {
    digitalWrite(sensor, DEACTIVATE);
}


void refillGen02Tank(){
    Generator02FuelHysteresis Generator02FuelHysteresis;
    Generator02FuelHysteresis.startGen02TankFuelReplenishment();
}

void Generator02FuelHysteresis :: startGen02TankFuelReplenishment() {
  char data1[750];
  StaticJsonDocument<1024> doc;
  gen02FuelTankState = FUEL_REFILL_IN_PROGRESS;
  generator02FuelTankControl.currentMillis = millis();
  if (generator02FuelTankControl.currentMillis - generator02FuelTankControl.previousMillis > generator02FuelTankControl.flowInterval) {
    
    generator02FuelTankControl.pulse1Sec = generator02FuelTankControl.pulseCount;
    generator02FuelTankControl.pulseCount = 0;
    generator02FuelTankControl.flowRate = ((1000.0 / (millis() - generator02FuelTankControl.previousMillis)) * generator02FuelTankControl.pulse1Sec) / generator02FuelTankControl.calibrationFactor;
    generator02FuelTankControl.previousMillis = millis();
    generator02FuelTankControl.flowMilliLitres = (generator02FuelTankControl.flowRate / 60) * 1000;
    generator02FuelTankControl.totalMilliLitres += generator02FuelTankControl.flowMilliLitres;
    
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
    
    doc["source"] = "Gen02TankReplenishment";
    doc["rate"] = generator02FuelTankControl.flowRate;
    doc["vol"] = generator02FuelTankControl.totalMilliLitres / 1000;
    String payload = doc.as<String>();
    payload.toCharArray(data1, (payload.length() + 1));
    mqttClient.publish("FM-SYS", data1);
  }
}

void processGenerator02FuelHysteresis() {
    Generator02FuelHysteresis Generator02FuelHysteresis;
    char data[750];
    StaticJsonDocument<750> doc;

    generator02FuelTankControl.genFuelLevelReading = 86;
    if (generator02FuelTankControl.genFuelLevelReading != generator02FuelTankControl.previousGenFuelLevelReading) {
        gen02FuelTankState = FUEL_REFILL_NOT_IN_PROGRESS;
        Serial.println("Gen02FuelTankGauge: " +(String)generator02FuelTankControl.genFuelLevelReading);
        doc["sensor"] = "Gen02TankGauge";
        doc["vol"] = generator02FuelTankControl.genFuelLevelReading;

        String payload = doc.as<String>();
        payload.toCharArray(data, (payload.length() + 1));
        mqttClient.publish("FM-gen02", data); 
        Generator02FuelHysteresis.setDayTankFuelGaugeLevel(generator02FuelTankControl.genFuelLevelReading);
        Generator02FuelHysteresis.computeGenerator02FuelTankLevel();
        generator02FuelTankControl.delayTime = 2000;
        generator02FuelTankControl.start_time_micros = millis();  
        while ((millis() - generator02FuelTankControl.start_time_micros) < generator02FuelTankControl.delayTime){
            ;
        }
    }
    generator02FuelTankControl.previousGenFuelLevelReading = generator02FuelTankControl.genFuelLevelReading;
}



#endif