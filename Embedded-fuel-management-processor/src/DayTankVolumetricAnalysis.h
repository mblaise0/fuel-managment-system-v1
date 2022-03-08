
#ifndef _DAY_TANK_VOLUMETRIC_ANALYSIS
#define _DAY_TANK_VOLUMETRIC_ANALYSIS

#define DAY_TANK_VOL 11000
#define DAY_TANK_MAX_VOL 10450
#define DAY_TANK_MIN_VOL 2620
#define DAY_TANK_AVG_VOL 5225

void dayTankFuelVolumeRunTime();
Task ReportDayTankVolumetricAnalysis(1000, TASK_FOREVER, &dayTankFuelVolumeRunTime);

class DayTankVolumetricAnalysis {
    public:
        char data[2500];
        StaticJsonDocument<2500> doc; 

        void computeIncomingFuelVolume(int incomingVolume) {
            day_tank_volumetric_analysis.totalAvailableFuelVolume = incomingVolume + _available_fuel_volume_day_tank;
            _available_fuel_volume_day_tank = day_tank_volumetric_analysis.totalAvailableFuelVolume;
            String payload = (String)_available_fuel_volume_day_tank;  
            payload.toCharArray(data, (payload.length() + 1));
            client.publish("DTank", data, true);
            Serial.println("Incoming-Volume-to-day-tank: "+ (String)incomingVolume + "   CurrentFuelVolume: " + (String)_available_fuel_volume_day_tank);
        }

        void computeOutputFuelVolume(int outgoingVolume) {
            day_tank_volumetric_analysis.totalSupplyFuelVolume = _available_fuel_volume_day_tank - outgoingVolume;
            _available_fuel_volume_day_tank = day_tank_volumetric_analysis.totalSupplyFuelVolume;
            String payload = (String)_available_fuel_volume_day_tank;  
            payload.toCharArray(data, (payload.length() + 1));
            client.publish("DTank", data, true);
            Serial.println("Outgoing-Volume-from-day-tank :"+ (String)outgoingVolume + "   CurrentFuelVolume: " + (String)_available_fuel_volume_day_tank);
        }


        void monitorDayTankFuelLevel () {
            char data1[250]; String payload1;
            underground_tank_analysis.tank01 = _available_fuel_volume_underground_tank01;
            underground_tank_analysis.tank02 = _available_fuel_volume_underground_tank02;

            day_tank_volumetric_analysis.currentVolume = _available_fuel_volume_day_tank;
            if (day_tank_volumetric_analysis.currentVolume != day_tank_volumetric_analysis.previousFuelVolume) {
                doc["source"] = "day-tank";
                if (day_tank_volumetric_analysis.currentVolume >= DAY_TANK_MAX_VOL) {
                    doc["vol"] = day_tank_volumetric_analysis.currentVolume;
                    
                    String payload = "D.DTKR";
                    payload.toCharArray(data, (payload.length() + 1));
                    for (int i = 0; i < 3; i++){
                        client.publish("DTKR", data);
                    }
                    Serial.println("Day-tank: Maximum Level     Volume: " + (String)day_tank_volumetric_analysis.currentVolume);   
                } 
                else if (day_tank_volumetric_analysis.currentVolume <= DAY_TANK_MIN_VOL) {
                    if (underground_tank_analysis.tank01 > UNDERGROUND_TANK_MIN_VOL) {
                        payload1 = "UT01";  
                    } else{
                        if (underground_tank_analysis.tank02 > UNDERGROUND_TANK_MIN_VOL) {
                            payload1 = "UT02";  
                        }else {
                            payload1 = "Both Underground-Tanks are at minimum level";
                            payload1.toCharArray(data1, (payload1.length() + 1));
                            client.publish("FM-Report", data1);
                        }
                    }
                    doc["vol"] = day_tank_volumetric_analysis.currentVolume;
                    payload1.toCharArray(data1, (payload1.length() + 1));
                    for (int i = 0; i < 3; i++){
                        client.publish("ADTKR", data1, 2);
                    }
                    Serial.println("Day-tank: Minimum Level     Volume: " + (String)day_tank_volumetric_analysis.currentVolume);
                }
                else {
                    doc["vol"] = day_tank_volumetric_analysis.currentVolume;
                    Serial.println("Day-tank: " + (String)day_tank_volumetric_analysis.currentVolume);
                }
                String payload = doc.as<String>();
                payload.toCharArray(data, (payload.length() + 1));
                client.publish("FM-report", data);
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }
            day_tank_volumetric_analysis.previousFuelVolume = day_tank_volumetric_analysis.currentVolume;
        }

};



void dayTankFuelVolumeRunTime() {
    DayTankVolumetricAnalysis dayTankVolumetricAnalysis;
    dayTankVolumetricAnalysis.monitorDayTankFuelLevel();
}

#endif  //_DAY_TANK_VOLUMETRIC_ANALYSIS