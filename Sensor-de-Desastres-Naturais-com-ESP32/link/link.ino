#include <RH_ASK.h>
#include <SPI.h>

// Configuração do driver RF:
// - Baud rate: 9600
// - Pino RX: 16
// - Pino TX: 17
// - Sem pino PTT (usado -1)
RH_ASK driver(9600, 12, 13, -1);

void setup() {
  Serial.begin(9600);
  if (!driver.init()) {
    Serial.println("Erro ao iniciar RF");
  } else {
    Serial.println("RF iniciado com sucesso");
  }
}

void loop() {
  uint8_t buf[32];
  uint8_t buflen = sizeof(buf);

  // Verifica se há uma mensagem recebida
  if (driver.recv(buf, &buflen)) {
    String mensagem =  String((char*)buf);
    Serial.print("Recebido: ");
    Serial.println((char*)buf);
    const char *msg = buf.substring(3, 17);
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
    Serial.print("Mensagem Enviada: ");
    //Serial.println("Mensagem transmitida");
  }
}