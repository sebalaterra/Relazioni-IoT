#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <ArduinoJson.h>
#include <Process.h> //comandi shell sulla CPU Linux


const int TEMP_PIN = A1;
const int LED_PIN = 12;
const float B = 4275;
const float R0 = 100000;
const float T0 = 298.15;
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;
DynamicJsonDocument doc_snd(capacity);

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(TEMP_PIN, INPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(9600);
  Bridge.begin();
  digitalWrite(LED_PIN,HIGH);
}

void loop() {
  int n = postRequest(senMLEncode("temperature", readTemp(), "Cel"));
  
 if (n == 200){
    Serial.println("Tutto ok: " + String(n));
  }
  else{
    Serial.println("Errore: " + String(n));
  }
  
  delay(1000);  
}

int postRequest(String data){
  Process p; 
  Serial.println(data);
  p.begin("curl");
  p.addParameter("-H");
  p.addParameter("Content-Type: application/json");
  p.addParameter("-X");
  p.addParameter("POST");
  p.addParameter("-d");
  p.addParameter(data);
  p.addParameter("http://192.168.1.24:8080/log"); 
  p.run();

  return p.exitValue();
}

float readTemp(){
  int sig = analogRead(TEMP_PIN);
  float R = ((1023.0/(float)sig) - 1.0)*R0;
  float log_sig = log(R/R0);
  float T = 1/((log_sig/B) + (1/298.15));
  float temp = T - 273.15;
  return temp;
}


String senMLEncode(String res, float v, String unit){
  doc_snd.clear();
  doc_snd["bn"] = "Yun";
  if (unit!=""){
    doc_snd["e"][0]["u"] = unit;
  }
  else{
    doc_snd["e"][0]["u"] = (char*)NULL;
  }
  
  if (String(v)!=""){
    doc_snd["e"][0]["v"] = v;
  }
  else{
    doc_snd["e"][0]["v"] = (char*)NULL;
  }

  if (res!=""){
    doc_snd["e"][0]["n"] = res;
  }
  else{
    doc_snd["e"][0]["n"] = (char*)NULL;
  }

  doc_snd["e"][0]["t"] = String(millis()/1000.0) + "s";

  String output;
  serializeJson(doc_snd, output);
  return output;
}
