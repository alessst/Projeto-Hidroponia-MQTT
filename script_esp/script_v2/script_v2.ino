#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Definição dos pinos dos LEDs RGB
const int redPin1 = 12;
const int greenPin1 = 13;
const int bluePin1 = 15;

const int redPin2 = 25;
const int greenPin2 = 26;
const int bluePin2 = 27;

// Pino do sensor de temperatura DS18B20
const int oneWireBus = 4;

// Pino do sensor de umidade do solo
const int soilMoisturePin = 34;

// Pino do relé
const int relayPin = 32;

// Pino do botão
const int buttonPin = 33;
bool relayState = false;

// Inicializa o sensor OneWire e a biblioteca DallasTemperature
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// Configuração do display OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Credenciais Wi-Fi
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Endereço e porta do servidor MQTT
const char* mqtt_server = "your_mqtt_server";
const int mqtt_port = 1883;

// Inicialize o cliente WiFi e o cliente MQTT
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600); // Inicializa a comunicação serial

  // Inicializa o display OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Falha ao inicializar o display SSD1306"));
    for(;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  // Inicializa o sensor DS18B20
  sensors.begin();

  // Configura os pinos dos LEDs RGB como saída
  pinMode(redPin1, OUTPUT);
  pinMode(greenPin1, OUTPUT);
  pinMode(bluePin1, OUTPUT);

  pinMode(redPin2, OUTPUT);
  pinMode(greenPin2, OUTPUT);
  pinMode(bluePin2, OUTPUT);

  // Configura o pino do relé como saída
  pinMode(relayPin, OUTPUT);

  // Configura o pino do botão como entrada
  pinMode(buttonPin, INPUT_PULLUP);

  // Inicializa os estados dos LEDs como desligados
  digitalWrite(redPin1, LOW);
  digitalWrite(greenPin1, LOW);
  digitalWrite(bluePin1, LOW);

  digitalWrite(redPin2, LOW);
  digitalWrite(greenPin2, LOW);
  digitalWrite(bluePin2, LOW);

  // Define o estado inicial do relé
  digitalWrite(relayPin, relayState ? HIGH : LOW);

  // Conecta ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Configura o servidor MQTT
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT server");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop() {
  // Verifica o estado do botão
  if (digitalRead(buttonPin) == LOW) {
    // Inverte o estado do relé quando o botão é pressionado
    relayState = !relayState;
    digitalWrite(relayPin, relayState ? HIGH : LOW);
    delay(250); // Delay pequeno para evitar o efeito de bouncing
  }

  // Atualiza a leitura do sensor de temperatura
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  Serial.print("Temperatura: ");
  Serial.print(temperatureC);
  Serial.println(" C");

  // Lê o valor do sensor de umidade do solo
  int soilMoistureValue = analogRead(soilMoisturePin);
  Serial.print("Nivel de Agua: ");
  Serial.println(soilMoistureValue);

  // Controla o LED RGB 1 com base na temperatura
  if (temperatureC < 15.0) {
    digitalWrite(redPin1, LOW);
    digitalWrite(greenPin1, LOW);
    digitalWrite(bluePin1, HIGH);
  } else if (temperatureC >= 15.0 && temperatureC <= 25.0) {
    digitalWrite(redPin1, LOW);
    digitalWrite(greenPin1, HIGH);
    digitalWrite(bluePin1, LOW);
  } else if (temperatureC > 25.0) {
    digitalWrite(redPin1, HIGH);
    digitalWrite(greenPin1, LOW);
    digitalWrite(bluePin1, LOW);
  }

  // Controla o LED RGB 2 com base no nível de água
  if (soilMoistureValue < 2000) {
    digitalWrite(redPin2, LOW);
    digitalWrite(greenPin2, HIGH);
    digitalWrite(bluePin2, LOW);
  } else if (soilMoistureValue >= 2000 && soilMoistureValue < 2300) {
    digitalWrite(redPin2, LOW);
    digitalWrite(greenPin2, LOW);
    digitalWrite(bluePin2, HIGH);
  } else {
    digitalWrite(redPin2, HIGH);
    digitalWrite(greenPin2, LOW);
    digitalWrite(bluePin2, LOW);
  }

  // Atualiza o display OLED com a temperatura e o nível de água
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print("Estado dos LEDs:");
  
  display.setCursor(0,20);
  display.print("Temp:");
  display.println(temperatureC);
  
  display.setCursor(0,30);
  display.print("Nivel de Agua:");
  display.println(soilMoistureValue);

  display.setCursor(0,40);
  display.print("Relé: ");
  display.println(relayState ? "Desligado" : "ligado");

  display.display();

  // Verifica o estado do cliente MQTT
  if (!client.connected()) {
    while (!client.connected()) {
      if (client.connect("ESP32Client")) {
        Serial.println("Reconnected to MQTT server");
      } else {
        Serial.print("Failed, rc=");
        Serial.print(client.state());
        delay(2000);
      }
    }
  }
  client.loop();

  // Envia os dados para o servidor MQTT
  char temperatureStr[8];
  dtostrf(temperatureC, 1, 2, temperatureStr);
  char moistureStr[8];
  itoa(soilMoistureValue, moistureStr, 10);

  client.publish("sensor/temperature", temperatureStr);
  client.publish("sensor/moisture", moistureStr);

  delay(1000); // Atualiza a cada 1 segundo
}
