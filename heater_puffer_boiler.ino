/* supervision system for heater (puffer and boiler)
 * sw by Nicu FLORICA (niq)ro)
 * ver.0 - initial, 22.2.2012, Craiova
 * ver.1 - added new condition (winter/summer time)
 * ver.1a - small corrections
 * 
*/

#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define pompa1 7  // pin for control pump no. 1
#define pompa2 6  // pin for control pump no. 2
#define senzori 9   // DS18B20 sensors
#define selector 8  // pin for winter/summer selector

#define precizie 9  // temperature precision (9..12bites)

OneWire oneWire(senzori);  // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.
DeviceAddress senzorcazan, senzorpufer, senzorboiler; // arrays to hold device addresses 

LiquidCrystal lcd(11, 12, 5, 4, 3, 2); 
// LCD module connections (RS, E, D4, D5, D6, D7)

float dete = 3.;  // hysteresis temperature for water
float tecazan = 0;   // temperature for water in heater
float tepufer = 0;   // temperature for water in puffer
float teboiler = 0;   // temperature for water in boiler

int tppauza = 10;  // sleep time for read the sensors
int tpcitire = 0;  // sensors read time

// for relay module (0 = ON, 1 = OFF)
#define oprit 1
#define pornit 0

byte starep1 = 0;  // status for pump no. 1
byte starep2 = 0;  // status for pump no. 1

byte calorifer = 1;  // status for heating system (1 is for winter, 0 - summer)


byte cazan[] = {  // https://maxpromer.github.io/LCD-Character-Creator/
  B00000,
  B01111,
  B10000,
  B01110,
  B00001,
  B01110,
  B10000,
  B01111
};

byte pufer[] = {
  B01110,
  B10001,
  B10011,
  B10101,
  B10101,
  B10011,
  B10001,
  B01110
};

byte boiler[] = {
  B00100,
  B11011,
  B00100,
  B00100,
  B11110,
  B11111,
  B00001,
  B00000
};

byte pump1[] = {
  B00010,
  B00010,
  B00010,
  B00000,
  B01100,
  B01010,
  B01100,
  B01000
};

byte pump2[] = {
  B00101,
  B00101,
  B00101,
  B00000,
  B01100,
  B01010,
  B01100,
  B01000
};

byte vara[] = {
  B00100,
  B10101,
  B01110,
  B11011,
  B01110,
  B10101,
  B00100,
  B00000
};

void setup()
{   
Serial.begin(9600);  // start serial port
Serial.println("...");

lcd.begin(16,2); // initialize the LCD
lcd.clear();  // clear the screen
  lcd.createChar(5, cazan);  // create custom char
  lcd.createChar(1, pufer);
  lcd.createChar(2, boiler);
  lcd.createChar(3, pump1);
  lcd.createChar(4, pump2);
  lcd.createChar(6, vara);

lcd.setCursor(0,0);
lcd.print(" Sistem v.1.0.0 ");
lcd.setCursor(0,1);
lcd.print("    by niq_ro   ");

pinMode(pompa1, OUTPUT);
pinMode(pompa2, OUTPUT);
digitalWrite(pompa1, oprit);
digitalWrite(pompa2, oprit);
pinMode(selector, INPUT);
digitalWrite(selector, HIGH);

if (digitalRead(selector) == LOW) calorifer = 0;  // summer time

sensors.begin();   // Start up the library for DS18B20 sensors
 // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");  
  if (!sensors.getAddress(senzorcazan, 0)) Serial.println("Unable to find address for Device 0");
  if (!sensors.getAddress(senzorpufer, 1)) Serial.println("Unable to find address for Device 1");
  if (!sensors.getAddress(senzorboiler, 2)) Serial.println("Unable to find address for Device 2");
  Serial.print("Device 0 Address: ");  // show the addresses we found on the bus
  printAddress(senzorcazan);
  Serial.println();
  Serial.print("Device 1 Address: ");
  printAddress(senzorpufer);
  Serial.println();
  sensors.setResolution(senzorcazan, precizie);  // set the resolution to 9 bit per device
  sensors.setResolution(senzorpufer, precizie);
  sensors.setResolution(senzorboiler, precizie);
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(senzorcazan), DEC);
  Serial.println();
  Serial.print("Device 1 Resolution: ");
  Serial.print(sensors.getResolution(senzorpufer), DEC);
  Serial.println();
  Serial.print("Device 2 Resolution: ");
  Serial.print(sensors.getResolution(senzorboiler), DEC);
  Serial.println();

delay(2000);
lcd.clear();

Serial.print("Requesting temperatures...");
sensors.requestTemperatures();
Serial.println("DONE");

tecazan = sensors.getTempC(senzorcazan);
  if(tecazan == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
tepufer = sensors.getTempC(senzorpufer);
  if(tepufer == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
teboiler = sensors.getTempC(senzorboiler);
  if(teboiler == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
} // end setup

void loop()
{

if (millis()/1000 - tpcitire > tppauza)
{
Serial.print("Requesting temperatures...");
sensors.requestTemperatures();
Serial.println("DONE");

tecazan = sensors.getTempC(senzorcazan);
  if(tecazan == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
tepufer = sensors.getTempC(senzorpufer);
  if(tepufer == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
teboiler = sensors.getTempC(senzorboiler);
  if(teboiler == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
tpcitire = millis();
}

if ((tecazan > tepufer + dete) and (calorifer == 1))  // just is winter time
{
  digitalWrite(pompa1, pornit);  // pump no.1 is on
  starep1 = 1;
  lcd.setCursor(15,0);
  lcd.write(3); // show the pump no. 1
}

if (tecazan < tepufer)
{
  digitalWrite(pompa1, oprit);  // pump no.1 is off
  starep1 = 0;
  lcd.setCursor(15,0);
  lcd.print(" ");
}

if ((tecazan > teboiler + dete) and (starep1 == 0))  // just if pump no. 1 is off
{
  digitalWrite(pompa2, pornit);  // pump no.2 is on
  starep2 = 1;
  lcd.setCursor(15,1);
  lcd.write(4); // show the pump no. 2
}

if ((tecazan < teboiler) or (starep1 == 1))
{
  digitalWrite(pompa2, oprit);  // pump no.2 is off
  starep2 = 0;
  lcd.setCursor(15,1);
  lcd.print(" ");
}

// show the heater temperature
    if (tecazan < 0) tecazan = 0;
    lcd.setCursor(0, 0);
    lcd.write(5);  // heater symbol
    if (tecazan < 100) lcd.print(":");
    if (tecazan < 10) lcd.print(" ");
    lcd.print(tecazan,0);
    lcd.print(char(223));
    lcd.print("C");

// show the puffer temperature
if (calorifer == 1)
{
    if (tepufer < 0) tepufer = 0;
    lcd.setCursor(7, 0);
    lcd.write(1);  // puffer symbol
    if (tepufer < 100) lcd.print(":");
    if (tepufer < 10) lcd.print(" ");
    lcd.print(tepufer,0);
    lcd.print(char(223));
    lcd.print("C");
}

// show the boiler temperature
    if (teboiler < 0) teboiler = 0;
    lcd.setCursor(7, 1);
    lcd.write(2);  // boiler symbol
    if (teboiler < 100) lcd.print(":");
    if (teboiler < 10) lcd.print(" ");
    lcd.print(teboiler,0);
    lcd.print(char(223));
    lcd.print("C");

if (calorifer == 1)
{
  lcd.setCursor(14,0);
  lcd.print(" ");
}
else
{
  lcd.setCursor(14,0);
  lcd.write(6); // show the sun symbol
}

}  // end main loop


// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
