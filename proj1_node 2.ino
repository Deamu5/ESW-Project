#include "ThingSpeak.h"
#include <ESP8266WiFi.h>
#include "DHT.h"
#include <MiCS6814-I2C.h>
#define DHTPIN D3     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#include <SDS011.h>
#include <TinyGPS.h>
float p10,p25;
int error;
//test server ip - 139.59.42.21
String CSE_IP      = "onem2m.iiit.ac.in";
// oneM2M : CSE params
int   CSE_HTTP_PORT = 80;
String CSE_NAME    = "in-name";
String CSE_M2M_ORIGIN  = "admin:admin";

int TY_CI  = 4;
int REQUEST_TIME_OUT = 5000; //
String st;


TinyGPS gps;
SoftwareSerial ss(7, 8);
  
static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void print_float(float val, float invalid, int len, int prec)
{
  if (val == invalid)
  {
    while (len-- > 1){
      Serial.print('*');
      st = st + '*';
    }
    st = st + ',';
    
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    st = st+ String(val) + ',';
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartdelay(0);
}

static void print_date(TinyGPS &gps)
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE) {
    Serial.print("********** ******** ");
    st = st + ("********,");
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
        month, day, year, hour, minute, second);
    Serial.print(sz);
    //ThingSpeak.setField(6, sz);
    
    st  = st + String(sz) + ',';
    
  }
  smartdelay(0);
}

//ssid -esw-m19@iiith
//pass -e5W-eMai@3!20hOct
#define SECRET_SSID "tarmac"   
#define SECRET_PASS "12345678"
  
//#define SECRET_WRITE_APIKEY "A7YDK1O23YQ9LO68"
#define SECRET_WRITE_APIKEY "IB5GN3CG3HFZCZ7A"
SDS011 my_sds;  
DHT dht(DHTPIN, DHTTYPE);
MiCS6814 sensor;
bool sensorConnected;
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
WiFiClient  client;

//unsigned long myChannelNumber=868883;
unsigned long myChannelNumber =897898;

const char myWriteAPIKey[] = SECRET_WRITE_APIKEY;




String doPOST(String url, int ty, String rep) {

  String postRequest = String() + "POST " + url + " HTTP/1.1\r\n" +
                       "Host: " + CSE_IP + ":" + CSE_HTTP_PORT + "\r\n" +
                       "X-M2M-Origin: " + CSE_M2M_ORIGIN + "\r\n" +
                       "Content-Type: application/json;ty=" + ty + "\r\n" +
                       "Content-Length: " + rep.length() + "\r\n"
                       "Connection: close\r\n\n" +
                       rep;

  // Connect to the CSE address

  Serial.println("connecting to " + CSE_IP + ":" + CSE_HTTP_PORT + " ...");

  // Get a client
  WiFiClient client2;
  if (!client2.connect(CSE_IP, CSE_HTTP_PORT)) {
    Serial.println("Connection failed !");
    return "error";
  }

  // if connection succeeds, we show the request to be send
#ifdef DEBUG
  Serial.println(postRequest);
#endif

  // Send the HTTP POST request
  client2.print(postRequest);

  // Manage a timeout
  unsigned long startTime = millis();
  while (client2.available() == 0) {
    if (millis() - startTime > REQUEST_TIME_OUT) {
      Serial.println("Client Timeout");
      client2.stop();
      return "error";
    }
  }

  // If success, Read the HTTP response
  String result = "";
  if (client2.available()) {
    result = client2.readStringUntil('\r');
    //    Serial.println(result);
  }
  while (client2.available()) {
    String line = client2.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection...");
  return result;
}


String createCI(String ae, String cnt, String ciContent) {
  String ciRepresentation =
    "{\"m2m:cin\": {"
    "\"con\":\"" + ciContent + "\""
    "}}";
  return doPOST("/" + CSE_NAME + "/" + ae + "/" + cnt, TY_CI, ciRepresentation);
}

void gps_Read() {
  float flat, flon;
  unsigned long age, date;
  
  gps.f_get_position(&flat, &flon, &age);
  print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
  ThingSpeak.setField(8,flat);
  ThingSpeak.setField(9,flon);
  print_date(gps);
}

void setup(){
  // Initialize serial connection
  Serial.begin(115200);
   WiFi.mode(WIFI_STA); 
   WiFi.disconnect();
  ThingSpeak.begin(client);  // Initialize ThingSpeak
Serial.println(WiFi.macAddress());

  my_sds.begin(D5,D6);
  dht.begin();
sensorConnected = sensor.begin();
//  
  if (sensorConnected == true) {
    sensor.powerOn();
    } else {
    Serial.println("Couldn't connect to MiCS-6814 sensor");
  }
}


void GasSensor(){
  delay(3000);
    Serial.println("GasSensor");
  float c;
  c = sensor.measureCO();
  Serial.print("Concentration of CO: ");
  if(c >= 0) Serial.println(c);
  else Serial.println("invalid");
  ThingSpeak.setField(3, c);
  st = st + String(c) + ',';
  c = sensor.measureNO2();
  Serial.print("Concentration of NO2: ");
  if(c >= 0) Serial.println(c);
  else Serial.println("invalid");
  ThingSpeak.setField(4, c);
  st = st + String(c) + ',';
  c = sensor.measureNH3();
  Serial.print("Concentration of NH3: ");
  if(c >= 0) Serial.println(c);
  else Serial.println("invalid");
  ThingSpeak.setField(5, c);
  st = st + String(c) + ',';
  }
void DhtSensor()
{
    //delay(1000);
    Serial.println("DhtSensor");
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print(F("Humidity: "));
  Serial.print(h);
 
   ThingSpeak.setField(1,h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
int  temp=int(t);
  ThingSpeak.setField(2,t);
  //ThingSpeak.setField(2,temp);
  Serial.print(F("°C "));
  //Serial.print(f);
  Serial.print("\n");
st=st+String(t)+","+String(h)+",";
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
  st=st+String(p25)+","+String(p10)+",";
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
  st="";
  DhtSensor();
  PmSensor(); 
  GasSensor();
  //gps_Read();
  st = st + "OAP5_2";
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
  createCI("Team11_Outdoor_Air_Pollution_Mobile", "node_2", st);
//Serial.println(WiFi.macAddress());
 delay(15000);
  }
