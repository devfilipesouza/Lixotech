#include <ESP32Servo.h>

float distancia = 0;

// Função para ler a distância do sensor ultrassônico no ESP32
long readUltrasonicDistance(int triggerPin, int echoPin)
{
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  // Usamos a função pulseInLong para maior precisão com o ESP32
  long duration = pulseInLong(echoPin, HIGH);
  return duration;
}

Servo servo_13;

// Define o pino para o sensor infravermelho e os LEDs indicadores
const int irSensorPin = 33;  // Pino do sensor IR
const int ledGreenPin = 14;  // Pino do LED verde (lixeira disponível)
const int ledRedPin = 27;    // Pino do LED vermelho (lixeira cheia)

void setup()
{
  // Define os pinos para o sensor ultrassônico
  int triggerPin = 25;
  int echoPin = 26;
  
  // Configura os modos dos pinos
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(irSensorPin, INPUT);      // Configura o pino do sensor IR como entrada
  pinMode(ledGreenPin, OUTPUT);     // Configura o pino do LED verde como saída
  pinMode(ledRedPin, OUTPUT);       // Configura o pino do LED vermelho como saída

  // Configura o servo no pino 13 e inicializa na posição 0°
  servo_13.attach(13, 500, 2500);
  servo_13.write(0);
  delay(1000);
}

void loop()
{
  // Calcula a distância em centímetros
  distancia = 0.01723 * readUltrasonicDistance(25, 26);

  // Verifica se a lixeira está cheia pelo sensor IR
  bool isFull = digitalRead(irSensorPin) == HIGH;

  if (isFull) {
    digitalWrite(ledRedPin, HIGH);   // Liga o LED vermelho (lixeira cheia)
    digitalWrite(ledGreenPin, LOW);  // Desliga o LED verde
  } else {
    digitalWrite(ledRedPin, LOW);    // Desliga o LED vermelho
    digitalWrite(ledGreenPin, HIGH); // Liga o LED verde (lixeira disponível)
  }

  if (distancia <= 50) {
    servo_13.write(90);  // Muda o servo para 90°
    delay(5000);         // Espera por 5000 milissegundos
    servo_13.write(0);   // Retorna o servo para 0°
    delay(1000);         // Espera um segundo antes de voltar a medir
  }

  delay(200);  // Pequeno atraso para estabilizar o loop
}
