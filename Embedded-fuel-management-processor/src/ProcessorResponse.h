#ifndef _PROCESSOR_RESPONSE
#define _PROCESSOR_RESPONSE

//prototype
void startUnderTank01ReplenishmentProcess();
void startUnderTank02ReplenishmentProcess();
void startDayTankReplenishmentProcess();
void startGen01TankReplenishmentProcess();
void startGen02TankReplenishmentProcess();
void startGen03TankReplenishmentProcess();
void start335KvaGen01Replenishment();

void stopUnderTank01ReplenishmentProcess();
void stopUnderTank02ReplenishmentProcess();
void stopDayTankReplenishmentProcess();
void stopGen01TankReplenishmentProcess();
void stopGen02TankReplenishmentProcess();
void stopGen03TankReplenishmentProcess();
void start335KvaGen02Replenishment();
void stop335KvaGen01Replenishment();
void stop335KvaGen02Replenishment();

Task UnderTank01ReplenishmentProcess(1000, TASK_FOREVER, &startUnderTank01ReplenishmentProcess);
Task UnderTank02ReplenishmentProcess(1000, TASK_FOREVER, &startUnderTank02ReplenishmentProcess);
Task DayTankReplenishmentProcess(1000, TASK_FOREVER, &startDayTankReplenishmentProcess);
Task Gen01TankReplenishmentProcess(1000, TASK_FOREVER, &startGen01TankReplenishmentProcess);
Task Gen02TankReplenishmentProcess(1000, TASK_FOREVER, &startGen02TankReplenishmentProcess);
Task Gen03TankReplenishmentProcess(1000, TASK_FOREVER, &startGen03TankReplenishmentProcess);
Task Begin335KvaGen01Replenishment(1000, TASK_FOREVER, &start335KvaGen01Replenishment);
Task Begin335KvaGen02Replenishment(1000, TASK_FOREVER, &start335KvaGen02Replenishment);

Task TerminateUnderTank01ReplenishmentProcess(1000, TASK_FOREVER, &stopUnderTank01ReplenishmentProcess);
Task TerminateUnderTank02ReplenishmentProcess(1000, TASK_FOREVER, &stopUnderTank02ReplenishmentProcess);
Task TerminateDayTankReplenishmentProcess(1000, TASK_FOREVER, &stopDayTankReplenishmentProcess);
Task TerminateGen01TankReplenishmentProcess(1000, TASK_FOREVER, &stopGen01TankReplenishmentProcess);
Task TerminateGen02TankReplenishmentProcess(1000, TASK_FOREVER, &stopGen02TankReplenishmentProcess);
Task TerminateGen03TankReplenishmentProcess(1000, TASK_FOREVER, &stopGen03TankReplenishmentProcess);
Task Terminate335KvaGen01ReplenishmentProcess(1000, TASK_FOREVER, &stop335KvaGen01Replenishment);
Task Terminate335KvaGen02ReplenishmentProcess(1000, TASK_FOREVER, &stop335KvaGen02Replenishment);


void stop335KvaGen01Replenishment() {
    char data[750];
    StaticJsonDocument<750> doc;

    String payload1 = "D.335G01R";  
    payload1.toCharArray(data, (payload1.length() + 1));
    for (int i = 0; i < 3; i++) {
        client.publish("FM-Node03", data, 2);
    }
    Terminate335KvaGen01ReplenishmentProcess.disable();
}

void stop335KvaGen02Replenishment() {
    char data[750];
    StaticJsonDocument<750> doc;

    String payload1 = "D.335G01R";  
    payload1.toCharArray(data, (payload1.length() + 1));
    for (int i = 0; i < 3; i++) {
        client.publish("FM-Node03", data, 2);
    }
    Terminate335KvaGen02ReplenishmentProcess.disable();
}

void start335KvaGen01Replenishment() {
    char data[750];
    StaticJsonDocument<750> doc;

    String payload1 = "E.335G01R";  
    payload1.toCharArray(data, (payload1.length() + 1));
    for (int i = 0; i < 3; i++) {
        client.publish("FM-Node03", data, 2);
    }
    Begin335KvaGen01Replenishment.disable();
}

void start335KvaGen02Replenishment() {
    char data[750];
    StaticJsonDocument<750> doc;

    String payload1 = "E.335G02R";  
    payload1.toCharArray(data, (payload1.length() + 1));
    for (int i = 0; i < 3; i++) {
        client.publish("FM-Node03", data, 2);
    }
    Begin335KvaGen02Replenishment.disable();
}

// Response Runtime
void startUnderTank01ReplenishmentProcess() {
    char data[750];
    StaticJsonDocument<750> doc;

    String payload1 = "A.UT01R";  
    payload1.toCharArray(data, (payload1.length() + 1));
    for (int i = 0; i < 3; i++) {
        client.publish("FM-Node01", data, 2);
    }
    UnderTank01ReplenishmentProcess.disable();
}

void startUnderTank02ReplenishmentProcess() {
    char data[750];
    StaticJsonDocument<750> doc;

    String payload1 = "A.UT02R";  
    payload1.toCharArray(data, (payload1.length() + 1));
    for (int i = 0; i < 3; i++) {
        client.publish("FM-Node02", data, 2);
    }
    UnderTank02ReplenishmentProcess.disable();
}

void startDayTankReplenishmentProcess() {
    char data[750];
    StaticJsonDocument<750> doc;

    String payload1 = "";  
    payload1.toCharArray(data, (payload1.length() + 1));
    client.publish("FM-Node02", data, 2);
    DayTankReplenishmentProcess.disable();
}

void startGen01TankReplenishmentProcess() {
    char data[750];
    StaticJsonDocument<750> doc;

    String payload1 = "E.G01R";  
    payload1.toCharArray(data, (payload1.length() + 1));
    for (int i = 0; i < 3; i++) {
        client.publish("FM-Node03", data, 2);
    }
    Gen01TankReplenishmentProcess.disable();
}

void startGen02TankReplenishmentProcess() {
    char data[750];
    StaticJsonDocument<750> doc;

    String payload1 = "E.G02R";  
    payload1.toCharArray(data, (payload1.length() + 1));
    for (int i = 0; i < 3; i++) {
        client.publish("FM-Node03", data, 2);
    }
    Gen02TankReplenishmentProcess.disable();
}

void startGen03TankReplenishmentProcess() {
    char data[750];
    StaticJsonDocument<750> doc;

    String payload1 = "E.G03R";  
    payload1.toCharArray(data, (payload1.length() + 1));
    for (int i = 0; i < 3; i++) {
        client.publish("FM-Node03", data, 2); 
    }
    Gen03TankReplenishmentProcess.disable();   
}

void stopUnderTank01ReplenishmentProcess() {
    char data[750];
    StaticJsonDocument<750> doc;

    String payload1 = "D.UT01R";  
    payload1.toCharArray(data, (payload1.length() + 1));
    for (int i = 0; i < 3; i++) {
        client.publish("FM-Node01", data, 2);
    }
    TerminateUnderTank01ReplenishmentProcess.disable();
    Serial.println("Command to Stop UTank01 Replenishment has been issued....");
}

void stopUnderTank02ReplenishmentProcess() {
    char data[750];
    StaticJsonDocument<750> doc;

    String payload1 = "S.UTR";    // close UndegroundTank02 valve
    payload1.toCharArray(data, (payload1.length() + 1));
    for (int i = 0; i < 3; i++) {
        client.publish("FM-Node01", data, 2);
    }
    Serial.println("Command to Stop UTank02 Replenishment has been issued....");
    TerminateUnderTank02ReplenishmentProcess.disable();
}

void stopDayTankReplenishmentProcess() {

}

void stopGen01TankReplenishmentProcess() {
    char data[750];
    StaticJsonDocument<750> doc;

    String payload1 = "D.G01R";  
    payload1.toCharArray(data, (payload1.length() + 1));
    for (int i = 0; i < 3; i++) {
        client.publish("FM-Node03", data, 2); 
    }
    TerminateGen01TankReplenishmentProcess.disable();
}

void stopGen02TankReplenishmentProcess() {
    char data[750];
    StaticJsonDocument<750> doc;

    String payload1 = "D.G02R";  
    payload1.toCharArray(data, (payload1.length() + 1));
    for (int i = 0; i < 3; i++) {
        client.publish("FM-Node03", data, 2); 
    }
    TerminateGen02TankReplenishmentProcess.disable();
}

void stopGen03TankReplenishmentProcess() {
    char data[750];
    StaticJsonDocument<750> doc;

    String payload1 = "D.G03R";  
    payload1.toCharArray(data, (payload1.length() + 1));
    for (int i = 0; i < 3; i++) {
        client.publish("FM-Node03", data, 2); 
    }
    TerminateGen03TankReplenishmentProcess.disable();
}

#endif  //_PROCESSOR_RESPONSE