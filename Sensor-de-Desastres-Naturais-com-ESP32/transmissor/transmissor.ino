
// =======================================================================

// ==== CÓDIGO TRANSMISSOR (NÓ 1) - VERSÃO FINAL CORRIGIDA            ====

// =======================================================================

#include <RH_ASK.h>

#include <SPI.h>

#include <ArduinoJson.h>

#include <Wire.h>

#include <RTClib.h>



// --- Configurações ---

const unsigned long INTERVALO_MEDICAO = 10; // Em segundos

// ** ALTERAÇÃO 1: Aumentar o timeout para dar mais margem **

const unsigned long TIMEOUT_CONFIRMACAO = 15;  // Em segundos

const unsigned long TEMPO_INATIVIDADE_PARA_DORMIR = 40; // Em segundos

const unsigned long DURACAO_SONO = 20; // Em segundos



const int LED_PIN = 2;

const int TRIG_PIN = 19;

const int ECHO_PIN = 18;



// --- Endereços da rede ---

const uint8_t ENDERECO_ESTE_NO = 1;

const uint8_t ENDERECO_NO_LINK = 2;



// --- Máquina de Estados ---

enum Estado { OCIOSO, ENVIANDO, AGUARDANDO_CONFIRMACAO };



// --- Variáveis de estado na memória RTC ---

RTC_DATA_ATTR Estado estadoAtual = OCIOSO;

RTC_DATA_ATTR unsigned long ultimoEnvio = 0;

RTC_DATA_ATTR unsigned long ultimaMedicao = 0;

RTC_DATA_ATTR unsigned long ultimoTempoAtivo = 0;



// --- Buffer FIFO na memória RTC ---

const int FIFO_SIZE = 20;

RTC_DATA_ATTR float fifoBuffer[FIFO_SIZE];

RTC_DATA_ATTR char fifoTimestampBuffer[FIFO_SIZE][20];

RTC_DATA_ATTR int fifoHead = 0, fifoTail = 0, fifoCount = 0;



// --- Componentes ---

RH_ASK driver(2000, 4, 17); // Pinos: RX = 4, TX = 17

RTC_DS3231 rtc;



// --- Funções da Fila (FIFO) - Sem alterações ---

bool fifo_is_full() { return fifoCount >= FIFO_SIZE; }

bool fifo_is_empty() { return fifoCount == 0; }

bool fifo_push(float value, const char* timestamp) {

    if (fifo_is_full()) return false;

    fifoBuffer[fifoTail] = value;

    strcpy(fifoTimestampBuffer[fifoTail], timestamp);

    fifoTail = (fifoTail + 1) % FIFO_SIZE;

    fifoCount++;

    return true;

}

void fifo_pop() {

    if (fifo_is_empty()) return;

    fifoHead = (fifoHead + 1) % FIFO_SIZE;

    fifoCount--;

}

float fifo_peek_data() {

    if (fifo_is_empty()) return -1.0;

    return fifoBuffer[fifoHead];

}

const char* fifo_peek_timestamp() {

    if (fifo_is_empty()) return "1970-01-01 00:00:00";

    return fifoTimestampBuffer[fifoHead];

}



// --- Função de Medição (sem alterações de lógica) ---

float medirDistancia() {

    long duration;

    float distance = -1.0;

    for (int i = 0; i < 3; i++) {

        digitalWrite(TRIG_PIN, LOW);

        delayMicroseconds(2);

        digitalWrite(TRIG_PIN, HIGH);

        delayMicroseconds(10);

        digitalWrite(TRIG_PIN, LOW);

        duration = pulseIn(ECHO_PIN, HIGH, 50000);

        if (duration > 0) {

            distance = duration * 0.034 / 2;

            if (distance < 400) break;

        }

        delay(50);

    }

    return (distance > 0 && distance < 400) ? distance : -1.0;

}



void setup() {

    Serial.begin(9600);

    delay(1000);

    Serial.println("==============================================");

    Serial.println("Iniciando Transmissor (No 1) - vFinal");



    pinMode(LED_PIN, OUTPUT);

    pinMode(TRIG_PIN, OUTPUT);

    pinMode(ECHO_PIN, INPUT);



    if (!driver.init()) Serial.println("!! Falha ao iniciar Radio RF !!");

    driver.setThisAddress(ENDERECO_ESTE_NO);



    if (!rtc.begin()) Serial.println("!! Nao foi possivel encontrar o RTC !!");



    if (rtc.lostPower()) {

        Serial.println("RTC sem energia, ajustando horario e RESETANDO ESTADO.");

        rtc.adjust(DateTime(F(DATE), F(TIME)) - TimeSpan(0, 3, 0, 0));

        estadoAtual = OCIOSO;

        fifoHead = fifoTail = fifoCount = 0;

        ultimaMedicao = 0;

        ultimoEnvio = 0;

    } else {

        Serial.println(">>> Acordando do sono ou reinicio normal. Estado preservado. <<<");

    }



    if (estadoAtual == ENVIANDO || estadoAtual == AGUARDANDO_CONFIRMACAO) {

        digitalWrite(LED_PIN, HIGH);

    } else {

        digitalWrite(LED_PIN, LOW);

    }



    if (ultimoTempoAtivo == 0) {

        ultimoTempoAtivo = rtc.now().unixtime();

    }

    

    Serial.println("Setup completo. Iniciando operacao.");

}





// ** ALTERAÇÃO 2: LÓGICA DO LOOP REESTRUTURADA **

void loop() {

    DateTime agora = rtc.now();

    unsigned long agoraTimestamp = agora.unixtime();



    switch (estadoAtual) {

        case OCIOSO:

            // No estado OCIOSO, podemos fazer medições e checar se devemos dormir.

            if (agoraTimestamp - ultimaMedicao >= INTERVALO_MEDICAO) {

                ultimaMedicao = agoraTimestamp;

                if (!fifo_is_full()) {

                    float distancia = medirDistancia();

                    if (distancia > 0 && distancia < 20) {

                        char timestamp[20];

                        sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d", agora.year(), agora.month(), agora.day(), agora.hour(), agora.minute(), agora.second());

                        if (fifo_push(distancia, timestamp)) {

                            Serial.print(">>> OBJETO DETECTADO! Distancia: ");

                            Serial.print(distancia);

                            Serial.println(" cm.");

                            ultimoTempoAtivo = agoraTimestamp;

                        }

                    }

                }

            }

            

            if (!fifo_is_empty()) {

                estadoAtual = ENVIANDO;

                break;

            }



            if (agoraTimestamp - ultimoTempoAtivo > TEMPO_INATIVIDADE_PARA_DORMIR) {

                Serial.println("Inatividade detectada. Indo dormir...");

                Serial.flush();

                ESP.deepSleep(DURACAO_SONO * 1000000);

            }

            break;



        case ENVIANDO:

            {

                ultimoTempoAtivo = agoraTimestamp;

                digitalWrite(LED_PIN, HIGH);

                

                JsonDocument doc;

                char msgParaEnviar[128];

                doc["id"] = ENDERECO_ESTE_NO;

                doc["data"] = fifo_peek_data();

                doc["timestamp"] = fifo_peek_timestamp();

                serializeJson(doc, msgParaEnviar);



                driver.setHeaderFrom(ENDERECO_ESTE_NO);

                driver.setHeaderTo(ENDERECO_NO_LINK);

                

                driver.send((uint8_t *)msgParaEnviar, strlen(msgParaEnviar));

                driver.waitPacketSent();

                Serial.print("-> Enviando para o Link (2): ");

                Serial.println(msgParaEnviar);

                

                ultimoEnvio = agoraTimestamp;

                estadoAtual = AGUARDANDO_CONFIRMACAO;

            }

            break;



        case AGUARDANDO_CONFIRMACAO:

            // Neste estado, o foco é 100% em escutar o rádio ou checar o timeout.

            // NENHUMA outra tarefa (como medirDistancia) é executada aqui.

            {

                uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];

                uint8_t buflen = sizeof(buf);



                if (driver.recv(buf, &buflen)) {

                  Serial.println("Message found");

                    if (driver.headerFrom() == ENDERECO_NO_LINK) {

                        buf[buflen] = '\0';

                        Serial.print("\n<- Confirmacao recebida do Link (2): ");

                        Serial.println((char*)buf);



                        JsonDocument confirmDoc;

                        if (deserializeJson(confirmDoc, buf) == DeserializationError::Ok) {

                            if (abs(confirmDoc["confirm_data"].as<float>() - fifo_peek_data()) < 0.01) {

                                Serial.println(">>> SUCESSO! Confirmacao valida. Ciclo completo!");

                                fifo_pop();

                                digitalWrite(LED_PIN, LOW);

                                estadoAtual = OCIOSO;

                                ultimoTempoAtivo = agoraTimestamp;

                            } else {

                                Serial.println("!!! AVISO: Confirmacao com dados incorretos.");

                            }

                        } else {

                            Serial.println("!!! ERRO: JSON da confirmacao invalido.");

                        }

                    }

                }



                if (agoraTimestamp - ultimoEnvio > TIMEOUT_CONFIRMACAO) {

                    if (estadoAtual == AGUARDANDO_CONFIRMACAO) {

                         Serial.println("\n!! TIMEOUT! Nenhuma confirmacao recebida. Reenviando...");

                         estadoAtual = ENVIANDO;

                         ultimoTempoAtivo = agoraTimestamp;

                    }

                }

            }

            break;

    }

}