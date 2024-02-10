#include <FirebaseESP8266.h>
#include <Arduino_JSON.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#define FIREBASE_HOST "solar-cleanig-default-rtdb.asia-southeast1.firebasedatabase.app/" //Without http:// or https:// schemes
#define FIREBASE_AUTH "a9tL6X4S3DnL1u475GHVLJS2oel4H1Dnk0G4tauM"

#define WIFI_SSID "realme 7i"
#define WIFI_PASSWORD "12345678"

const int sensorIn = A0;



int mVperAmp = 185; // use 185 for 5A, 100 for 20A Module and 66 for 30A Module
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;


WiFiClientSecure client;
String GAS_ID = "AKfycbxAexDQMGG3xSYV3S2rgzeEaG8NbhmW0reDtRX1jmV054ITgO2ycA4NY1NslF1lJ19F";
const char* host = "script.google.com";
const int httpsPort =443;

//Define FirebaseESP8266 data object
FirebaseData firebaseData;
void setup()
{
  Serial.begin(115200);
  pinMode(A0, INPUT);
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

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
    client.setInsecure();
}
  void loop() 
  {  
 Voltage = getVPP();
 VRMS = (Voltage/2.0) *0.707; // sq root
 AmpsRMS = (VRMS * 1000)/mVperAmp;
 float Wattage = (220*AmpsRMS)-18; //Observed 18-20 Watt when no load was connected, so substracting offset value to get real consumption.
 Serial.print(AmpsRMS);
 Serial.println(" Amps RMS ");
 Serial.print(Wattage); 
 Serial.println(" Watt ");


 if (Firebase.pushInt(firebaseData, "/FirebaseIOT/AmpsRMS", AmpsRMS))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  if (Firebase.pushInt(firebaseData, "/FirebaseIOT/wattage", Wattage))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
 
  if (Firebase.pushInt(firebaseData, "/FirebaseIOT/VRMs", VRMS))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
  sendData(AmpsRMS, Wattage,VRMS);
  
 
  delay(2000);

}
float getVPP()
{
  float result;
  
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  
   uint32_t start_time = millis();

   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(sensorIn);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the maximum sensor value*/
           minValue = readValue;
       }
/*       Serial.print(readValue);
       Serial.println(" readValue ");
       Serial.print(maxValue);
       Serial.println(" maxValue ");
       Serial.print(minValue);
       Serial.println(" minValue ");
       delay(1000); */
    }
   
   // Subtract min from max
   result = ((maxValue - minValue) * 5)/1024.0;
      
   return result;
 }
 void sendData(double AmpsRMS, float  Wattage,double VRMS  ) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //----------------------------------------

  //----------------------------------------Processing data and sending data
  String string_AmpsRMS =  String(AmpsRMS);

  String string_Wattage =  String(Wattage); 
   String string_voltege=  String(VRMS); 
  
    
  
  String url = "/macros/s/" + GAS_ID + "/exec?AmpsRMS=" + string_AmpsRMS + "&Wattage=" +string_Wattage+"&voltage="+string_voltege;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino 
  }CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  /*
  function doGet(e) { 
  Logger.log( JSON.stringify(e) );
  var result = 'Ok';
  if (e.parameter == 'undefined') {
    result = 'No Parameters';
  }
  else {
    var sheet_id ='19zqzRkYGVAN6g5a5G3dQYAFU5H2Jf3kPUcdnYZrXhLc' ;   // Spreadsheet ID
    var sheet = SpreadsheetApp.openById(sheet_id).getActiveSheet();
    var newRow = sheet.getLastRow() + 1;            
    var rowData = [];
    
    for (var param in e.parameter) {
      Logger.log('In for loop, param=' + param);
      var value = stripQuotes(e.parameter[param]);
      Logger.log(param + ':' + e.parameter[param]);
      switch (param) {
        case 'temperature':
          rowData[1] = value; // Temperature in column C
          result = 'Temperature Written on column C'; 
          break;
        case 'humidity':
          rowData[2] = value; // Humidity in column D
          result += ' ,Humidity Written on column D'; 
          break;  
          case 'curent':
          rowData[3] = value; // Humidity in column D
          result += ' ,time Written on column e'; 
          break; 
            case 'voltage':
          rowData[3] = value; // Humidity in column D
          result += ' ,time Written on column e'; 
          break; 
         
        default:
          result = "unsupported parameter";
      }
    }
    Logger.log(JSON.stringify(rowData));
    var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
    newRange.setValues([rowData]);
  }
  return ContentService.createTextOutput(result);
}
function stripQuotes( value ) {
  return value.replace(/^["']|['"]$/g, "");
}

*/
  //----------------------------------------
} 
//=============================================
