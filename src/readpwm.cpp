#include "readpwm.h"

//Variables for reading pwm
String pwmStr = "";
char bufferFloat[20];
bool outputSent = false;
float period = 0;
int flag_PWM, flag_PWM_out = 0;

void readpwm(){
    
    period = pulseIn(PWM_pin, HIGH, 30000);  //period em us
    sprintf(bufferFloat, "%f", period);
      
    if (period >= 900 and period <= 2100) {

        if (period > 1500 && period < 2100) {
            flag_PWM = 1;

        } else if (period > 850 && period < 1300) {
            flag_PWM = 0;
        }
    }
    if (flag_PWM != flag_PWM_out) {
        getTIME();

        if (flag_PWM == 1) {
        Serial.print("PWM signal (ON):");
        Serial.println(period);
        appendFile(SD, "/data.txt", pwmNAME[0]);
        appendFile(SD, "/data.txt", "\t");
        pwmStr += pwmNAME[0];
        appendFile(SD, "/data.txt", bufferFloat);
        appendFile(SD, "/data.txt", "\n");
        pwmStr += bufferFloat;
        digitalWrite(PWM_LED, HIGH);
        outputSent = true;


        } else {
        Serial.print("PWM signal (OFF):");
        Serial.println(period);
        appendFile(SD, "/data.txt", pwmNAME[1]);
        appendFile(SD, "/data.txt", "\t");
        pwmStr += pwmNAME[1];
        appendFile(SD, "/data.txt", bufferFloat);
        appendFile(SD, "/data.txt", "\n");
        pwmStr += bufferFloat;
        digitalWrite(PWM_LED, LOW);
        outputSent = true;
        }

        flag_PWM_out = flag_PWM;
    }
}