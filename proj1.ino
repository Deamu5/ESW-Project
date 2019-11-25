#include "ThingSpeak.h"
#include <ESP8266WiFi.h>
#include "DHT.h"
#include <MiCS6814-I2C.h>
#define DHTPIN D3     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#include <SDS011.h>
float p10,p25;
int error;


#define SECRET_SSID "Sasuke Uchiha"    // replace MySSID with your WiFi network name
#define SECRET_PASS "556556556"  // replace MyPassword with your WiFi password

SDS011 my_sds;  
DHT dht(DHTPIN, DHTTYPE);
MiCS6814 sensor;
bool sensorConnected;
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
WiFiClient  client;
#define SECRET_WRITE_APIKEY "A7YDK1O23YQ9LO68"

unsigned long myChannelNumber = 868883;
const char myWriteAPIKey[] = SECRET_WRITE_APIKEY;


void setup(){
  // Initialize serial connection
  Serial.begin(115200);
   WiFi.mode(WIFI_STA); 
   WiFi.disconnect();
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  my_sds.begin(D5,D6);
  dht.begin();
//  delay(3000);

  // Connect to sensor using default I2C address (0x04)
  // Alternatively the address can be passed to begin(addr)
  sensorConnected = sensor.begin();
  
  if (sensorConnected == true) {
    sensor.powerOn();
      } else {
    Serial.println("Couldn't connect to MiCS-6814 sensor");
  }
  }
void GasSensor(){
  //delay(3000);
    Serial.println("GasSensor");
  if (sensorConnected) {
    // Print live values
    Serial.print("CO : ");
    Serial.print(sensor.measureCO());
    ThingSpeak.setField(3,sensor.measureCO() );
    Serial.print(" \t");
    Serial.print("NO2 : ");
    Serial.print(sensor.measureNO2());
    ThingSpeak.setField(4,sensor.measureNO2());
    Serial.print("\t");
    Serial.print("NH3 : ");
    Serial.println(sensor.measureNH3());
    ThingSpeak.setField(5,sensor.measureNH3());
  }
  }
void DhtSensor(){
    //delay(1000);
    Serial.println("DhtSensor");
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
   ThingSpeak.setField(1,h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
int  temp=int(t);
  ThingSpeak.setField(2,t);
//   ThingSpeak.setField(2,temp);
  Serial.print(F("°C "));
  //Serial.print(f);
  Serial.print("\n");
}
void PmSensor(){
  //delay(1000);
  Serial.println("PmSensor");
error = my_sds.read(&p25,&p10);
  if (! error) {
    Serial.print("P2.5: "+String(p25)+"\t");
     ThingSpeak.setField(6,String(p25));
    Serial.println("P10:  "+String(p10));
     ThingSpeak.setField(7,String(p10));
  }
}
  void loop() {
   if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected");
  }
  delay(1000);
  GasSensor();
  DhtSensor();
  PmSensor(); 
  //delay(1000);
  Serial.println("\n\n"); 
  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
   
 delay(15000);
  }
