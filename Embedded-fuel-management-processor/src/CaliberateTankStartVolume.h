
#ifndef _SET_TANK_INITIAL_VOLUME
#define _SET_TANK_INITIAL_VOLUME


class CaliberateTankStartVolume {
    protected:
        String TankName;
        int TankInitalVolume;
    public:
        String command;
        String tankInitials;
        String tankVolume;
        char data[2050];
        StaticJsonDocument<2050> doc;

        void processIncomingParameter(){
            if (Serial.available()) {
                char c = Serial.read();
                if (c == '\n') {
                parseParameters(command);
                command = "";
                }
                else {
                command += c;
                 }
            
            }
        }

        void parseParameters(String instruction) {
            Serial.println("Incoming-command: "+ instruction);
            tankInitials = instruction.substring(0, instruction.indexOf(" "));
            tankVolume = instruction.substring(instruction.indexOf(" ") + 1);

            if (tankInitials.equalsIgnoreCase("UNTK01")) {
                String payload = "UPT01"+(String)_available_fuel_volume_underground_tank01;  
                payload.toCharArray(data, (payload.length() + 1));
                client.publish("UTank", data, true);
                _available_fuel_volume_underground_tank01 = tankVolume.toInt();

            }else if (tankInitials.equalsIgnoreCase("UNTK02")) {
                String payload = "UPT02"+(String)_available_fuel_volume_underground_tank02;  
                payload.toCharArray(data, (payload.length() + 1));
                client.publish("OUTank", data, true);
                _available_fuel_volume_underground_tank02 = tankVolume.toInt();

            }else if (tankInitials.equalsIgnoreCase("DAYTK")) {
                _available_fuel_volume_day_tank = tankVolume.toInt();
                String payload = "UDT"+(String)_available_fuel_volume_day_tank;  
                payload.toCharArray(data, (payload.length() + 1));
                client.publish("DTankR", data, true);
            }else if (tankInitials.equalsIgnoreCase("GEN01")) {
                _available_fuel_volume_gen01 = tankVolume.toInt();
                String payload = "SG01"+(String)_available_fuel_volume_gen01;  
                payload.toCharArray(data, (payload.length() + 1));
                client.publish("GTank", data, true);
            }else if (tankInitials.equalsIgnoreCase("GEN02")) {
                _available_fuel_volume_gen02 = tankVolume.toInt();
                String payload = "SG02"+(String)_available_fuel_volume_gen02;  
                payload.toCharArray(data, (payload.length() + 1));
                client.publish("UGen02", data, true);
            }else if (tankInitials.equalsIgnoreCase("GEN03")) {
                _available_fuel_volume_gen03 = tankVolume.toInt();
                String payload = "SG03"+(String)_available_fuel_volume_gen03;  
                payload.toCharArray(data, (payload.length() + 1));
                client.publish("OGTank", data, true);
            }else {
                Serial.println("Command Not Found");
                // Serial.write("Command Not Found");
            }
        }
};

void commandRuntime();
Task ParseCommand(500, TASK_FOREVER, &commandRuntime);

void commandRuntime() {
    CaliberateTankStartVolume caliberateTankStartVolume;
    caliberateTankStartVolume.processIncomingParameter();
}
#endif    //_SET_TANK_INITIAL_VOLUME