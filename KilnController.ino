#include <Wire.h>
#include "ESPRotary.h"
#include <LiquidCrystal_I2C.h>
#include "Adafruit_MAX31855.h"

ESPRotary r = ESPRotary(14, 12);
LiquidCrystal_I2C lcd(0x27,16,2);

#define MAXDO   5
#define MAXCS   0
#define MAXCLK  15
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);


int  counter = 1;
int counter2;
int timercounter = 1;
long  previousposition = 0;
long  newposition;
int buttonvalue;


float startingtemp;
float currenttemp;
float targettemp;
int finaltemp;
float ramprate;
float overalltime;
float overallstarttime;
float firingstarttime;
float currentmillis;
bool soak = false;
bool firingstarted = false;

int soakmillis;
int targettemp1;
int targettemp2;
int targettemp3;
int targettemp4;
int ramp1;
int ramp2;
int ramp3;
int ramp4;
int previousmillis = 0;
bool kilnrunning = false;

//function for scrolling through menu options using rotary encoder
void Rotaryrotate(ESPRotary& r) {
  newposition = (r.getPosition())/4; 
  if ((newposition > previousposition && counter == 2) || (newposition < previousposition && counter == 3)) {
    counter = 1;
    previousposition = newposition;
    Serial.println("Bisque");
    setdisplay();
    lcd.print("Bisque");
  }
  if ((newposition < previousposition && counter == 1) || (newposition > previousposition && counter == 3)) {
    counter = 2;
    previousposition = newposition;
    Serial.println("Glaze");
    setdisplay();
    lcd.print("Glaze");
  }
  if ((newposition < previousposition && counter == 2) || (newposition > previousposition && counter == 1 )) {
    counter = 3;
    previousposition = newposition;
    Serial.println("Overglaze");
    setdisplay();
    lcd.print("Overglaze");
  }
}



void bisque (){
  counter = 4;
  firingstarttime = millis();
  overallstarttime = millis();
  startingtemp = correctedCelsius();
  targettemp1 = 182;
  targettemp2 = 460;
  finaltemp = 1000;
  ramprate = 1.1;
  ramp2 = 2;
  ramp3 = 2.7;
}

void glaze() {
  counter = 5;
  counter2 = 1;
  firingstarttime = millis();
  overallstarttime = millis();
  startingtemp = correctedCelsius();
  targettemp1 = 121;
  targettemp2 = 1149;
  targettemp3 = 1200;
  finaltemp = 1149;
  ramprate = 0.7;
  ramp2 = 3;
  ramp3 = 0.7;
  ramp4 = 8;
}

void overglaze(){
  counter = 6;
  firingstarttime = millis();
  overallstarttime = millis();
  startingtemp = correctedCelsius();
  finaltemp = 638;
  ramprate = 2.2;
}

void setdisplay(){
  lcd.clear();
  lcd.setCursor(0,0);
}

float minutefunction(float currentMillis){
  float minute;
  minute = (currentMillis - firingstarttime)/60000;
  return minute;
}


float targetTempFunction (float currentMinute)
{
  float target;
  float currenttemperature = correctedCelsius();
  target = startingtemp + currentMinute * ramprate;
  //counter==4 is bisque firing parameters
  if (counter == 4){
    if ((currenttemperature >= targettemp1) && (timercounter == 1)){
      timercounter = 2;
      ramprate = ramp2;
      startingtemp = targettemp1;
      firingstarttime = millis();
       
    }
    else if ((currenttemperature >= targettemp2) && (timercounter == 2)){
      timercounter = 3;
      ramprate = ramp3;
      startingtemp = targettemp2;
      firingstarttime = millis(); 
    }
    else if ((currenttemperature >= finaltemp) && (timercounter == 3)) {
      timercounter = 4;
      ramprate = 0;
      startingtemp = finaltemp;
    }
  }
  //counter==5 is glaze firing parameters
  if (counter == 5){
    if ((currenttemperature >= targettemp1) && (timercounter == 1)){
      timercounter = 2;
      ramprate = ramp2;
      startingtemp = targettemp1;
      firingstarttime = millis(); 
    }
    else if ((currenttemperature >= targettemp2) && (timercounter == 2)){
      timercounter = 3;
      ramprate = ramp3; 
      startingtemp = targettemp2;
      firingstarttime = millis();
    }
    else if ((currenttemperature >= targettemp3) && (timercounter == 3)){
      timercounter = 4;
      ramprate = ramp4; 
      startingtemp = targettemp3;
      firingstarttime = millis();
    }
    else if ((currenttemperature >= finaltemp) && (ramprate == ramp4) && (timercounter == 4)) {
      timercounter = 5;
      ramprate = 0;
      startingtemp = finaltemp;
    }
  }
  //counter==6 is overglaze firing parameters
  if(counter==6){
    if ((currenttemperature >= finaltemp) && (timercounter == 1)) {
      timercounter = 2;
      ramprate = 0;
      startingtemp = finaltemp;
    }
  }
  //stop firing when finished
  if ((firingstarted == true) && (currenttemperature >= finaltemp) && (counter2 != 5) && (counter2 != 6))
  {
    kilnrunning = false;
    while (1 == 1){
      heatOFF();
      delay(1000);
    }
  }
  return target;
}

void heatON(){
  digitalWrite(16, HIGH);
}

void heatOFF(){
  digitalWrite(16, LOW);
}

float correctedCelsius(){
   
   // MAX31855 thermocouple voltage reading in mV
   float thermocoupleVoltage = (thermocouple.readCelsius() - thermocouple.readInternal()) * 0.041276;
   
   // MAX31855 cold junction voltage reading in mV
   float coldJunctionTemperature = thermocouple.readInternal();
   float coldJunctionVoltage = -0.176004136860E-01 +
      0.389212049750E-01  * coldJunctionTemperature +
      0.185587700320E-04  * pow(coldJunctionTemperature, 2.0) +
      -0.994575928740E-07 * pow(coldJunctionTemperature, 3.0) +
      0.318409457190E-09  * pow(coldJunctionTemperature, 4.0) +
      -0.560728448890E-12 * pow(coldJunctionTemperature, 5.0) +
      0.560750590590E-15  * pow(coldJunctionTemperature, 6.0) +
      -0.320207200030E-18 * pow(coldJunctionTemperature, 7.0) +
      0.971511471520E-22  * pow(coldJunctionTemperature, 8.0) +
      -0.121047212750E-25 * pow(coldJunctionTemperature, 9.0) +
      0.118597600000E+00  * exp(-0.118343200000E-03 * 
                           pow((coldJunctionTemperature-0.126968600000E+03), 2.0) 
                        );
                        
                        
   // cold junction voltage + thermocouple voltage         
   float voltageSum = thermocoupleVoltage + coldJunctionVoltage;
   
   // calculate corrected temperature reading based on coefficients for 3 different ranges   
   float b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
   if(thermocoupleVoltage < 0){
      b0 = 0.0000000E+00;
      b1 = 2.5173462E+01;
      b2 = -1.1662878E+00;
      b3 = -1.0833638E+00;
      b4 = -8.9773540E-01;
      b5 = -3.7342377E-01;
      b6 = -8.6632643E-02;
      b7 = -1.0450598E-02;
      b8 = -5.1920577E-04;
      b9 = 0.0000000E+00;
   }
   
   else if(thermocoupleVoltage < 20.644){
      b0 = 0.000000E+00;
      b1 = 2.508355E+01;
      b2 = 7.860106E-02;
      b3 = -2.503131E-01;
      b4 = 8.315270E-02;
      b5 = -1.228034E-02;
      b6 = 9.804036E-04;
      b7 = -4.413030E-05;
      b8 = 1.057734E-06;
      b9 = -1.052755E-08;
   }
   
   else if(thermocoupleVoltage < 54.886){
      b0 = -1.318058E+02;
      b1 = 4.830222E+01;
      b2 = -1.646031E+00;
      b3 = 5.464731E-02;
      b4 = -9.650715E-04;
      b5 = 8.802193E-06;
      b6 = -3.110810E-08;
      b7 = 0.000000E+00;
      b8 = 0.000000E+00;
      b9 = 0.000000E+00;
   }
   
   else {
      // TODO: handle error - out of range
      return 0;
   }
   
   return b0 + 
      b1 * voltageSum +
      b2 * pow(voltageSum, 2.0) +
      b3 * pow(voltageSum, 3.0) +
      b4 * pow(voltageSum, 4.0) +
      b5 * pow(voltageSum, 5.0) +
      b6 * pow(voltageSum, 6.0) +
      b7 * pow(voltageSum, 7.0) +
      b8 * pow(voltageSum, 8.0) +
      b9 * pow(voltageSum, 9.0);
}
  
void setup(){
  r.setChangedHandler(Rotaryrotate);
  pinMode(13, INPUT_PULLUP);
  pinMode(16, OUTPUT);
  Serial.begin(9600);
  Wire.begin(4, 2);
  lcd.begin();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Bisque");
  heatOFF();
  startingtemp = correctedCelsius();
   
 }


void loop(){
  r.loop();

  //if button is pushed and on bisque menu option 
  if (digitalRead(13) == LOW && counter == 1){
    firingstarted = true;
    kilnrunning = true;
    bisque(); //gets temperature and ramp parameters
    currenttemp = correctedCelsius(); //start temperature
    currentmillis = millis(); //current time
    targettemp = targetTempFunction(minutefunction(currentmillis)); //calculates target temp given temperature and ramp parameters
    previousmillis = firingstarttime; //start time when button pushed
    //update lcd display
    setdisplay();
    lcd.print("CurTemp: ");
    lcd.print(currenttemp);
    lcd.setCursor(0,1);
    lcd.print("TarTemp: ");
    lcd.print(targettemp);
  }
  //if button is pushed and on glaze menu option 
  else if (digitalRead(13) == LOW && counter == 2){
    firingstarted = true;
    kilnrunning = true;
    glaze(); //gets temperature and ramp parameters
    currenttemp = correctedCelsius();
    currentmillis = millis(); //current time
    targettemp = targetTempFunction(minutefunction(currentmillis)); //calculates target temp given temperature and ramp parameters
    previousmillis = firingstarttime; //start time when button pushed
    //update lcd display
    setdisplay();
    lcd.print("CurTemp: ");
    lcd.print(currenttemp);
    lcd.setCursor(0,1);
    lcd.print("TarTemp: ");
    lcd.print(targettemp);
  }
  //if button is pushed and on overglaze menu option 
  else if (digitalRead(13) == LOW && counter == 3){
    firingstarted = true;
    kilnrunning = true;
    overglaze(); //gets temperature and ramp parameters
    currenttemp = correctedCelsius(); //start temperature
    currentmillis = millis(); //current time
    targettemp = targetTempFunction(minutefunction(currentmillis)); //calculates target temp given temperature and ramp parameters
    previousmillis = firingstarttime; //start time when button pushed
    //update lcd display
    setdisplay();
    lcd.print("CurTemp: ");
    lcd.print(currenttemp);
    lcd.setCursor(0,1);
    lcd.print("TarTemp: ");
    lcd.print(targettemp);
  }

  currenttemp = correctedCelsius();
  currentmillis = millis();
  
  if (soak == false){
  targettemp = targetTempFunction(minutefunction(currentmillis));
  }
 
  if (firingstarted == true && currentmillis-previousmillis >= 1000){
    previousmillis = currentmillis;
    setdisplay();
    lcd.print("CurTemp: ");
    lcd.print(currenttemp);
    lcd.setCursor(0,1);
    lcd.print("TarTemp: ");
    lcd.print(targettemp);
    totaltime(currentmillis);
    delay(100);
  }

  //soak1 for glaze fire 
  if ((counter2 == 1) && (currenttemp > targettemp1)){
    counter2 = 2;
    targettemp = targettemp1;
    soak = true;
    soakmillis = millis();
   }
  //end soak1 for glaze fire
  if ((counter2 == 2) && (soak == true) && (currentmillis - soakmillis >= 3.6e+6)){
    counter2 = 3;
    soak = false;
  }
  //soak2 for glaze fire
  if ((counter2 == 3) && (currenttemp > targettemp3)){
    counter2 = 4;
    targettemp = targettemp3;
    soak = true;
    soakmillis = millis();
   }
   //end soak2 for glaze fire
  if ((counter2 == 4) && (soak == true) && (currentmillis - soakmillis >= 600000)){
    counter2 = 5;
    soak = false;
  }
  //soak3 for glaze fire
  if ((counter2 == 5) && (currenttemp > finaltemp)){
    counter2 = 6;
    targettemp = finaltemp;
    soak = true;
    soakmillis = millis();
   }
  //end soak3 for glaze fire
  if ((counter2 == 6) && (soak == true) && (currentmillis - soakmillis >= 1.8e+6)){
    counter2 = 7;
    soak = false;
  }

  //turn elements on if thermocouple temperature is below target temperature
  if ((firingstarted == true)&&(currenttemp < targettemp))
  {
    heatON();
  }
  //turn elements off if thermocouple temperature is above target temperature
  if ((firingstarted == true)&&(currenttemp > targettemp))
  {
    heatOFF();
  }
}



  
