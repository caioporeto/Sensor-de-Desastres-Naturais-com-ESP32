// =================================================================

// ==== CÓDIGO LINK/PONTE (NÓ 2) - COM ROTEAMENTO POR ENDEREÇO  ====

// =================================================================

#include <RH_ASK.h>

#include <SPI.h>

#include <ArduinoJson.h>



// --- Configurações ---

const int LED_PIN = 2;



// --- Endereços da rede ---

const uint8_t ENDERECO_ESTE_NO = 2;

const uint8_t ENDERECO_NO_TRANSMISSOR = 1;

const uint8_t ENDERECO_NO_RECEPTOR = 3;



// --- Componentes ---

RH_ASK driver(2000, 4, 17);



// --- Buffers FIFO ---

const int FIFO_SIZE = 5;

const int MAX_MSG_LEN = RH_ASK_MAX_MESSAGE_LEN;



// Fila de ALTA PRIORIDADE: mensagens do Nó 3 para o Nó 1

char fifoPrioritario[FIFO_SIZE][MAX_MSG_LEN];

uint8_t fifoPrioritarioLen[FIFO_SIZE];

int fifoPrioHead = 0, fifoPrioTail = 0, fifoPrioCount = 0;



// Fila de PRIORIDADE NORMAL: mensagens do Nó 1 para o Nó 3

char fifoNormal[FIFO_SIZE][MAX_MSG_LEN];

uint8_t fifoNormalLen[FIFO_SIZE];

int fifoNormalHead = 0, fifoNormalTail = 0, fifoNormalCount = 0;



// --- Funções das Filas (FIFO) - Sem alterações ---

bool fifoPrio_is_full() { return fifoPrioCount >= FIFO_SIZE; }

bool fifoPrio_is_empty() { return fifoPrioCount == 0; }

bool fifoPrio_push(const uint8_t* msg, uint8_t len) {

    if (fifoPrio_is_full()) return false;

    memcpy(fifoPrioritario[fifoPrioTail], msg, len);

    fifoPrioritarioLen[fifoPrioTail] = len;

    fifoPrioTail = (fifoPrioTail + 1) % FIFO_SIZE;

    fifoPrioCount++;

    return true;

}

void fifoPrio_pop() {

    if (fifoPrio_is_empty()) return;

    fifoPrioHead = (fifoPrioHead + 1) % FIFO_SIZE;

    fifoPrioCount--;

}

uint8_t fifoPrio_peek(uint8_t* buffer) {

    if (fifoPrio_is_empty()) return 0;

    uint8_t len = fifoPrioritarioLen[fifoPrioHead];

    memcpy(buffer, fifoPrioritario[fifoPrioHead], len);

    return len;

}



bool fifoNormal_is_full() { return fifoNormalCount >= FIFO_SIZE; }

bool fifoNormal_is_empty() { return fifoNormalCount == 0; }

bool fifoNormal_push(const uint8_t* msg, uint8_t len) {

    if (fifoNormal_is_full()) return false;

    memcpy(fifoNormal[fifoNormalTail], msg, len);

    fifoNormalLen[fifoNormalTail] = len;

    fifoNormalTail = (fifoNormalTail + 1) % FIFO_SIZE;

    fifoNormalCount++;

    return true;

}

void fifoNormal_pop() {

    if (fifoNormal_is_empty()) return;

    fifoNormalHead = (fifoNormalHead + 1) % FIFO_SIZE;

    fifoNormalCount--;

}

uint8_t fifoNormal_peek(uint8_t* buffer) {

    if (fifoNormal_is_empty()) return 0;

    uint8_t len = fifoNormalLen[fifoNormalHead];

    memcpy(buffer, fifoNormal[fifoNormalHead], len);

    return len;

}



void setup() {

    Serial.begin(9600);

    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);

    if (!driver.init()) {

        Serial.println("!! Falha ao iniciar o Radio RF !!");

        while (true);

    }

    driver.setThisAddress(ENDERECO_ESTE_NO);

    Serial.println("==============================================");

    Serial.println("Link RF com ROTEAMENTO (No 2) iniciado!");

}



void loop() {

    // ETAPA 1: Ouvir o rádio

    uint8_t buf[MAX_MSG_LEN];

    uint8_t buflen = sizeof(buf);



    while (driver.recv(buf, &buflen)) {

        buf[buflen] = '\0';

        

        uint8_t idOrigem = driver.headerFrom();



        if (idOrigem == ENDERECO_NO_RECEPTOR) { // Veio do Receptor (3)? É confirmação! PRIORIDADE!

            if (fifoPrio_push(buf, buflen)) {

                Serial.print("<- [PRIORIDADE] Recebido do RECEPTOR (3). Na fila: ");

                Serial.println(fifoPrioCount);

            } else {

                Serial.println("!! ERRO: Fila de prioridade CHEIA! Pacote descartado.");

            }

        } else if (idOrigem == ENDERECO_NO_TRANSMISSOR) { // Veio do Transmissor (1)? É dado! Fila normal.

            if (fifoNormal_push(buf, buflen)) {

                Serial.print("<- [NORMAL] Recebido do TRANSMISSOR (1). Na fila: ");

                Serial.println(fifoNormalCount);

            } else {

                Serial.println("!! ERRO: Fila normal CHEIA! Pacote descartado.");

            }

        } else {

            Serial.print("<- Mensagem recebida de um nó desconhecido (");

            Serial.print(idOrigem);

            Serial.println(") e descartada.");

        }

        

        buflen = sizeof(buf); 

    }



    // ETAPA 2: Retransmitir com base na prioridade

    uint8_t msgParaEnviar[MAX_MSG_LEN];

    uint8_t lenParaEnviar;



    // Fila de prioridade (confirmações para o Nó 1)

    if (!fifoPrio_is_empty()) {

        digitalWrite(LED_PIN, HIGH);

        lenParaEnviar = fifoPrio_peek(msgParaEnviar);

        

        // ** CORREÇÃO **

        // Define explicitamente que o Link (2) é o remetente e o Nó 1 é o destinatário.

        driver.setHeaderFrom(ENDERECO_ESTE_NO);

        driver.setHeaderTo(ENDERECO_NO_TRANSMISSOR);

        

        driver.send(msgParaEnviar, lenParaEnviar);

        driver.waitPacketSent();

        fifoPrio_pop();



        msgParaEnviar[lenParaEnviar] = '\0';

        Serial.print("-> [PRIORIDADE] Retransmitindo para o Transmissor (1): ");

        Serial.println((char*)msgParaEnviar);



        digitalWrite(LED_PIN, LOW);

    } 

    // Fila normal (dados para o Nó 3)

    else if (!fifoNormal_is_empty()) {

        digitalWrite(LED_PIN, HIGH);

        lenParaEnviar = fifoNormal_peek(msgParaEnviar);



        // ** CORREÇÃO **

        // Define explicitamente que o Link (2) é o remetente e o Nó 3 é o destinatário.

        driver.setHeaderFrom(ENDERECO_ESTE_NO);

        driver.setHeaderTo(ENDERECO_NO_RECEPTOR);



        driver.send(msgParaEnviar, lenParaEnviar);

        driver.waitPacketSent();

        fifoNormal_pop();



        msgParaEnviar[lenParaEnviar] = '\0';

        Serial.print("-> [NORMAL] Retransmitindo para o Receptor (3): ");

        Serial.println((char*)msgParaEnviar);

        

        digitalWrite(LED_PIN, LOW);

    }

}