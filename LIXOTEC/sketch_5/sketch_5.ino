#include <ESP32Servo.h>

float distancia = 0;

// Pinos do sensor ultrassônico
const int triggerPin = 25;
const int echoPin = 26;

Servo servo_13;

// Função para ler a distância do sensor ultrassônico no ESP32
long readUltrasonicDistance(int triggerPin, int echoPin) {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  // Usamos a função pulseInLong para maior precisão com o ESP32
  long duration = pulseInLong(echoPin, HIGH);
  return duration;
}

void setup() {
  Serial.begin(115200);

  // Configura os modos dos pinos
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Configura o servo no pino 13 e inicializa na posição 0°
  servo_13.attach(13, 500, 2500);
  servo_13.write(0);
  delay(1000);
}

void loop() {
  // Calcula a distância em centímetros
  distancia = 0.01723 * readUltrasonicDistance(triggerPin, echoPin);

  // Aciona o servo se a distância for menor ou igual a 10 cm
  if (distancia <= 10) {
    servo_13.write(90);  // Muda o servo para 90° (abrindo)
    delay(5000);         // Espera por 5000 milissegundos
    servo_13.write(0);   // Retorna o servo para 0° (fechando)
    delay(1000);         // Espera um segundo antes de voltar a medir
  }

  delay(200);  // Pequeno atraso para estabilizar o loop
}
