#include <RH_ASK.h>
#include <SPI.h>

#define LED_BUILTIN 2  // Definindo o pino do LED (geralmente é o pino 2)

RH_ASK driver(9600, 4, 17, -1);  // Configuração do driver RF com RX no pino 16 e TX no pino 17

void setup() {
  Serial.begin(9600);  // Inicializa a comunicação serial a 9600 bauds

  // Inicializa o driver de comunicação RF
  if (!driver.init()) {
    Serial.println("Erro ao iniciar RF");  // Se a inicialização falhar
  } else {
    Serial.println("RF iniciado com sucesso");  // Se a inicialização for bem-sucedida
  }

  pinMode(LED_BUILTIN, OUTPUT);  // Configura o LED como saída
}

void loop() {
  uint8_t buf[32];  // Buffer para armazenar dados recebidos
  uint8_t buflen = sizeof(buf);  // Tamanho do buffer

  // Verifica se há dados recebidos
  if (driver.recv(buf, &buflen)) {
    // Converte o buffer em uma string
    String receivedMessage = String((char*)buf);

    // Imprime a mensagem recebida no monitor serial
    Serial.print("Recebido: ");
    Serial.println(receivedMessage);

    // Se a mensagem recebida for "A", envia "C"
    if (receivedMessage == "A") {
      Serial.println("Enviando mensagem X...");
      const char *msg = "X";  // Mensagem a ser enviada
      driver.send((uint8_t *)msg, strlen(msg));  // Envia a mensagem via RF
      driver.waitPacketSent();  // Espera até que o pacote seja completamente enviado
      digitalWrite(LED_BUILTIN, HIGH);  // Acende o LED para indicar que a mensagem foi enviada
    } else {
      digitalWrite(LED_BUILTIN, LOW);  // Se a mensagem não for "A", o LED será apagado
    }
  } else {
    // Se não houver mensagem recebida, apaga o LED
    digitalWrite(LED_BUILTIN, LOW);
  }

  // delay(100);  // Aguarda um curto intervalo antes de verificar novamente
}
