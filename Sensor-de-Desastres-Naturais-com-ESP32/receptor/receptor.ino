RECEPTOR:
#include <RH_ASK.h>
#include <SPI.h>

#define LED_BUILTIN 2  // Pino do LED (geralmente 2)

RH_ASK driver(2000, 4, 17, -1);  // RX no pino 16 (GPIO 4), TX no pino 17

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
  const unsigned long sendInterval = 100;  // Tempo entre os envios (ms)
  // Escuta por determinado intervalo
  while( (millis() - lastSendTime < sendInterval) ) {
    if (driver.recv(buf, &buflen)) {
      buf[buflen] = '\0';  // Garante terminação nula
      String receivedMessage = String((char*)buf);

      // Serial.print("Recebido: ");
      Serial.println(receivedMessage);

      if (strcmp((char*)buf, "A") == 0) {
        // Serial.println("Enviando mensagem X...");
        const char *msg = "X";
        driver.send((uint8_t *)msg, strlen(msg));
        driver.waitPacketSent();
        digitalWrite(LED_BUILTIN, HIGH);
      }
    }
  }
 digitalWrite(LED_BUILTIN, LOW);
}