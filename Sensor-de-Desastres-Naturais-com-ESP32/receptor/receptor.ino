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
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  uint8_t buf[32];
  uint8_t buflen = sizeof(buf);
  unsigned long lastSendTime = millis();
  const unsigned long sendInterval = 100;

  while ((millis() - lastSendTime < sendInterval)) {
    if (driver.recv(buf, &buflen)) {
      buf[buflen] = '\0';

      Serial.print("[");
      Serial.print(millis());
      Serial.print(" ms] Recebido: ");
      Serial.println((char*)buf);

      // Copia o buffer
      char bufferCopia[32];
      strcpy(bufferCopia, (char*)buf);

        char *id = strtok(bufferCopia, ":");
        char *dado = strtok(nullptr, "");

        if (id != nullptr && dado != nullptr) {
          if (strcmp(id, "2") == 0) {
            char resposta[50] = "3:";
            strcat(resposta, dado);

            driver.send((uint8_t *)resposta, strlen(resposta));
            driver.waitPacketSent();
            digitalWrite(LED_BUILTIN, HIGH);

            Serial.print("[");
            Serial.print(millis());
            Serial.print(" ms] Enviado: ");
            Serial.println(resposta);
          } else {
            Serial.print("[");
            Serial.print(millis());
            Serial.println(" ms] Ignorado: ID não é 2");
          }
        }
    }
  }

  digitalWrite(LED_BUILTIN, LOW);
}