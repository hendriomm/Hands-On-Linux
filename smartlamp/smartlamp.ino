#include <DHT.h>

#define DHTPIN 15       
#define DHTTYPE DHT11

// Definição dos pinos de LED e LDR
int ledPin = 25;
int ldrPin = 36;

int ledValue = 10; // Valor inicial do LED
int ldrMax = 4000; // Valor máximo do LDR

DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(9600);
    
    pinMode(ledPin, OUTPUT);
    pinMode(ldrPin, INPUT);
    Serial.printf("SmartLamp Initialized.\n");

   processCommand("GET_LDR");
}

// Função loop será executada infinitamente pelo ESP32
void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    processCommand(command);
  }    
}

void processCommand(String command) {
    if (command.startsWith("SET_LED")) {
        int value = command.substring(8).toInt(); //Pega o valor para  representar a intensidade de luminosidade do led 
        if (value >= 0 && value <= 100) {
            ledValue = map(value, 0, 100, 0, 255); //Normaliza o valor da  intensidade do led entre 0 e 255 
            //Serial.print("Valor normalizado: ");
            //Serial.println(ledValue);
            ledUpdate();
            Serial.print("RES SET_LED "); //Valor inserido está no intervalo 0 a 100
            Serial.println(value);
        } else {
            Serial.println("RES SET_LED -1"); //Entrada inválida do comando SET_LED
        }
    } else if (command.startsWith("GET_LED")) {
        Serial.print("RES GET_LED ");
        Serial.println(map(ledValue, 0, 255, 0, 100));
    } else if (command.startsWith("GET_LDR")) {
        Serial.print("RES GET_LDR ");
        int ldrValue = ldrGetValue();
        Serial.println(map(ldrValue, 0, ldrMax, 0, 100));
    } else if(command.startsWith("GET_TEMP")){
        Serial.print("RES GET_TEMP ");
        int tempDHT = getTemp();
        Serial.println(tempDHT);
    } else if(command.startsWith("GET_HUM")){
        Serial.print("RES GET_HUM ");
        int humDHT = getHum();
        Serial.println(humDHT);
    } else {
        Serial.println("ERR Unknown command.");
    }
}

float getHum(){
  return dht.readHumidity();
}

float getTemp(){
  return dht.readTemperature();
}

// Função para atualizar o valor do LED
void ledUpdate() {
    analogWrite(ledPin, ledValue);
}

// Função para ler o valor do LDR
int ldrGetValue() {
    return analogRead(ldrPin);
}
