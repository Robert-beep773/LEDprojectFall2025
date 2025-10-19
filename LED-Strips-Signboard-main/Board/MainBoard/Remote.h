#ifndef REMOTE_H
#define REMOTE_H

class RemoteControl {
private:
    uint8_t bright;
    bool remoteStatus;
    unsigned int enteredValue;
    int minu;
    int tbIndex;
    int fIndex;
    bool timerInputMode;
    String timerCodes[10] = {"fb04", "fa05", "f906", "f708", "f609", "f50a", "f30c", "f20d", "f10e", "ee11"};

    int getNumberFromIR(String command);
    void manualTimerInput();
    void changeTBColourScheme();
    void changeFColourScheme(); 
    void adjustBrightness(float change);
    void toggleRemote(String remoteCode);
    void handleTimerCodes(String remoteCode);
    void setDefaultMessage(String remoteCode);

public:
    RemoteControl();
    void setupRemote();
    void displayDefaultMessage();
    void useRemote();
    uint8_t getBrightness();
};

#endif

