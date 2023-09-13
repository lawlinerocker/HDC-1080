#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#include <WebServer.h>
#include <Wire.h>
#include <NTPClient.h>
const char* ssid = "test";
const char* password = "12345678";
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
WebServer server(80);

#define DATABASE_URL "https://projectembeded3-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define USER_EMAIL "test@gmail.com"
#define USER_PASSWORD "123456"

// WiFi Access Config
#define WIFI_SSID "HUAWEI P30"
#define WIFI_PASSWORD "aabbcc11123" 

#define API_KEY "AIzaSyBXaWBweUFj274vs3-Kxj1bVl4JD9-qJfc"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String formattedDate;
String dayStamp;
String timeStamp;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;


String tempPath = "/temperature";
String humPath = "/humidity";
String parentPath;

double temperature;
double humidity;
int result;
FirebaseJson json;

void test(){
  server.send(200,"text/html",SendHTML());
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);
  
  WiFi.softAP(ssid,password);
  WiFi.softAPConfig(local_ip,gateway,subnet);
  Serial.print("IP Address:");
  Serial.println(WiFi.softAPIP());
  delay(100);

  // put your setup code here, to run once:
  Wire.begin(SDA,SCL);
  Wire.beginTransmission(0x40);
  Wire.write(0x00);
  Wire.write(0x02);
  Wire.endTransmission();
  delay(20);
  
  server.on("/",test);
  server.begin();
  Serial.println("HTTP server started");


  timeClient.begin();
  //GMT+7 ------ +1=3600 -1=-3600
  timeClient.setTimeOffset(25200);
}

String tempfullPath;
String humfullPath;

void loop() {
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  formattedDate = timeClient.getFormattedDate();
  //2018-05-02T13:00:27Z
//  Serial.println(formattedDate);
  int splitT = formattedDate.indexOf("T");
//  Serial.println(splitT);
  dayStamp = formattedDate.substring(0, splitT);
  //Serial.print("DATE: ");
//  Serial.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
//  Serial.println(timeStamp);
  //Serial.print("HOUR: ");
  
  server.handleClient();

  // put your main code here, to run repeatedly:
  humidity=readSensor(&temperature);
  //  Serial.println("Start");
//  temperature = 30;
//  humidity = 50;

  Serial.print("\n%");
  Serial.printf("Relative Humidity(RH): %.2f",humidity);
  Serial.println("%");
  Serial.printf("Temperature(°C): %.2f°C\n",temperature);
  delay(1000);
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0))
  {
 sendDataPrevMillis = millis();
    parentPath= String(dayStamp) + "/" + String(timeStamp);
//    json.set(tempPath.c_str(), String(temperature));
//    json.set(humPath.c_str(), String(humidity));
//    Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json);
    tempfullPath = parentPath + "/temperature";
    humfullPath = parentPath + "/humidity";
    Firebase.setDouble(fbdo, F("/now/temperature"),temperature);
    Firebase.setDouble(fbdo, F("/now/humidity"),humidity);
    Firebase.setDouble(fbdo, tempfullPath ,temperature);
    Firebase.setDouble(fbdo, humfullPath ,humidity);
//    temperature = Firebase.getDouble(fbdo, tempfullPath);
//    humidity = Firebase.getDouble(fbdo, humfullPath);
  }
}




String SendHTML(){
  String ptr = "<!DOCTYPE html><html>\n";
  ptr += "<head><meta http-equiv=\"refresh\" content=\"3\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Header</title></head>\n";
  ptr += "<style>html {font-family: Helvetica; display: inline-block; margin: 0px auto; text-align:center;}\n";
  ptr += "</style>\n";
  ptr += "<h1>ESP32 Web Server</h1>\n";
  ptr += "<h2>Temperature : ";
  ptr += temperature;
  ptr += " C";
  ptr += "</h2>\n\n";
  ptr += "<h2>Humidity : ";
  ptr += humidity;
  ptr += " %\n";
  ptr += "</html>";
  return ptr;
}

double readSensor(double *temperature)
{
  ///hold 2 bytes from i2c 0-255
  uint8_t Byte[4];
  ///all contents temp registers -32768-32767
  uint16_t tmp;
  /// all contents humid reg
  uint16_t humidity;
  Wire.beginTransmission(0x40);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(20);
  //request from hdc1080 -> 4 bytes
  Wire.requestFrom(0x40,4);
  delay(1);
  //check if returned successed
  if(4<=Wire.available())
  {
    //byte 0 contains upper byte <-- temp
    Byte[0]=Wire.read();
//    Serial.printf("\nByte 0 %d",Byte[0]);
    //byte 1 contains lower byte <-- temp
    Byte[1]=Wire.read();
//    Serial.printf("\nByte 1 %d",Byte[1]);
    //byte 3 contains upper byte <-- humid
    Byte[3]=Wire.read();
//    Serial.printf("\nByte 3 %d",Byte[3]);
    //byte 4 contains lower byte <-- humid
    Byte[4]=Wire.read();
//    Serial.printf("\nByte 4 %d",Byte[4]);

    //byte 0 + byte 1 =lower+upper --> 16 bit int
    tmp=(((unsigned int)Byte[0]<<8 | Byte[1]));
//    Serial.printf("\ntmp: %d",tmp);
    //calculate--> temp=(temp/2^16) *165°C-40°C
    *temperature=(double)(tmp)/(65536)*165-40;
    //byte 3 + byte 4=lower+upper --> 16 bit int
    humidity=(((unsigned int)Byte[3]<<8 | Byte[4]));
//    Serial.printf("\nhumid: %d",humidity);
    //calculate humid=(humid/2^16)*100%
    return (double)(humidity)/(65536)*100;
  }
}
