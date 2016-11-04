#include <ESP8266WiFi.h>
#include <PubSubClient.h>


#define RED_PIN D1
#define GREEN_PIN D2
#define BLUE_PIN D3
#define WHITE_PIN D4


// Update these with values suitable for your network.
const char* ssid = "****";
const char* password = "*****";
const char* mqtt_server = "192.168.1.15";



float red = 0.0;
float green = 0.0;
float blue = 0.0;
float white = 0.0;
bool fading = 0;
int targetN = 0;

int targetRed, targetBlue, targetGreen, targetWhite;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {

  Serial.begin(115200);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(WHITE_PIN, OUTPUT);

  analogWrite(RED_PIN, 0);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, 0);
  analogWrite(WHITE_PIN, 0);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
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


// This here defines what happens when a message is recieved
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println();

  String thistopic = String(topic);
  Serial.println(thistopic);

  // BLOCK COLOUR - rrrgggbbbwwwtt...ttt where 0<=rrr<=255 and t measured in miliseconds
  if (thistopic == "home/lights/kitchen/block") {
    Serial.println("Setting block color");
    Serial.println(message);

    targetRed = message.substring(0, 3).toInt();
    targetGreen = message.substring(3, 6).toInt();
    targetBlue = message.substring(6, 9).toInt();
    targetWhite = message.substring(9, 12).toInt();
    targetN = message.substring(12).toInt() / 10;
    if (targetN > 0) {
      fading = true;
    } else {

      analogWrite(RED_PIN, targetRed);
      analogWrite(GREEN_PIN, targetGreen);
      analogWrite(BLUE_PIN, targetBlue);
      analogWrite(WHITE_PIN, targetWhite);

      red = targetRed;
      green = targetGreen;
      blue = targetBlue;
      white = targetWhite;
    }


  }

  // Rainbow scroll

}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("kitchenlightsesp", "home/status/connections/kitchen", 0, false, "disconnected")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("home/status/connections/kitchen", "connected");
      // ... and resubscribe
      client.subscribe("home/lights/kitchen/#");
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
  if (fading) {

    int n = 0;
    float deltaRed = (targetRed - red) / targetN;
    float deltaGreen = (targetGreen - green) / targetN;
    float deltaBlue = (targetBlue - blue) / targetN;
    float deltaWhite = (targetWhite - white) / targetN;

    while (fading) {
      red += deltaRed;
      green += deltaGreen;
      blue += deltaBlue;
      white += deltaWhite;

      analogWrite(RED_PIN, (int)red);
      analogWrite(GREEN_PIN, (int)green);
      analogWrite(BLUE_PIN, (int)blue));
      analogWrite(WHITE_PIN, (int)white));

      n += 1;
      if (n >= targetN) {
        n = 0;
        fading = false;
      }
      delay(10);
      client.loop();
    }
  }
  yield(); //for good measure
}
