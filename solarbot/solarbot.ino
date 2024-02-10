#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define FIREBASE_HOST "harsha2-7efc2-default-rtdb.firebaseio.com/" //Without http:// or https:// schemes
#define FIREBASE_AUTH "kwBJuiP1UMCx14zLC1WLYjrxf2Li6QvozgkATVVE"
const char *ssid     = "H";
const char *password = "12345678";
int a1 =D0;
int a2=D1;
int b1=D2;
int b2=D3;
long trig1=D4;
long echo1=D5;
#define ENA   D8
const int trigPin = D6;
const int echoPin = D7;
#define ENB   A0

long duration;
float distanceCm;
#define SOUND_VELOCITY 0.034
long Zeit;
long Strecke;

FirebaseData firebaseData;
FirebaseData ledData;
FirebaseData led;
FirebaseJson json;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"in.pool.ntp.org", 19800, 60000);

void setup() {
Serial.begin(115200);

 pinMode(ENB, OUTPUT); 
 pinMode(ENA, OUTPUT); 
 pinMode(a1,OUTPUT);
    pinMode(a2,OUTPUT);
    pinMode(b1,OUTPUT);
    pinMode(b2,OUTPUT);
pinMode(trig1,OUTPUT);
pinMode(echo1,INPUT);
pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);
  
 WiFi.begin(ssid, password);
while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  
  timeClient.begin();
   
}

void loop() {
   timeClient.update();
   String a = timeClient.getFormattedTime();
   
  Firebase.getString(ledData, "/FirebaseIOT/man");
  Firebase.getInt(led, "/FirebaseIOT/speed"); 

  digitalWrite(trig1,LOW);
  delayMicroseconds(1);
  
  digitalWrite(trig1,HIGH);
  delayMicroseconds(5);
  digitalWrite(trig1,LOW);
  
  Zeit = pulseIn(echo1,HIGH);
  Strecke=Zeit*0.017;
    digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
duration = pulseIn(echoPin, HIGH);
distanceCm = duration * SOUND_VELOCITY/2;
Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
Serial.print(Strecke);
  
  String c =ledData.stringData();
  int speedCar  =led.stringData();

   Firebase.setString(firebaseData, "/FirebaseIOT/dis",Strecke);

  Serial.print(c);
  
bool b =  (a > "01:00:20" && a <=  "05:34:20") ;
if ((c == "1") ||  b  ){
  if (   Strecke > 10){
     

digitalWrite(a1, HIGH);
digitalWrite(a2, LOW); 
analogWrite(ENA, speedCar);
digitalWrite(b1, HIGH);
digitalWrite(b2, LOW);
 analogWrite(ENB, speedCar);
String t =  "forward";
Serial.println("on");
Serial.println(t);

 Firebase.setString(firebaseData, "/FirebaseIOT/status", t);
  }
 else {

digitalWrite(a1, LOW);
digitalWrite(a2,LOW); 
digitalWrite(b1, LOW);
digitalWrite(b2, LOW);
String t =  "stopped";
Serial.println(t);

Firebase.setString(firebaseData, "/FirebaseIOT/status", t);
   
 }
}
if (c == "0"){

digitalWrite(a1, LOW);
digitalWrite(a2, LOW); 
digitalWrite(b1, LOW);
digitalWrite(b2, LOW);
String t =  "stopped";

Serial.println(t);
Firebase.setString(firebaseData, "/FirebaseIOT/status", t);
 } 
if ((c == "2") ||  b){
  if ( distanceCm > 5){

     

digitalWrite(a1, LOW);
digitalWrite(a2, HIGH); 
analogWrite(ENA, speedCar);
digitalWrite(b1,LOW);
digitalWrite(b2, HIGH);
analogWrite(ENB, speedCar);
String t =  "back";
Serial.println("on");
Serial.println(t);

 Firebase.setString(firebaseData, "/FirebaseIOT/status", t);
  }
 if ( distanceCm <  10 ){

digitalWrite(a1, LOW);
digitalWrite(a2,LOW); 
digitalWrite(b1, LOW);
digitalWrite(b2, LOW);
String t =  "stopped";

Serial.println(t);

Firebase.setString(firebaseData, "/FirebaseIOT/status", t);
   
 }

}
}

    
