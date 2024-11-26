#include <ESP32Servo.h>
#include <WiFi.h>

// Parâmetros de conexão Wi-Fi
const char* ssid = "IPhone de Filipe";  // nome da rede Wi-Fi
const char* password = "filipe2010";   // senha da rede Wi-Fi

float distancia = 0;

// Função para ler a distância do sensor ultrassônico no ESP32
long readUltrasonicDistance(int triggerPin, int echoPin)
{
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  // Função pulseInLong para maior precisão com o ESP32
  long duration = pulseInLong(echoPin, HIGH);
  return duration;
}

Servo servo_13;

// Define o pino para o sensor infravermelho e os LEDs indicadores
const int irSensorPin = 33;  // Pino do sensor IR
const int ledGreenPin = 14;  // Pino do LED verde (lixeira disponível)
const int ledRedPin = 27;    // Pino do LED vermelho (lixeira cheia)

void setup() {
  Serial.begin(115200);
  
  // Conexão ao Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando-se à rede Wi-Fi...");
  
  // Aguarda conexão com o Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("\nConectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Pinos do sensor ultrassônico
  int triggerPin = 25;
  int echoPin = 26;
  
  // Configuração dos modos dos pinos
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(irSensorPin, INPUT);      // Configura o pino do sensor IR como entrada
  pinMode(ledGreenPin, OUTPUT);     // Configura o pino do LED verde como saída
  pinMode(ledRedPin, OUTPUT);       // Configura o pino do LED vermelho como saída

  // Configuração do servo no pino 13 e estado inicial em 0
  servo_13.attach(13, 500, 2500);
  servo_13.write(0);
  delay(1000);
}

void loop() {
  // Verifica se a lixeira está cheia pelo sensor Infra
  bool isFull = digitalRead(irSensorPin) == HIGH;

  if (isFull) {
    // Lixeira cheia: Acende o LED vermelho e apaga o LED verde
    digitalWrite(ledRedPin, HIGH);
    digitalWrite(ledGreenPin, LOW);

    // Garante que o servo permaneça fechado
    servo_13.write(0);
  } else {
    // Lixeira disponível: Acende o LED verde e apaga o LED vermelho
    digitalWrite(ledRedPin, LOW);
    digitalWrite(ledGreenPin, HIGH);

    // Calcula a distância em centímetros somente se a lixeira estiver disponível
    distancia = 0.01723 * readUltrasonicDistance(25, 26);

    // Move o servo se a lixeira está disponível e a distância for adequada (<= 20 cm)
    if (distancia <= 20) {
      servo_13.write(90);  // Muda o servo para 90° (abrindo a lixeira)
      delay(5000);         // Espera por 5000 milissegundos com a lixeira aberta
      servo_13.write(0);   // Retorna o servo para 0° (fechando a lixeira)
      delay(1000);         // Espera um segundo antes de voltar a medir
    }
  }

  delay(200);  // Pequeno atraso para estabilizar o loop
}
