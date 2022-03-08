
#ifndef _UNDER_GROUND_TANK_VOLUMETRIC_ANALYSIS
#define _UNDER_GROUND_TANK_VOLUMETRIC_ANALYSIS

#define UNDERGROUND_TANK_VOL 45000
#define UNDERGROUND_TANK_MAX_VOL 42000
#define UNDERGROUND_TANK_MIN_VOL 12000
#define UNDERGROUND_TANK_AVG_VOL 33000
#define UNDERGROUND_TANK_EMERGENCY_ALLOWED_VOL 8500

void undergroundTank01VolumeRunTime();
void undergroundTank02VolumeRunTime();
void tankSelectionForUnderGroundReplenishmentRuntime();

Task TankSelectionForUnderGroundReplenishment(1000, TASK_FOREVER, &tankSelectionForUnderGroundReplenishmentRuntime);
Task ReportUndergroundTank01VolumetricAnalysis(1000, TASK_FOREVER, &undergroundTank01VolumeRunTime);
Task ReportUndergroundTank02VolumetricAnalysis(1000, TASK_FOREVER, &undergroundTank02VolumeRunTime);

class UndergroundTankVolumetricAnalysis{
    protected:
        int IncomingVolume;
        int OutgoingVolume;

    public:
        char data[2500];
        StaticJsonDocument<2500> doc;
        
        void computeIncomingFuelVolumeTank01(int incomingVolume) {
            underground_tank01.totalAvailableFuelVolume = incomingVolume + _available_fuel_volume_underground_tank01;
            _available_fuel_volume_underground_tank01 = underground_tank01.totalAvailableFuelVolume;
            String payload = (String)_available_fuel_volume_underground_tank01;  
            payload.toCharArray(data, (payload.length() + 1));
            client.publish("UTank01", data, true);
            Serial.println("Incoming-Volume: "+ (String)incomingVolume + "   CurrentFuelVolume: " + (String)_available_fuel_volume_underground_tank01);
        }

        void computeOutputFuelVolumeTank01(int outgoingVolume) {
            underground_tank01.totalSupplyFuelVolume = _available_fuel_volume_underground_tank01 - outgoingVolume;
            _available_fuel_volume_underground_tank01 = underground_tank01.totalSupplyFuelVolume;
            String payload = (String)_available_fuel_volume_underground_tank01;  
            payload.toCharArray(data, (payload.length() + 1));
            client.publish("UTank01", data, true);
            Serial.println("Outgoing-Volume :"+ (String)outgoingVolume + "   CurrentFuelVolume: " + (String)_available_fuel_volume_underground_tank01);
        }

        void computeIncomingFuelVolumeTank02(int incomingVolume) {
            underground_tank02.totalAvailableFuelVolume = incomingVolume + _available_fuel_volume_underground_tank02;
            _available_fuel_volume_underground_tank02 = underground_tank02.totalAvailableFuelVolume;
            String payload = (String)_available_fuel_volume_underground_tank02;  
            payload.toCharArray(data, (payload.length() + 1));
            client.publish("UTank02", data, true);
            Serial.println("Incoming-Volume: "+ (String)incomingVolume + "   CurrentFuelVolume: " + (String)_available_fuel_volume_underground_tank02);
        }

        void computeOutputFuelVolumeTank02(int outgoingVolume) {
            underground_tank02.totalSupplyFuelVolume = _available_fuel_volume_underground_tank02 - outgoingVolume;
            _available_fuel_volume_underground_tank02 = underground_tank02.totalSupplyFuelVolume;
            String payload = (String)_available_fuel_volume_underground_tank02;  
            payload.toCharArray(data, (payload.length() + 1));
            client.publish("UTank02", data, true);
            Serial.println("Incoming-Volume: "+ (String)outgoingVolume + "   CurrentFuelVolume: " + (String)_available_fuel_volume_underground_tank02);
        }

        void selectTankToReplenish() {
            
            underground_tank_analysis.tank01 = _available_fuel_volume_underground_tank01;
            underground_tank_analysis.tank02 = _available_fuel_volume_underground_tank02;
            if ((underground_tank_analysis.tank01 <= UNDERGROUND_TANK_MIN_VOL) && (underground_tank_analysis.tank02 <= UNDERGROUND_TANK_MIN_VOL)) {
                underground_tank_analysis.payload = "underground-tank01-selected";
                underground_tank_analysis.payload.toCharArray(data, (underground_tank_analysis.payload.length() + 1));
                client.publish("FM-Report", data, 2);
                UnderTank01ReplenishmentProcess.enable();
                Serial.println("c1: underground-tank01-selected");
            }else if ((underground_tank_analysis.tank01 >= UNDERGROUND_TANK_AVG_VOL) && (underground_tank_analysis.tank02 < UNDERGROUND_TANK_MIN_VOL)) {
                UnderTank02ReplenishmentProcess.enable();
                underground_tank_analysis.payload = "A.UTR";
                underground_tank_analysis.payload.toCharArray(data, (underground_tank_analysis.payload.length() + 1));
                for (int i = 0; i < 3; i++){
                    client.publish("FM-Node01", data, 2);
                }
                Serial.println("c2: underground-tank02-selected");
            }else if ((underground_tank_analysis.tank01 >= UNDERGROUND_TANK_MAX_VOL) && (underground_tank_analysis.tank02 < UNDERGROUND_TANK_AVG_VOL)) {
                underground_tank_analysis.payload = "A.UT02V";
                underground_tank_analysis.payload.toCharArray(data, (underground_tank_analysis.payload.length() + 1));
                for (int i = 0; i < 3; i++){
                    client.publish("FM-Node02", data, 2);
                }
                Serial.println("c3: underground-tank02-selected");
            }else if ((underground_tank_analysis.tank01 < UNDERGROUND_TANK_AVG_VOL) && (underground_tank_analysis.tank02 >= UNDERGROUND_TANK_MAX_VOL)) {
                underground_tank_analysis.payload = "underground-tank01-selected";
                underground_tank_analysis.payload.toCharArray(data, (underground_tank_analysis.payload.length() + 1));
                client.publish("FM-Report", data, 2);
                UnderTank01ReplenishmentProcess.enable();
                Serial.println("c4: underground-tank01-selected");
            }else if ((underground_tank_analysis.tank01 >= UNDERGROUND_TANK_MAX_VOL) && (underground_tank_analysis.tank02 >= UNDERGROUND_TANK_MAX_VOL)) {
                underground_tank_analysis.payload = "S.UTR";
                underground_tank_analysis.payload.toCharArray(data, (underground_tank_analysis.payload.length() + 1));
                for (int i = 0; i < 3; i++){
                    client.publish("FM-Node01", data, 2);
                }
                Serial.println("c5: Both Tanks are filled...Replenishment aborted");
            }
        }


        void monitorUnderGroundTank01FuelLevel () {
            char data1[250]; char data2[250];
            underground_tank01.currentVolume = _available_fuel_volume_underground_tank01;
            if (underground_tank01.currentVolume != underground_tank01.previousFuelVolume) {
                //report volume;
                doc["source"] = "uTank01";
                if (underground_tank01.currentVolume >= UNDERGROUND_TANK_MAX_VOL) {
                    if (_available_fuel_volume_underground_tank02 < UNDERGROUND_TANK_MAX_VOL){
                        String payload = "A.UT02R";  //Activate Underground Tank02Valve
                        payload.toCharArray(data, (payload.length() + 1));
                        for (int i = 0; i < 3; i++) {
                            client.publish("FM-Node02", data, 2);
                        }
                        TerminateUnderTank01ReplenishmentProcess.enable();
                        Serial.println("Activate Tank02Valve     Volume: " + (String)underground_tank01.currentVolume);
                    }else{
                        //deactivate Tank01 Valve
                        doc["vol"] = underground_tank01.currentVolume;
                        String payload = "D.UT01R";  //stop  All undergroundtank replenishment
                        payload.toCharArray(data, (payload.length() + 1));
                        for (int i = 0; i < 3; i++) {
                            client.publish("FM-Node01", data, 2);
                        }
                        Serial.println("Tank01: Maximum Level     Volume: " + (String)underground_tank01.currentVolume);   
                    }
                }else if (underground_tank01.currentVolume <= UNDERGROUND_TANK_MIN_VOL) {
                    doc["vol"] = underground_tank01.currentVolume;
                    Serial.println("Tank01: Minimum Level     Volume: " + (String)underground_tank01.currentVolume);
                }else if ((underground_tank01.currentVolume < UNDERGROUND_TANK_MAX_VOL) || (underground_tank01.currentVolume == UNDERGROUND_TANK_AVG_VOL)) {
                    doc["advice"] = "replenishment due in exactly 1week";
                    Serial.println("Tank01: Average Level     Volume: " + (String)underground_tank01.currentVolume);
                }else {
                    doc["vol"] = underground_tank01.currentVolume;
                    Serial.println("Tank01Volume: " + (String)underground_tank01.currentVolume);
                }
                String payload = doc.as<String>();
                payload.toCharArray(data, (payload.length() + 1));
                client.publish("FM-report", data);
                vTaskDelay(200 / portTICK_PERIOD_MS);

                String payload1 = (String)underground_tank01.currentVolume;  
                payload1.toCharArray(data1, (payload1.length() + 1));
                client.publish("UTank01", data1, 2);
            }
            underground_tank01.previousFuelVolume = underground_tank01.currentVolume;
        }


        void monitorUnderGroundTank02FuelLevel () {
            char data1[250]; char data2[250];
            underground_tank02.currentVolume = _available_fuel_volume_underground_tank02;
            if (underground_tank02.currentVolume != underground_tank02.previousFuelVolume) {
                //report volume;
                doc["source"] = "uTank02";
                if (underground_tank02.currentVolume >= UNDERGROUND_TANK_MAX_VOL) {
                    doc["vol"] = underground_tank02.currentVolume;
                    if (_available_fuel_volume_underground_tank01 < UNDERGROUND_TANK_MAX_VOL){
                        //activate Tank02 valve
                        String payload = "A.UT01R";  //Activate Underground Tank01 Valve
                        payload.toCharArray(data, (payload.length() + 1));
                        for (int i = 0; i < 3; i++){
                            client.publish("FM-Node01", data, 2);
                        }
                        vTaskDelay(200);
                        TerminateUnderTank02ReplenishmentProcess.enable();
                        Serial.println("Tank02: Maxed-out, Activate Tank02Valve     Volume: " + (String)underground_tank02.currentVolume);
                    }else{
                        //deactivate Tank02 Valve
                        String payload = "D.UT02R";  //Deactivate underground tank02 replenishment
                        payload.toCharArray(data1, (payload.length() + 1));
                        for (int i = 0; i < 3; i++) {
                            client.publish("FM-Node02", data1, 2);
                        }
                        vTaskDelay(200);
                        TerminateUnderTank02ReplenishmentProcess.enable();
                        Serial.println("Tank02: Maximum Level     Volume: " + (String)underground_tank02.currentVolume);      
                    }
                }else if (underground_tank02.currentVolume <= UNDERGROUND_TANK_MIN_VOL) {
                    doc["vol"] = underground_tank02.currentVolume;
                    Serial.println("Tank02: Minimum Level     Volume: " + (String)underground_tank02.currentVolume);
                }else if ((underground_tank02.currentVolume < UNDERGROUND_TANK_MAX_VOL) || (underground_tank02.currentVolume == UNDERGROUND_TANK_AVG_VOL)) {
                    doc["advise"] = "replenishment due in exactly 1week";
                    Serial.println("Tank02: Average Level     Volume: " + (String)underground_tank02.currentVolume);                    
                }else {
                    doc["vol"] = underground_tank02.currentVolume;
                    Serial.println("Tank02Volume: " + (String)underground_tank02.currentVolume);
                }
                String payload = doc.as<String>();
                payload.toCharArray(data, (payload.length() + 1));
                client.publish("FM-report", data);

                String payload1 = (String)underground_tank02.currentVolume;  
                payload1.toCharArray(data1, (payload1.length() + 1));
                client.publish("UTank02", data1, 2);
            }
            underground_tank02.previousFuelVolume = underground_tank02.currentVolume;
        }
};



void tankSelectionForUnderGroundReplenishmentRuntime() {
    UndergroundTankVolumetricAnalysis undergroundTankVolumetricAnalysis;
    undergroundTankVolumetricAnalysis.selectTankToReplenish();
    TankSelectionForUnderGroundReplenishment.disable();
}

void undergroundTank01VolumeRunTime() {
    UndergroundTankVolumetricAnalysis undergroundTankVolumetricAnalysis;
    undergroundTankVolumetricAnalysis.monitorUnderGroundTank01FuelLevel();
}

void undergroundTank02VolumeRunTime() {
    UndergroundTankVolumetricAnalysis undergroundTankVolumetricAnalysis;
    undergroundTankVolumetricAnalysis.monitorUnderGroundTank02FuelLevel();
}

#endif //_UNDER_GROUND_TANK_VOLUMETRIC_ANALYSIS