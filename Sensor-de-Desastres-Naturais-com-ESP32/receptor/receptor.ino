#include <RH_ASK.h>
#include <SPI.h>

RH_ASK driver(9600, 16, -1, -1);

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
        Serial.print("Recebido: ");
        Serial.println((char*)buf);
    } else {
        Serial.println("Aguardando mensagem...");
    }
}
