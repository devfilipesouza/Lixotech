#include <ESP32Servo.h>
#include <WiFi.h>
#include <ThingSpeak.h>

// Parâmetros de conexão Wi-Fi
const char* ssid = "filipe";
const char* password = "12345678";

// Parâmetros de conexão ThingSpeak
unsigned long myChannelNumber = 2743808;
const char* myWriteAPIKey = "UV06KDIC9LRS75O7";

WiFiClient client;

float distancia1 = 0;
float distancia2 = 0;
bool isFull = false;  // Estado atual da lixeira (cheia/vazia)

// Pinos do sensor ultrassônico
const int triggerPin1 = 25;  // Sensor ultrassônico 1 (nível da lixeira)
const int echoPin1 = 26;

const int triggerPin2 = 32;  // Sensor ultrassônico 2 (detecção de lixeira cheia)
const int echoPin2 = 33;

Servo servo_13;

const int ledGreenPin = 14;  // LED verde (lixeira disponível)
const int ledRedPin = 27;    // LED vermelho (lixeira cheia)

// Função para ler a distância de um sensor ultrassônico
long readUltrasonicDistance(int triggerPin, int echoPin) {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  return pulseInLong(echoPin, HIGH);
}

// Tenta conectar ao Wi-Fi (com limite de tempo)
void connectToWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.print("Conectando-se à rede Wi-Fi");

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      Serial.print(".");
      delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConectado ao Wi-Fi!");
      Serial.print("Endereço IP: ");
      Serial.println(WiFi.localIP());
      ThingSpeak.begin(client);
    } else {
      Serial.println("\nFalha ao conectar ao Wi-Fi. Operando offline.");
    }
  }
}

// Atualiza os LEDs com base no estado da lixeira
void updateLeds() {
  if (isFull) {
    digitalWrite(ledRedPin, HIGH);   // Acende o LED vermelho
    digitalWrite(ledGreenPin, LOW); // Apaga o LED verde
  } else {
    digitalWrite(ledGreenPin, HIGH);  // Acende o LED verde
    digitalWrite(ledRedPin, LOW);     // Apaga o LED vermelho
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000); // Atraso para garantir que o Serial Monitor esteja pronto
  
  Serial.println("Inicializando...");
  connectToWiFi();
  
  pinMode(triggerPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(triggerPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledRedPin, OUTPUT);

  servo_13.attach(13, 500, 2500);
  servo_13.write(0);  // Inicializa o servo fechado

  updateLeds();  // Atualiza os LEDs com o estado inicial da lixeira
}

void loop() {
  // Lê a distância do sensor 1 (nível da lixeira)
  distancia1 = 0.01723 * readUltrasonicDistance(triggerPin1, echoPin1);

  // Abre a lixeira se algo estiver a menos de 20 cm e a lixeira não estiver cheia
  if (distancia1 <= 20 && !isFull) {
    Serial.println("Movimento detectado! Abrindo a lixeira.");
    servo_13.write(90);  // Abre a lixeira
    delay(5000);         // Mantém a lixeira aberta por 5 segundos
    servo_13.write(0);   // Fecha a lixeira
  }

  // Lê a distância do sensor 2 (detecção de lixeira cheia)
  distancia2 = 0.01723 * readUltrasonicDistance(triggerPin2, echoPin2);

  // Atualiza o estado da lixeira (cheia/vazia) com limite de 7 cm
  bool currentIsFull = (distancia2 <= 7);  // Alterado de 2 cm para 7 cm
  if (currentIsFull != isFull) {
    isFull = currentIsFull;
    updateLeds();  // Atualiza os LEDs com o novo estado
    Serial.println(isFull ? "Lixeira cheia!" : "Lixeira disponível.");

    // Tenta enviar para o ThingSpeak apenas se houver conexão
    if (WiFi.status() == WL_CONNECTED) {
      int x = ThingSpeak.writeField(myChannelNumber, 1, isFull ? 1 : 0, myWriteAPIKey);
      Serial.println(x == 200 ? "Status enviado para o ThingSpeak." : "Erro ao enviar status.");
    }
  }

  // Garante que a lixeira permaneça fechada se estiver cheia
  if (isFull) {
    servo_13.write(0);  // Mantém a tampa fechada
  }

  delay(200);  // Pequeno atraso para estabilizar o loop
}