
#ifndef _GENERATOR_FUEL_VOLUMETRIC_ANALYSIS
#define _GENERATOR_FUEL_VOLUMETRIC_ANALYSIS

#define GEN_MAX_FUEL 500
#define GEN_AVG_FUEL 350
#define GEN_MIN_FUEL 200

#define GEN_335KVA_MAX_FUEL 400
#define GEN_335KVA_MIN_FUEL 200

void monitorGenerator01FuelVolume();
void monitorGenerator02FuelVolume();
void monitorGenerator03FuelVolume();

Task ReportGenerator01FuelVolume(1000, TASK_FOREVER, &monitorGenerator01FuelVolume);
Task ReportGenerator02FuelVolume(1000, TASK_FOREVER, &monitorGenerator02FuelVolume);
Task ReportGenerator03FuelVolume(1000, TASK_FOREVER, &monitorGenerator03FuelVolume);


class GeneratorTankVolumetricAnalysis {
    protected:
        int Gen01FuelVolume;
        int Gen02FuelVolume;
        int Gen03FuelVolume;
    public:
        char data[2050];
        StaticJsonDocument<2050> doc;
        // GeneratorTankVolumetricAnalysis(int gen01FuelVolume, int gen02FuelVolume, int gen03FuelVolume) {

        // }
        void computeFuelInputToGenerator(int incomingVolume, int fuelAvailable, String topic) {
            const char* topicName;
            topicName = topic.c_str();

            generator_volumetric_analysis.totalAvailableFuelVolume = fuelAvailable + incomingVolume;
            fuelAvailable = generator_volumetric_analysis.totalAvailableFuelVolume; 
            String payload = (String)fuelAvailable;  
            payload.toCharArray(data, (payload.length() + 1));
            client.publish(topicName, data, true);
            Serial.println("Incoming-Volume: "+ (String)incomingVolume + "   CurrentFuelVolume: " + (String)fuelAvailable);
        }


        void computFuelConsumedByGenerator(int countsInSeconds, int genTankAvailableFuel, String topic) {
            const char* topicName;
            topicName = topic.c_str();
            //computing the consumption rate
            // 10/12KVA Generators consumes 0.9 litres per hour (half-load) approximate 1L
            //                              2.9 litres per hour (full load) approximate 3L
            generator_volumetric_analysis.genActiveTimeInHours = countsInSeconds / 3600;
            generator_volumetric_analysis.consumedFuelVolume = round(generator_volumetric_analysis.genActiveTimeInHours * 2.9);
            generator_volumetric_analysis.totalFuelConsumption = genTankAvailableFuel - generator_volumetric_analysis.consumedFuelVolume;
            genTankAvailableFuel = generator_volumetric_analysis.totalFuelConsumption;

            String payload = (String)genTankAvailableFuel;  
            payload.toCharArray(data, (payload.length() + 1));
            client.publish(topicName, data, true);
            Serial.println("Volume-consumed: "+ (String)generator_volumetric_analysis.consumedFuelVolume + "   CurrentFuelVolume: " + (String)genTankAvailableFuel);
        }


        void monitorGen01FuelLevel() {
            char data1[250];

            gen01_volumetric_analysis.currentVolume = _available_fuel_volume_gen01;
            if (gen01_volumetric_analysis.currentVolume != gen01_volumetric_analysis.previousFuelVolume) {
                doc["source"] = "Gen01";
                if (gen01_volumetric_analysis.currentVolume >= GEN_MAX_FUEL) {
                    doc["vol"] = gen01_volumetric_analysis.currentVolume;
                    Gen01TankReplenishmentProcess.enable();
                    Serial.println("Gen01: Maximum Level     Volume: " + (String)gen01_volumetric_analysis.currentVolume); 
                }
                else if (gen01_volumetric_analysis.currentVolume <= GEN_MIN_FUEL) {
                    doc["vol"] = gen01_volumetric_analysis.currentVolume;
                    TerminateGen01TankReplenishmentProcess.enable();
                    Serial.println("Gen01: Minimum Level     Volume: " + (String)gen01_volumetric_analysis.currentVolume); 
                }
                String payload = doc.as<String>();
                payload.toCharArray(data, (payload.length() + 1));
                client.publish("FM-report", data);
                vTaskDelay(200 / portTICK_PERIOD_MS);

                String payload1 = (String)gen01_volumetric_analysis.currentVolume;  
                payload1.toCharArray(data1, (payload1.length() + 1));
                client.publish("GEN01", data1, true);
            }
            gen01_volumetric_analysis.previousFuelVolume = gen01_volumetric_analysis.currentVolume;
        }



            void monitorGen02FuelLevel() {
                char data1[250];

                gen02_volumetric_analysis.currentVolume = _available_fuel_volume_gen02;
                if (gen02_volumetric_analysis.currentVolume != gen02_volumetric_analysis.previousFuelVolume) {
                    doc["source"] = "gen02";
                    if (gen02_volumetric_analysis.currentVolume >= GEN_MAX_FUEL) {
                        doc["vol"] = gen02_volumetric_analysis.currentVolume;
                        Gen02TankReplenishmentProcess.enable();
                        Serial.println("Gen02: Maximum Level     Volume: " + (String)gen02_volumetric_analysis.currentVolume); 
                    }
                    else if (gen02_volumetric_analysis.currentVolume < GEN_MIN_FUEL) {
                        doc["vol"] = gen02_volumetric_analysis.currentVolume;
                        TerminateGen02TankReplenishmentProcess.enable();
                        Serial.println("Gen02: Minimum Level     Volume: " + (String)gen02_volumetric_analysis.currentVolume); 
                    }
                    String payload = doc.as<String>();
                    payload.toCharArray(data, (payload.length() + 1));
                    client.publish("FM-report", data);
                    vTaskDelay(200 / portTICK_PERIOD_MS);

                    String payload1 = (String)gen02_volumetric_analysis.currentVolume;  
                    payload1.toCharArray(data1, (payload1.length() + 1));
                    client.publish("GEN02", data1, true);
                }
                gen02_volumetric_analysis.previousFuelVolume = gen02_volumetric_analysis.currentVolume;
            }


            void monitorGen03FuelLevel() {
                char data1[250];

                gen03_volumetric_analysis.currentVolume = _available_fuel_volume_gen03;
                if (gen03_volumetric_analysis.currentVolume != gen03_volumetric_analysis.previousFuelVolume) {
                    doc["source"] = "gen03";
                    if (gen03_volumetric_analysis.currentVolume >= GEN_MAX_FUEL) {
                        doc["vol"] = gen03_volumetric_analysis.currentVolume;
                        Gen03TankReplenishmentProcess.enable();
                        Serial.println("Gen03: Maximum Level     Volume: " + (String)gen03_volumetric_analysis.currentVolume); 
                    }
                    else if (gen03_volumetric_analysis.currentVolume < GEN_MIN_FUEL) {
                        doc["vol"] = gen03_volumetric_analysis.currentVolume;
                        TerminateGen03TankReplenishmentProcess.enable();
                        Serial.println("Gen03: Minimum Level     Volume: " + (String)gen03_volumetric_analysis.currentVolume); 
                    }
                    String payload = doc.as<String>();
                    payload.toCharArray(data, (payload.length() + 1));
                    client.publish("FM-report", data);
                    vTaskDelay(200 / portTICK_PERIOD_MS);

                    String payload1 = (String)gen03_volumetric_analysis.currentVolume;  
                    payload1.toCharArray(data1, (payload1.length() + 1));
                    client.publish("GEN03", data1, true);
                }
                gen03_volumetric_analysis.previousFuelVolume = gen03_volumetric_analysis.currentVolume;
            }


            void gen01335KVAFuelLevel() {
                char data1[250];
                gen01_335kva_volumetric_analysis.currentVolume = _available_fuel_volume_335kva_01;
                if (gen01_335kva_volumetric_analysis.currentVolume != gen01_335kva_volumetric_analysis.previousFuelVolume) {
                    doc["source"] = "335kvaGen01";
                    if (gen01_335kva_volumetric_analysis.currentVolume >= GEN_335KVA_MAX_FUEL) {
                        doc["vol"] = gen01_335kva_volumetric_analysis.currentVolume;
                        Terminate335KvaGen01ReplenishmentProcess.enable();
                        Serial.println("335KVAGen01: Maximum Level     Volume: " + (String)gen01_335kva_volumetric_analysis.currentVolume); 
                    }
                    else if (gen01_335kva_volumetric_analysis.currentVolume <= GEN_335KVA_MIN_FUEL) {
                        doc["vol"] = gen01_335kva_volumetric_analysis.currentVolume;
                        Begin335KvaGen01Replenishment.enable();
                        Serial.println("335KVAGen01: Minimum Level     Volume: " + (String)gen01_335kva_volumetric_analysis.currentVolume); 
                    }
                    else {
                        doc["vol"] = gen01_335kva_volumetric_analysis.currentVolume;
                    }
                    String payload = doc.as<String>();
                    payload.toCharArray(data, (payload.length() + 1));
                    client.publish("FM-report", data);
                    vTaskDelay(200 / portTICK_PERIOD_MS);

                    String payload1 = (String)gen01_335kva_volumetric_analysis.currentVolume;  
                    payload1.toCharArray(data1, (payload1.length() + 1));
                    client.publish("335kva01", data1, true);
                }
                gen01_335kva_volumetric_analysis.previousFuelVolume = gen01_335kva_volumetric_analysis.currentVolume;
            }


            void gen02335KVAFuelLevel() {
                char data1[250];
                gen02_335kva_volumetric_analysis.currentVolume = _available_fuel_volume_335kva_02;
                if (gen02_335kva_volumetric_analysis.currentVolume != gen02_335kva_volumetric_analysis.previousFuelVolume) {
                    doc["source"] = "335kvaGen02";
                    if (gen02_335kva_volumetric_analysis.currentVolume >= GEN_335KVA_MAX_FUEL) {
                        doc["vol"] = gen02_335kva_volumetric_analysis.currentVolume;
                        Terminate335KvaGen02ReplenishmentProcess.enable();
                        Serial.println("335KVAGen02: Maximum Level     Volume: " + (String)gen02_335kva_volumetric_analysis.currentVolume); 
                    }
                    else if (gen02_335kva_volumetric_analysis.currentVolume < GEN_335KVA_MIN_FUEL) {
                        doc["vol"] = gen02_335kva_volumetric_analysis.currentVolume;
                        Begin335KvaGen02Replenishment.enable();
                        Serial.println("335KVAGen02: Minimum Level     Volume: " + (String)gen02_335kva_volumetric_analysis.currentVolume); 
                    }
                    String payload = doc.as<String>();
                    payload.toCharArray(data, (payload.length() + 1));
                    client.publish("FM-report", data);
                    vTaskDelay(200 / portTICK_PERIOD_MS);

                    String payload1 = (String)gen02_335kva_volumetric_analysis.currentVolume;  
                    payload1.toCharArray(data1, (payload1.length() + 1));
                    client.publish("335kva02", data1, true);
                }
                gen02_335kva_volumetric_analysis.previousFuelVolume = gen02_335kva_volumetric_analysis.currentVolume;
            }
};




void monitorGenerator01FuelVolume() {
    GeneratorTankVolumetricAnalysis generator01TankVolumetricAnalysis;
    generator01TankVolumetricAnalysis.monitorGen01FuelLevel();
}

void monitorGenerator02FuelVolume() {
    GeneratorTankVolumetricAnalysis generator02TankVolumetricAnalysis;
    generator02TankVolumetricAnalysis.monitorGen02FuelLevel();
}

void monitorGenerator03FuelVolume() {
    GeneratorTankVolumetricAnalysis generator03TankVolumetricAnalysis;
    generator03TankVolumetricAnalysis.monitorGen03FuelLevel();
}


//Timer is to be set on the Generator module

// void startTimer();
// Task timer(1000, TASK_FOREVER, &startTimer);

// int c = 0;
// int timeInMinutes = 0;

// void startTimer() {
//     c++;
//     if (c == 60){
//         timeInMinutes = timeInMinutes + c;
//         Serial.println("Time-in-seconds: "+ (String)timeInMinutes);
//         c= 0;
//     }
// }
#endif