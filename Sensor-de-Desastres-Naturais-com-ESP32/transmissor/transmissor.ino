#include <RH_ASK.h>
#include <SPI.h>

#define LED_BUILTIN 2
#define trigPin 19
#define echoPin 18

int confirmacao = 1;

RH_ASK driver(2000, 4, 17, -1);

int distancia(){
  long duration;
  float distance = 20;

  while(distance >= 20){
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(trigPin, LOW);

    // Lê o tempo do pulso recebido no pino ECHO
    duration = pulseIn(echoPin, HIGH);

    // Calcula a distância (em cm)
    distance = duration * 0.034 / 2;
    Serial.println(distance);
  }
  return distance;
}

void setup() {  
  Serial.begin(9600);
  if (!driver.init()) {
    Serial.println("Erro ao iniciar RF");
  } else {
    Serial.println("RF iniciado com sucesso");
  }

  // driver.setThisAddress(1);  // Define o ID deste dispositivo como 1

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // LED começa desligado

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  uint8_t buf[32];
  uint8_t buflen = sizeof(buf);

  int lastSendTime = millis();
  int sendInterval = 400;

  if (confirmacao == 1){
    int dado = distancia();
    confirmacao = 0;
  }

  // Escuta por um intervalo de tempo definido
  while (millis() - lastSendTime <= sendInterval) {
    if (driver.recv(buf, &buflen)) {
      buf[buflen] = '\0';  // Garante que o buffer seja uma string válida
      // Serial.print("[");
      // Serial.print(millis());
      // Serial.print(" ms] Recebido: ");
      // Serial.println((char*)buf);
      
      digitalWrite(LED_BUILTIN, HIGH);
      // Se a mensagem recebida for "V", imprime "ACABOU" e interrompe o loop
      if (strcmp((char*)buf, "V") == 0) {
        Serial.print("[");
        Serial.print(millis());
        Serial.println(" ms] ACABOU");
        confirmacao = 1;
      }
    }
  }
  
  digitalWrite(LED_BUILTIN, LOW);
  const char *msg = "U";
  driver.send((uint8_t *)msg, strlen(msg));
  driver.waitPacketSent();  // Aguarda o envio completo do pacote
}
