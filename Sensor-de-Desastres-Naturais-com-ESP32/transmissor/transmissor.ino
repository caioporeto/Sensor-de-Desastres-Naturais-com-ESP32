#include <RH_ASK.h>
#include <SPI.h>

RH_ASK driver(9600, -1, 17, -1);

void setup() {
    Serial.begin(9600);
    if (!driver.init()) {
        Serial.println("Erro ao iniciar RF");
    } else {
        Serial.println("RF iniciado com sucesso");
    }
}

void loop() {
    const char *msg = "AAA37graus";
    Serial.println("Enviando mensagem...");
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
    Serial.println("Mensagem enviada");
}