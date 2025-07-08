// =================================================================

// ==== CÓDIGO RECEPTOR (NÓ 3) - COM ENDEREÇAMENTO PARA O NÓ 2  ====

// =================================================================

#include <RH_ASK.h>

#include <SPI.h>

#include <ArduinoJson.h>



// --- Configurações ---

const int LED_PIN = 2;



// --- Endereços da rede ---

const uint8_t ENDERECO_ESTE_NO = 3;

const uint8_t ENDERECO_NO_LINK = 2;

const uint8_t ENDERECO_NO_TRANSMISSOR = 1; // ID original do transmissor (para checar no JSON)



// --- Componentes ---

RH_ASK driver(2000, 4, 17);



void setup() {

    Serial.begin(9600);

    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);

    if (!driver.init()) {

        Serial.println("Falha ao iniciar o Radio RF");

        while (true);

    }

    driver.setThisAddress(ENDERECO_ESTE_NO);

    Serial.println("Receptor RF (No 3) com enderecamento iniciado!");

}



void loop() {

    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];

    uint8_t buflen = sizeof(buf);



    // Só processa mensagens vindas do Link (2)

    if (driver.recv(buf, &buflen) && driver.headerFrom() == ENDERECO_NO_LINK) {

        buf[buflen] = '\0';

        digitalWrite(LED_PIN, HIGH);

        Serial.print("\n<- Mensagem recebida do Link (2): ");

        Serial.println((char*)buf);



        JsonDocument doc;

        if (deserializeJson(doc, (char*)buf) == DeserializationError::Ok) {

            // Checa se o ID original (dentro do JSON) é do Transmissor (1)

            if (doc["id"] == ENDERECO_NO_TRANSMISSOR) {

                float distancia = doc["data"];

                const char* timestamp = doc["timestamp"];



                Serial.println("--------------------------------");

                Serial.println("DADOS VALIDOS DO SENSOR:");

                Serial.print("  > Horario: "); Serial.println(timestamp);

                Serial.print("  > Distancia: "); Serial.print(distancia); Serial.println(" cm");

                Serial.println("--------------------------------");



                // --- LÓGICA DE ENVIO DE CONFIRMAÇÃO ---

                JsonDocument confirmDoc;

                char resposta[64];

                confirmDoc["id"] = ENDERECO_ESTE_NO; // ID deste nó na resposta

                confirmDoc["confirm_data"] = distancia; 

                serializeJson(confirmDoc, resposta);

                

                // ** CORREÇÃO **

                // Define explicitamente o remetente e o destinatário antes de enviar a confirmação.

                driver.setHeaderFrom(ENDERECO_ESTE_NO);

                driver.setHeaderTo(ENDERECO_NO_LINK);



                driver.send((uint8_t *)resposta, strlen(resposta));

                driver.waitPacketSent();

                Serial.print("-> Confirmacao enviada para o Link (2): ");

                Serial.println(resposta);

            }

        } else {

             Serial.println("!!! ERRO: Falha ao interpretar JSON recebido.");

        }

        delay(200);

        digitalWrite(LED_PIN, LOW);

    }

}