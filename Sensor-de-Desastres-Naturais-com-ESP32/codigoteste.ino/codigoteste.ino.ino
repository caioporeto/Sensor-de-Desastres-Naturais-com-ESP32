#include <RH_ASK.h>
#include <SPI.h>

RH_ASK driver;

void setup() {
    Serial.begin(115200);
    if (!driver.init()) {
        Serial.println("Erro ao iniciar RF");
    }
    Serial.println("Iniciou!");
}

void loop() {
    uint8_t buf[32];
    uint8_t buflen = sizeof(buf);

    if (driver.recv(buf, &buflen)) {
        Serial.print("Recebido: ");
        Serial.println((char*)buf);
    }
}
