#include <RH_ASK.h>
#include <SPI.h>

#define LED_BUILTIN 2 // Led interno da ESP32

// Configura o driver para a comunicação RF
RH_ASK driver(2000, 4, 17, -1);

void setup() {
  // Inicializa a serial a 9600 bps
  Serial.begin(9600);
  if (!driver.init()) {
    Serial.println("Erro ao iniciar RF");
  } else {
    Serial.println("RF iniciado com sucesso");
  }

  driver.setThisAddress(2);  // Define o ID deste dispositivo como 2

  // Configura o led interno da ESP32
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // LED começa desligado
}

void loop() {
  uint8_t buf[32]; 
  uint8_t buflen = sizeof(buf);
  int lastSendTime = millis(); // Marca o início do período de escuta
  int sendInterval = 50; // Janela de recepção: 50 ms

  // Escuta por um intervalo de tempo definido
  while (millis() - lastSendTime <= sendInterval) {
    if (driver.recv(buf, &buflen)) {
      buf[buflen] = '\0';  // Garante que o buffer seja uma string válida
      // Serial.print("[");
      // Serial.print(millis());
      // Serial.print(" ms] Recebido: ");
      Serial.println((char*)buf); // Imprime o conteúdo recebido
      
      // Copia o buffer
      char bufferCopia[32];
      strcpy(bufferCopia, (char*)buf);

      char *id = strtok(bufferCopia, ":");      // Pega a parte antes dos dois-pontos
      char *dado = strtok(nullptr, "");         // Pega o restante da string
      
      digitalWrite(LED_BUILTIN, HIGH); // Acende o Led enquanto processa a mensagem
    
      if (id != nullptr && dado != nullptr) {
      // Verifica o conteúdo recebido e envia a resposta apropriada
        if (strcmp(id, "1") == 0) {
          char resposta[50] = "2:";
          strcat(resposta, dado);
          // Descomente a linha abaixo se desejar logar o envio com timestamp:
          // Serial.print("["); Serial.print(millis()); Serial.println(" ms] Enviando resposta: A");
          driver.send((uint8_t *)resposta, strlen(resposta));
          driver.waitPacketSent();
          // delay(25);
        }
        else if (strcmp(id, "3") == 0) {
          char resposta[50] = "4:";
          strcat(resposta, dado);
          // Descomente a linha abaixo se desejar logar o envio com timestamp:
          //  Serial.print("["); Serial.print(millis()); Serial.println(" ms] Enviando resposta: V");
          driver.send((uint8_t *)resposta, strlen(resposta));
          driver.waitPacketSent();
        }
      }
    }
  }
  
  // Delay opcional para estabilizar a comunicação, se necessário
  // delay(100);
  
  digitalWrite(LED_BUILTIN, LOW); // Apaga o led interno ao final do ciclo
}