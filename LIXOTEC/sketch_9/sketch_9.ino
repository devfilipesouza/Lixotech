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

float distancia = 0;
bool lastIsFull = false;  // Variável para registrar o estado anterior da lixeira

// Pinos do sensor ultrassônico
const int triggerPin1 = 25;  // Sensor ultrassônico 1 (para medir o nível)
const int echoPin1 = 26;     // Sensor ultrassônico 1

const int triggerPin2 = 32;  // Sensor ultrassônico 2 (para detectar lixeira cheia)
const int echoPin2 = 33;     // Sensor ultrassônico 2

// Função para ler a distância de um sensor ultrassônico no ESP32
long readUltrasonicDistance(int triggerPin, int echoPin) {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  return pulseInLong(echoPin, HIGH);
}

Servo servo_13;

const int ledGreenPin = 14;  // Pino do LED verde (lixeira disponível)
const int ledRedPin = 27;    // Pino do LED vermelho (lixeira cheia)

void connectToWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.print("Conectando-se à rede Wi-Fi");
    
    // Aguarda até que a conexão seja estabelecida ou falhe após 10 segundos
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      Serial.print(".");
      delay(500);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConectado ao Wi-Fi!");
      Serial.print("Endereço IP: ");
      Serial.println(WiFi.localIP());
      
      // Inicializa o ThingSpeak apenas se houver conexão
      ThingSpeak.begin(client);
    } else {
      Serial.println("\nFalha ao conectar ao Wi-Fi.");
    }
  }
}

void checkLixeiraStatus() {
  // Verifica a distância do sensor 2 (sensor para detectar lixeira cheia)
  float distancia2 = 0.01723 * readUltrasonicDistance(triggerPin2, echoPin2); // Distância do sensor 2

  if (distancia2 <= 2) {  // Se a distância for menor ou igual a 2 cm, lixeira cheia
    Serial.println("Lixeira está cheia.");
    digitalWrite(ledRedPin, HIGH);  // Acende o LED vermelho
    digitalWrite(ledGreenPin, LOW); // Apaga o LED verde
  } else {
    Serial.println("Lixeira está vazia.");
    digitalWrite(ledGreenPin, HIGH);  // Acende o LED verde
    digitalWrite(ledRedPin, LOW);    // Apaga o LED vermelho
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000); // Atraso para garantir que o Serial Monitor esteja pronto
  
  Serial.println("Inicializando...");
  connectToWiFi();
  
  pinMode(triggerPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  
  pinMode(triggerPin2, OUTPUT);  // Sensor ultrassônico 2
  pinMode(echoPin2, INPUT);      // Sensor ultrassônico 2

  pinMode(ledGreenPin, OUTPUT);  // Configura o LED verde
  pinMode(ledRedPin, OUTPUT);    // Configura o LED vermelho

  servo_13.attach(13, 500, 2500);
  servo_13.write(0);  // Inicializa o servo fechado
  delay(1000);

  // Verifica e imprime o status inicial da lixeira
  checkLixeiraStatus();
}

void loop() {
  // Tenta reconectar ao Wi-Fi se estiver desconectado
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado. Tentando reconectar...");
    connectToWiFi();
  }

  // Lê a distância do sensor 1 (nível da lixeira) e abre a lixeira se necessário
  distancia = 0.01723 * readUltrasonicDistance(triggerPin1, echoPin1); // Distância do sensor 1

  // Se a lixeira não estiver cheia, acende o LED verde e mantém o LED vermelho apagado
  if (distancia > 20) {
    digitalWrite(ledGreenPin, HIGH);  // Liga o LED verde (lixeira disponível)
    digitalWrite(ledRedPin, LOW);     // Desliga o LED vermelho
  } else {
    // Se a lixeira estiver cheia, acende o LED vermelho e mantém o verde apagado
    digitalWrite(ledGreenPin, LOW);   // Desliga o LED verde
    digitalWrite(ledRedPin, HIGH);    // Liga o LED vermelho
  }

  // Lê a distância do sensor 2 para detectar quando a lixeira estiver cheia (2 cm)
  float distancia2 = 0.01723 * readUltrasonicDistance(triggerPin2, echoPin2); // Distância do sensor 2

  if (distancia2 <= 2) {  // Se a distância for menor ou igual a 2 cm, lixeira cheia
    digitalWrite(ledRedPin, HIGH);  // Acende o LED vermelho (lixeira cheia)
    servo_13.write(0);  // Mantém o servo fechado

    // Envia dados para o ThingSpeak apenas se o estado mudou para cheio e o Wi-Fi está conectado
    if (!lastIsFull && WiFi.status() == WL_CONNECTED) {
      int x = ThingSpeak.writeField(myChannelNumber, 1, 1, myWriteAPIKey);
      if (x == 200) {
        Serial.println("Status de lixeira cheia enviado para o ThingSpeak");
      } else {
        Serial.println("Erro ao enviar status de lixeira cheia");
      }
      lastIsFull = true;
    }

    // Imprime no Serial Monitor que a lixeira está cheia
    Serial.println("Lixeira cheia!");

    delay(10000);  // Espera para evitar leituras excessivas
  } else {
    // Se a lixeira não estiver cheia, o LED vermelho é apagado
    digitalWrite(ledRedPin, LOW);  // Desliga o LED vermelho
  }

  delay(200);  // Pequeno atraso para estabilizar o loop
}
