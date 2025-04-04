#include <RH_ASK.h>
#include <SPI.h>

#define LED_BUILTIN 2

RH_ASK driver(9600, 4, 17, -1);

void setup() {
  Serial.begin(9600);
  if (!driver.init()) {
    Serial.println("Erro ao iniciar RF");
  } else {
    Serial.println("RF iniciado com sucesso");
  }
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // LED começa desligado
}

void loop() {
  uint8_t buf[32];
  uint8_t buflen = sizeof(buf);
  
  if (driver.recv(buf, &buflen)) {
    buf[buflen] = '\0';  // Garante que o buffer seja uma string válida
    Serial.print("Recebido: ");
    Serial.println((char*)buf);
    digitalWrite(LED_BUILTIN, HIGH);
    
    // Verifica o conteúdo recebido e envia a resposta apropriada
    if (strcmp((char*)buf, "U") == 0) {
      const char *msg = "A";
      // Serial.println("Enviando resposta: A");
      driver.send((uint8_t *)msg, strlen(msg));
      driver.waitPacketSent();
      // delay(25);
    }
    else if (strcmp((char*)buf, "X") == 0) {
      const char *msg = "V";
      // Serial.println("Enviando resposta: V");
      driver.send((uint8_t *)msg, strlen(msg));
      driver.waitPacketSent();
    }
    
    // delay opcional, se necessário para estabilizar a comunicação
    // delay(100);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}
