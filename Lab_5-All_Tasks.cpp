//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"
#include <string>
#include "rtos.h"

//=====[Declaration and initialization of public global objects]===============

using rtos::Thread;

DigitalIn enterButton(BUTTON1);
DigitalIn gasDetector(D2);
DigitalIn overTempDetector(D3);
DigitalIn aButton(D4);
DigitalIn bButton(D5);
DigitalIn cButton(D6);
DigitalIn dButton(D7);

DigitalOut alarmLed(LED1);
DigitalOut incorrectCodeLed(LED3);
DigitalOut systemBlockedLed(LED2);

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

//=====[Declaration and initialization of public global variables]=============

bool alarmState = OFF;
bool gas_state = OFF;
bool temperature_state = OFF;
int numberOfIncorrectCodes = 0;

//=====[Declarations (prototypes) of public functions]=========================

void inputsInit();
void outputsInit();

void alarmActivationUpdate();
void alarmDeactivationUpdate();

void alarmStateCheck();

void uartTask();
void availableCommands();

//=====[Main function, the program entry point after power on or reset]========

int main()
{
    Thread thread; //need this for some bullshit reason I don't understand
    inputsInit();
    outputsInit();
    while (true) {
      thread.start(alarmStateCheck);
      alarmActivationUpdate();
      alarmDeactivationUpdate();
      uartTask();
    }
}

//=====[Implementations of public functions]===================================

void inputsInit()
{
    gasDetector.mode(PullDown);
    overTempDetector.mode(PullDown);
    aButton.mode(PullDown);
    bButton.mode(PullDown);
    cButton.mode(PullDown);
    dButton.mode(PullDown);
}

void outputsInit()
{
    alarmLed = OFF;
    incorrectCodeLed = OFF;
    systemBlockedLed = OFF;
}

void alarmActivationUpdate()
{
    if (gasDetector == true) {
        alarmState = ON;
        gas_state = ON;
        alarmLed = alarmState;
        uartUsb.write( "Warning: gas detected \r\n", 28);
        delay (500);

    }else if (overTempDetector == true){
        alarmState = ON;
        temperature_state = ON;
        alarmLed = alarmState;
        uartUsb.write( "Warning: Over temperature \r\n", 32);
        delay (500);
    }else {
        gas_state = OFF;
        temperature_state = OFF;
    }
}

void alarmDeactivationUpdate()
{
    if ( numberOfIncorrectCodes < 5 ) {
        if ( aButton && bButton && cButton && dButton && !enterButton ) {
            incorrectCodeLed = OFF;
        }
        if ( enterButton && !incorrectCodeLed && alarmState ) {
            if ( aButton && bButton && !cButton && !dButton ) {
                alarmState = OFF;
                numberOfIncorrectCodes = 0;
            } else {
                incorrectCodeLed = ON;
                numberOfIncorrectCodes = numberOfIncorrectCodes + 1;
            }
        }
    } else {
        systemBlockedLed = ON;
    }
}

void uartTask()
{
    char receivedChar = '\0';
    if( uartUsb.readable() ) {
        uartUsb.read( &receivedChar, 1);
        if ( receivedChar == '1') {
            if ( alarmState == true ) {
                uartUsb.write( "The alarm is activated\r\n", 24);
            } else {
                uartUsb.write( "The alarm is not activated\r\n", 28);
            }
        }
        else if ( receivedChar == '2') {
            if ( gasDetector == true ) {
                uartUsb.write( "The Gas Detector is activated\r\n", 32);
            } else {
                uartUsb.write( "The Gas Detector is not activated\r\n", 36);
            }
        }
        else if ( receivedChar == '3') {
            if ( overTempDetector == true ) {
                uartUsb.write( "The Temperature Sensor is activated\r\n", 38);
                } else {
                uartUsb.write( "The Temperature Sensor is not activated \r\n", 42);
            } 
            
        } else {
            availableCommands();
        }
    }
}
void alarmStateCheck()
{  
    while(true){
        char str[100] = "";
        sprintf ( str, "\r\nCurrent alarm state: %d \r\n", alarmState);
        uartUsb.write( str, 28 );
        char str_gas[100] = "";
        sprintf ( str_gas, "Current gas state: %d \r\n", gas_state);
        uartUsb.write( str_gas, 28 );
        char str_temp[100] = "";
        sprintf ( str_temp, "Current temperature state: %d \r\n\r\n", temperature_state);
        uartUsb.write( str_temp, 28 );
        rtos::ThisThread::sleep_for(5000ms);
    }
}   
void availableCommands()
{
    uartUsb.write( "Available commands:\r\n", 21 );
    uartUsb.write( "Press '1' to get the alarm state\r\n\r\n", 36 );
    uartUsb.write( "Press '2' to get the Gas Sensor state\r\n\r\n", 42 );
    uartUsb.write( "Press '3' to get the Temperature Sensor state\r\n\r\n", 48 );
}
