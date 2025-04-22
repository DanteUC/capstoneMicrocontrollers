
#include <ESP8266WiFi.h>
#include <Adafruit_SCD30.h>
#include <PubSubClient.h>
#include <Wire.h> 

//Pins
#define PIR_PIN 5  // D1 pin for HC-SR501
#define SDA_PIN 4
#define SCL_PIN 0
Adafruit_SCD30 scd30;
// Replace the next variables with your SSID/Password combination
const char* ssid = "Fernando-Arias";
const char* password = "Ferma1011";

const char* mqtt_server = "192.168.137.33";

WiFiClient roomOccupancyClient;
PubSubClient client(roomOccupancyClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int occupancy = 0;
bool motionDetected = false;


void setup() {
  Serial.begin(115200); 
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(18, INPUT);
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!scd30.begin()) {
    Serial.println("Couldn't find SCD50 sensor");
    while (1);
  }
  Serial.println("SCD50 sensor initialized");

  Serial.println("Measurement Interval: ");
  Serial.println(scd30.getMeasurementInterval());
  Serial.print(" seconds");
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


  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "smartplug/state") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      //turn on;
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      //turn off;
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("OccupancyClient")) {
      Serial.println("connected");
      client.subscribe("smartplug/state");
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

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    //compute values here
    motionDetected = digitalRead(PIR_PIN);
    if (motionDetected) {
      Serial.println("Motion Detected!");
    } else {
      Serial.println("No motion detected");
    }
    float co2, temperature, humidity;
    // Read data from the SCD50 sensor
    if (scd30.dataReady()) {
      Serial.println("Data Available!");
      if(!scd30.read()){
        Serial.println("Error: Unable to read sensor data!");
        return;
      }
      Serial.println("CO2: ");
      Serial.println(scd30.CO2,3);
      Serial.println("ppm");
      Serial.println("");
      }else {
      Serial.println("No Data...");
      }

    // Delay between sensor readings
    delay(5000);
    occupancy = scd30.CO2/120;
    //end computation
    Serial.println(occupancy);
    client.publish("currentOccupancy", String(occupancy).c_str());

  }
}
