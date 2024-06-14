// Definição dos pinos de LED e LDR
int ledPin = 25;
int ldrPin = 36;

int ledValue = 10; // Valor inicial do LED
int ldrMax = 4000; // Valor máximo do LDR

void setup() {
    Serial.begin(9600);
    
    pinMode(ledPin, OUTPUT);
    pinMode(ldrPin, INPUT);
    Serial.printf("SmartLamp Initialized.\n");
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
            ledValue = value; //Normaliza o valor da  intensidade do led entre 0 e 255 
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
        Serial.println(ledValue);
    } else if (command.startsWith("GET_LDR")) {
        Serial.print("RES GET_LDR ");
        int ldrValue = ldrGetValue();
        Serial.println(map(ldrValue, 0, ldrMax, 0, 100));
    } else {
        Serial.println("ERR Unknown command.");
    }
}

// Função para atualizar o valor do LED
void ledUpdate() {
    analogWrite(ledPin, map(ledValue,0,100,0,255));
}

// Função para ler o valor do LDR
int ldrGetValue() {
    return analogRead(ldrPin);
}