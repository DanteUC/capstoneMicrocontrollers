
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define FAN_PIN 4          
// const char* ssid = "Fernando-Arias";
// const char* password = "Ferma1011";
const char* ssid = "DantePhone";
const char* password = "capstone";
//WILL CHANGE EVERY TIME
//const char* mqtt_server = "192.168.137.33";
const char* mqtt_server = "192.168.254.201";

WiFiClient fanSpeedClient;
PubSubClient client(fanSpeedClient);
long lastMsg = 0;
char msg[50];
float targetAirflow=0;
float currentAirflow=0;
int currentPWM=0;

//LEDPIN 4 IS ACTUALLY LABELLED PIN 2
                         

void setup() {
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(FAN_PIN, OUTPUT);
  Serial.begin(11520);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();


  if (String(topic) == "currentAirflow") {
    printf("message received on currentAirflow");
    currentAirflow = messageTemp.toFloat();
    
  }else if(String(topic) == "targetAirflow"){
    printf("message received on targetAirflow");
    targetAirflow = messageTemp.toFloat();
    
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("fanSpeedClient")) {
      Serial.println("connected");
      client.subscribe("targetAirflow");
      client.subscribe("currentAirflow");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //ADJUST FANSPEED HERE 
  if(currentAirflow < targetAirflow && currentPWM < 255){
    currentPWM=currentPWM + 5;
    analogWrite(FAN_PIN, currentPWM);
  }else if (currentAirflow/targetAirflow >= 1.1 && currentPWM >0){
    currentPWM = currentPWM - 5;
    analogWrite(FAN_PIN, currentPWM);
  }
  Serial.println("current airflow: "+ String(currentAirflow));
  Serial.println("target airflow: "+ String(targetAirflow));
  Serial.println("pwm: "+ String(currentPWM));
  delay(1000);
}
