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

  driver.setThisAddress(2);  // Define o ID deste dispositivo como 2

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // LED começa desligado
}

void loop() {
  uint8_t buf[32];
  uint8_t buflen = sizeof(buf);
  int lastSendTime = millis();
  int sendInterval = 50;

  // Escuta por um intervalo de tempo definido
  while (millis() - lastSendTime <= sendInterval) {
    if (driver.recv(buf, &buflen)) {
      buf[buflen] = '\0';  // Garante que o buffer seja uma string válida
      // Serial.print("[");
      // Serial.print(millis());
      // Serial.print(" ms] Recebido: ");
      // Serial.println((char*)buf);
      
      digitalWrite(LED_BUILTIN, HIGH);
    
      // Verifica o conteúdo recebido e envia a resposta apropriada
      if (strcmp((char*)buf, "U") == 0) {
        const char *msg = "A";
        // Descomente a linha abaixo se desejar logar o envio com timestamp:
        //  Serial.print("["); Serial.print(millis()); Serial.println(" ms] Enviando resposta: A");
        driver.send((uint8_t *)msg, strlen(msg));
        driver.waitPacketSent();
        // delay(25);
      }
      else if (strcmp((char*)buf, "X") == 0) {
        const char *msg = "V";
        // Descomente a linha abaixo se desejar logar o envio com timestamp:
        //  Serial.print("["); Serial.print(millis()); Serial.println(" ms] Enviando resposta: V");
        driver.send((uint8_t *)msg, strlen(msg));
        driver.waitPacketSent();
      }
    }
  }
  
  // Delay opcional para estabilizar a comunicação, se necessário
  // delay(100);
  
  digitalWrite(LED_BUILTIN, LOW);
}
