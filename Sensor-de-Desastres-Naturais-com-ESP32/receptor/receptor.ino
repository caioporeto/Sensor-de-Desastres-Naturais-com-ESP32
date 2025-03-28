#include <RH_ASK.h>
#include <SPI.h>

RH_ASK driver(9600, 12, -1, -1);

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

    if (driver.recv(buf, &buflen)) {
      String mensagem = String((char*)buf);
      Serial.print("Recebido: ");
      //mensagem = mensagem.substring(3,11);
      Serial.println(mensagem);
    } else {
        //Serial.println("Aguardando mensagem...");
    }
}
