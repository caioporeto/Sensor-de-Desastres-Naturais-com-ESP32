#include <RH_ASK.h>
#include <SPI.h>

#define LED_BUILTIN 2

RH_ASK driver(2000, 4, 17, -1);

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
  int lastSendTime = millis();
  int sendInterval = 400;

  while(millis() - lastSendTime <= sendInterval){
    if (driver.recv(buf, &buflen)) {
      buf[buflen] = '\0';  // Garante que o buffer seja uma string válida
      Serial.print("Got: ");  
      Serial.println((char*)buf);
      digitalWrite(LED_BUILTIN, HIGH);
      // Se a mensagem recebida for "V", imprime "ACABOU"
      if (strcmp((char*)buf, "V") == 0) {
        Serial.println("ACABOU");
        while(true);
      }
    }
  }
  
  digitalWrite(LED_BUILTIN, LOW);
  const char *msg = "U";
  Serial.println("Enviando mensagem...");
  driver.send((uint8_t *)msg, strlen(msg));
  driver.waitPacketSent();  // Aguarda o envio completo do pacote
  // delay(50);  // Delay para permitir que o módulo retorne ao modo RX

}