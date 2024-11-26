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

// Define o pino para o sensor infravermelho e os LEDs de marcação
const int irSensorPin = 33;  // Pino do sensor IR
const int ledGreenPin = 14;  // Pino do LED verde (lixeira vazia)
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
  // Verifica se a lixeira está cheia pelo sensor IR
  bool isFull = digitalRead(irSensorPin) == HIGH;

  if (isFull) {
    // Lixeira cheia: Acende o LED vermelho e apaga o LED verde
    digitalWrite(ledRedPin, HIGH);
    digitalWrite(ledGreenPin, LOW);

    // Impede o movimento do servo quando a lixeira está cheia
    servo_13.write(0); // Garante que o servo permaneça na posição fechada
  } else {
    // Lixeira disponível: Acende o LED verde e apaga o LED vermelho
    digitalWrite(ledRedPin, LOW);
    digitalWrite(ledGreenPin, HIGH);

    // Calcula a distância em centímetros se a lixeira está disponível
    distancia = 0.01723 * readUltrasonicDistance(25, 26);

    // Move o servo se a distância for menor ou igual a 50 cm
    if (distancia <= 50) {
      servo_13.write(90);  // Muda o servo para 90°
      delay(5000);         // Espera por 5000 milissegundos
      servo_13.write(0);   // Retorna o servo para 0°
      delay(1000);         // Espera um segundo antes de voltar a medir
    }
  }

  delay(200);  // Pequeno atraso para estabilizar o loop
}
