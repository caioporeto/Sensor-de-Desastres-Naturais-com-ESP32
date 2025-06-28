// ==== CÓDIGO DO RECEPTOR FINAL (NÓ 3) - Com JSON ====

#include <RH_ASK.h>
#include <SPI.h>
#include <ArduinoJson.h> // ===== MUDANÇA: Biblioteca para JSON

const int LED_PIN = 2;
const uint8_t NODE_ID = 3;
const uint8_t LINK_ID = 2; // ID da mensagem vinda do Link

RH_ASK driver(2000, 4, 17);

void setup() {
    Serial.begin(9600);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    if (!driver.init()) {
        Serial.println("Falha ao iniciar o Rádio RF");
        while (true);
    }
    driver.setThisAddress(NODE_ID);
    Serial.println("Receptor RF (JSON) iniciado com sucesso!");
}

void loop() {
    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);

    if (driver.recv(buf, &buflen)) {
        buf[buflen] = '\0';
        Serial.print("<- Mensagem recebida do Link: ");
        Serial.println((char*)buf);
        digitalWrite(LED_PIN, HIGH);

        delay(100);

        // ===== MUDANÇA JSON: Interpreta a mensagem =====
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, (char*)buf);

        if (!error) {
            int id = doc["id"];
            float distancia = doc["data"]; // ArduinoJson pode converter diretamente

            if (id == LINK_ID) {
                Serial.print("Dado de distancia recebido: ");
                Serial.println(distancia);

                // ===== MUDANÇA JSON: Monta a confirmação =====
                char resposta[64];
                JsonDocument docResposta;
                docResposta["id"] = NODE_ID;
                docResposta["data"] = doc["data"]; // Devolve o mesmo dado para confirmação
                serializeJson(docResposta, resposta);
                
                driver.send((uint8_t *)resposta, strlen(resposta));
                driver.waitPacketSent();
                Serial.print("-> Confirmacao enviada para o Link: ");
                Serial.println(resposta);
            }
        } else {
            Serial.print("!!! ERRO: Falha ao interpretar JSON: ");
            Serial.println(error.c_str());
        }
        
        delay(200);
        digitalWrite(LED_PIN, LOW);
    }
}