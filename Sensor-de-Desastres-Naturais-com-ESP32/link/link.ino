LINK:
// ==== CÓDIGO DO LINK/PONTE (NÓ 2) - Com JSON ====

#include <RH_ASK.h>
#include <SPI.h>
#include <ArduinoJson.h> // ===== MUDANÇA: Biblioteca para JSON

const int LED_PIN = 2;
const uint8_t NODE_ID = 2;
const uint8_t TRANSMITTER_ID = 1;
const uint8_t RECEIVER_ID = 3;
const uint8_t MSG_ID_TO_RECEIVER = 2;
const uint8_t MSG_ID_TO_TRANSMITTER = 4;

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
    Serial.println("Link RF (JSON) iniciado com sucesso!");
}

void loop() {
    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);

    if (driver.recv(buf, &buflen)) {
        buf[buflen] = '\0';
        digitalWrite(LED_PIN, HIGH);
        Serial.print("<- Mensagem recebida: ");
        Serial.println((char*)buf);

        delay(100);

        // ===== MUDANÇA JSON: Interpreta a mensagem =====
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, (char*)buf);

        if (!error) {
            int id = doc["id"];
            JsonVariant data = doc["data"]; // Pode ser float ou string

            char resposta[64];
            JsonDocument docResposta;

            if (id == TRANSMITTER_ID) {
                // Monta a resposta para o Receptor
                docResposta["id"] = MSG_ID_TO_RECEIVER;
                docResposta["data"] = data; // Repassa o mesmo dado
                serializeJson(docResposta, resposta);
                
                driver.send((uint8_t *)resposta, strlen(resposta));
                driver.waitPacketSent();
                Serial.print("-> Retransmitindo para o Receptor: ");
                Serial.println(resposta);

            } else if (id == RECEIVER_ID) {
                // Monta a resposta para o Transmissor
                docResposta["id"] = MSG_ID_TO_TRANSMITTER;
                docResposta["data"] = data; // Repassa o mesmo dado
                serializeJson(docResposta, resposta);

                driver.send((uint8_t *)resposta, strlen(resposta));
                driver.waitPacketSent();
                Serial.print("-> Retransmitindo confirmacao para o Transmissor: ");
                Serial.println(resposta);
            }
        } else {
             Serial.print("!!! ERRO: Falha ao interpretar JSON: ");
             Serial.println(error.c_str());
        }
        
        digitalWrite(LED_PIN, LOW);
    }
}