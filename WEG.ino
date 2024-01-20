#include <HTTPClient.h>

#include <ETH.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>

#include <ACS712.h>
#include <LiquidCrystal.h>


LiquidCrystal lcd(19,23,18,17,16,15);

String apiKeyWater = "82BG82YB83YTEMEQ"; // Enter your Write API key from ThingSpeak
String apiKeyGas = "4PASUP9IQRSPF073";
String apiKeyEnergy = "MYQ6V78B8T0IDZHQ";


const char *ssid = "Rohit";     // replace with your wifi ssid and wpa2 key
const char *pass = "rohit1234";
const char *server = "api.thingspeak.com";


WiFiClient client;

unsigned long myChannelNumber = 1674175;
const char *myWriteAPIKey = "82BG82YB83YTEMEQ";



char watt[5];
ACS712  sensor(ACS712_20A,34);



#define LED_BUILTIN 16
#define SENSORWATER  0
#define SENSORGAS  2

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;

float calibrationFactorWater = 4.5;
float calibrationFactorGas = 4.8;

volatile byte pulseCountWater;
volatile byte pulseCountGas;
byte pulse1Sec = 0;
byte pulse1SecGas = 0;

float flowRateWater;
unsigned long flowMilliLitresWater;
unsigned int totalMilliLitresWater;
float flowLitresWater;
float totalLitresWater;

float flowRateGas;
unsigned long flowMilliLitresGas;
unsigned int totalMilliLitresGas;
float flowLitresGas;
float totalLitresGas;

unsigned long last_time =0;
unsigned long current_time =0;
float Wh =0 ;  
void IRAM_ATTR pulseCounter()
{
  pulseCountWater++;
  
}

void IRAM_ATTR pulseCounterGas()
{
//  pulseCountWater++;
  pulseCountGas++;
}


void setup()
{
  
  Serial.print( "\t Starting: " );
  Serial.begin(115200);
  ConnectToWiFi();
  sensor.calibrate();
  
  delay(10);
  
 
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSORWATER, INPUT_PULLUP);
  
  pinMode(SENSORGAS, INPUT_PULLUP);
 
  pulseCountWater = 0;
  flowRateWater = 0.0;
  flowMilliLitresWater = 0;
  totalMilliLitresWater = 0;

  
  pulseCountGas = 0;
  flowRateGas  = 0.0;
  flowMilliLitresGas  = 0;
  totalMilliLitresGas  = 0;
  previousMillis = 0;
 
  attachInterrupt(digitalPinToInterrupt(SENSORWATER), pulseCounter, FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSORGAS), pulseCounterGas, FALLING);
}

void ConnectToWiFi()
{
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to "); Serial.println(ssid);
 
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
 
    if ((++i % 16) == 0)
    {
      Serial.println(F(" still trying to connect"));
    }
  }
 
  Serial.print(F("Connected. My IP address is: "));
  Serial.println(WiFi.localIP());
}


void loop()
{
  float V = 0.0001;
  float I = sensor.getCurrentAC();
// Serial.println(I);
  float P = V * I;
  last_time = current_time;
  current_time = millis();    
  Wh = Wh+  P *(( current_time -last_time) /3600000.0) ;
  //Wh=Wh/1000;
  dtostrf(Wh, 4, 4, watt);
  
  Serial.print( "\t Wh " );
  Serial.print(Wh);
  Serial.print( "\t Watt: " );
  Serial.print(watt);
   Serial.print("\t");
  delay(1000);
  
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) 
  {
    
    pulse1Sec = pulseCountWater;
    pulseCountWater = 0;
    
    pulse1SecGas = pulseCountGas;
    pulseCountGas = 0;
    
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRateWater = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactorWater;
//    previousMillis = millis();

    flowRateGas = ((1000.0 / (millis() - previousMillis)) * pulse1SecGas) / calibrationFactorGas;
    previousMillis = millis();
 
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    
    flowMilliLitresWater = (flowRateWater / 60) * 1000;
    flowLitresWater = (flowRateWater / 60);

    flowMilliLitresGas = (flowRateGas / 60) * 1000;
    flowLitresGas = (flowRateGas / 60);
 
    // Add the millilitres passed in this second to the cumulative total
   
    totalMilliLitresWater += flowMilliLitresWater;
    totalLitresWater += flowLitresWater;

    totalMilliLitresGas += flowMilliLitresGas;
    totalLitresGas += flowLitresGas;
     
    // Print the flow rate for this second in litres / minute
    Serial.print("Water Flow rate: ");
    Serial.print(float(flowRateWater));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space
 
    // Print the cumulative total of litres flowed since starting
    Serial.print(" Water Output Liquid Quantity: ");
    Serial.print(totalMilliLitresWater);
    Serial.print("mL");
    Serial.print(totalLitresWater);
    Serial.println("L");

  // Print the flow rate for this second in litres / minute
    Serial.print("Gas Flow rate: ");
    Serial.print(float(flowRateGas));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space
 
    // Print the cumulative total of litres flowed since starting
    Serial.print("Gas Output Liquid Quantity: ");
    Serial.print(totalMilliLitresGas);
    Serial.print("mL/");
    Serial.print(totalLitresGas);
    Serial.println("L");


  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("W:");
  lcd.print(totalLitresWater);
  lcd.print("L "); 
  lcd.print("G:");
  lcd.print(totalLitresGas);
  lcd.print("L ");
  lcd.setCursor(0, 1);      
  lcd.print("E:");
  lcd.print(watt);
  lcd.print("unit");
    
  }
   if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
  
   HTTPClient httpWater;  
   HTTPClient httpGas;   
   HTTPClient httpEnergy;    
String api_url_water = "https://api.thingspeak.com/update.json?api_key=";
      api_url_water += apiKeyWater;
      api_url_water += "&field1=";
      api_url_water += String(float(flowRateWater));
      api_url_water += "&field2=";
      api_url_water += String(totalLitresWater);
      api_url_water += "";

      String api_url_gas = "https://api.thingspeak.com/update.json?api_key=";
      api_url_gas += apiKeyGas;
      api_url_gas += "&field1=";
      api_url_gas += String(float(flowRateGas));
      api_url_gas += "&field2=";
      api_url_gas += String(totalLitresGas);
      api_url_gas += "";

      String api_url_energy = "https://api.thingspeak.com/update.json?api_key=";
      api_url_energy += apiKeyEnergy;
      api_url_energy += "&field1=";
      api_url_energy += String(watt);
      api_url_energy += "";
  
   httpWater.begin(api_url_water);  //Specify destination for HTTP request
   httpWater.addHeader("Content-Type", "application/x-www-form-urlencoded");             //Specify content-type header
  
   int httpResponseCode = httpWater.POST("");   //Send the actual POST request

  httpGas.begin(api_url_gas);  //Specify destination for HTTP request
   httpGas.addHeader("Content-Type", "application/x-www-form-urlencoded");             //Specify content-type header
  
   int httpResponseCodeGas = httpGas.POST(""); 

   httpEnergy.begin(api_url_energy);  //Specify destination for HTTP request
   httpEnergy.addHeader("Content-Type", "application/x-www-form-urlencoded");             //Specify content-type header
  
   int httpResponseCodeEnergy = httpEnergy.POST(""); 
   
   if(httpResponseCode>0||httpResponseCodeGas>0||httpResponseCodeEnergy>0){
  
    String response = httpWater.getString();                       //Get the response to the request
  
    Serial.println(httpResponseCode);   //Print return code
    Serial.println(response);           //Print request answer
  
   }else{
  
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  
   }
  
   httpWater.end();
   httpGas.end();
   httpEnergy.end();//Free resources
  
 }else{
  
    Serial.println("Error in WiFi connection");   
  
 } 
  }
