#include <RH_ASK.h>
#include <SPI.h>

#define LED_BUILTIN 2 // Led interno da ESP32

// Configura o driver para a comunicação RF
RH_ASK driver(2000, 4, 17, -1);

void setup() {
  // Inicializa a serial a 9600 bps
  Serial.begin(9600);

  // Inicializa o driver RF e avisa se falhar
  if (!driver.init()) {
    Serial.println("Erro ao iniciar RF");
  } else {
    Serial.println("RF iniciado com sucesso");
  }

  // Configura o led interno da ESP32
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // Led começa desligado
}

void loop() {
  uint8_t buf[32];
  uint8_t buflen = sizeof(buf);
  unsigned long lastSendTime = millis();
  const unsigned long sendInterval = 100; // Janela de recepção: 100 ms

  // Escuta por um intervalo de tempo definido
  while ((millis() - lastSendTime < sendInterval)) {
    if (driver.recv(buf, &buflen)) {
      buf[buflen] = '\0'; // Garante que o buffer seja uma string válida

      Serial.print("[");
      Serial.print(millis());
      Serial.print(" ms] Recebido: ");
      Serial.println((char*)buf);

      // Copia o buffer
      char bufferCopia[32];
      strcpy(bufferCopia, (char*)buf);

        char *id = strtok(bufferCopia, ":"); // Pega a parte antes dos dois-pontos
        char *dado = strtok(nullptr, ""); // Pega o restante da string

        if (id != nullptr && dado != nullptr) {
          // Filtra apenas mensagens com ID do remetente = 2
          if (strcmp(id, "2") == 0) {
            char resposta[50] = "3:";
            strcat(resposta, dado);

            driver.send((uint8_t *)resposta, strlen(resposta));
            driver.waitPacketSent();
            digitalWrite(LED_BUILTIN, HIGH); // Acende o led interno para indicar que enviou resposta

            // Exibe o conteúdo enviado no monitor serial
            Serial.print("[");
            Serial.print(millis());
            Serial.print(" ms] Enviado: ");
            Serial.println(resposta);
          } else {
            // ID != 2
            Serial.print("[");
            Serial.print(millis());
            Serial.println(" ms] Ignorado: ID não é 2");
          }
        }
    }
  }

  digitalWrite(LED_BUILTIN, LOW); // Apaga o led interno (acabou a janela de escuta/envio da resposta)
}