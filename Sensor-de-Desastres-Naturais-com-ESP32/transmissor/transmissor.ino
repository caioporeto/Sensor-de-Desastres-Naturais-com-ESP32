Transmissor
// ==== CÓDIGO DO TRANSMISSOR (NÓ 1) - Com Logs Específicos [FINAL] ====

#include <RH_ASK.h>
#include <SPI.h>
#include <ArduinoJson.h>

// --- Pinos ---
const int LED_PIN = 2;
const int TRIG_PIN = 19;
const int ECHO_PIN = 18;

// --- Configurações de Comunicação ---
const uint8_t NODE_ID = 1;
const uint8_t LINK_CONFIRMATION_ID = 4;
RH_ASK driver(2000, 4, 17); // RX=4, TX=17

// --- Controle de Estado ---
bool aguardandoConfirmacao = false;
unsigned long ultimoEnvio = 0;
const unsigned long TIMEOUT_CONFIRMACAO = 1500;

// --- Controle de Medição ---
unsigned long ultimaMedicao = 0;
const unsigned long INTERVALO_MEDICAO = 5000; // Medir a cada 5 segundos

// --- Buffer FIFO ---
const int FIFO_SIZE = 10;
float fifoBuffer[FIFO_SIZE];
int fifoHead = 0;
int fifoTail = 0;
int fifoCount = 0;

// --- Funções da Fila (FIFO) --- (sem alterações)
bool fifo_push(float value) {
    if (fifoCount >= FIFO_SIZE) {
        Serial.println("!!! ERRO: Fila (FIFO) cheia! Descartando nova medição.");
        return false;
    }
    fifoBuffer[fifoTail] = value;
    fifoTail = (fifoTail + 1) % FIFO_SIZE;
    fifoCount++;
    return true;
}

bool fifo_pop() {
    if (fifoCount <= 0) return false;
    fifoHead = (fifoHead + 1) % FIFO_SIZE;
    fifoCount--;
    return true;
}

float fifo_peek() {
    if (fifoCount <= 0) return -1.0;
    return fifoBuffer[fifoHead];
}

// --- Função para medir a distância --- (sem alterações, mas com a robustez que adicionamos)
float medirDistancia() {
    long duration;
    float distance = 30;
    int tentativas = 0;
    while (distance >= 20 && tentativas < 5) {
        digitalWrite(TRIG_PIN, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIG_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, LOW);
        duration = pulseIn(ECHO_PIN, HIGH, 50000); // Timeout no pulseIn
        if (duration > 0) {
            distance = duration * 0.034 / 2;
        } else {
            distance = 999; // Valor de erro
        }
        Serial.print("Medindo... Distancia: ");
        Serial.println(distance);
        tentativas++;
        if (distance >= 20) delay(100);
    }

    if (distance < 20) {
        Serial.print("OBJETO DETECTADO A ");
        Serial.print(distance);
        Serial.println(" cm");
    } else {
        Serial.println("Nenhum objeto proximo detectado.");
        return -1.0; 
    }
    return distance;
}

void setup() {
    Serial.begin(9600);
    pinMode(LED_PIN, OUTPUT);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    if (!driver.init()) {
        Serial.println("Falha ao iniciar o Rádio RF");
        while (true);
    }
    driver.setThisAddress(NODE_ID);
    Serial.println("Transmissor RF (FIFO & JSON) [Logs Específicos] iniciado!");
    digitalWrite(LED_PIN, LOW);
}

void loop() {
    // Passo 1: Medir em intervalos de tempo
    if (millis() - ultimaMedicao > INTERVALO_MEDICAO) {
        ultimaMedicao = millis();
        if (fifoCount < FIFO_SIZE) {
            Serial.println("\n--- Iniciando ciclo de medição ---");
            float distancia = medirDistancia();
            if (distancia > 0) {
               if (fifo_push(distancia)) {
                  Serial.println("Medição adicionada à fila.");
               }
            }
        } else {
            Serial.println("Fila cheia, medição pulada.");
        }
    }

    // Passo 2: Enviar item da fila se possível
    if (!aguardandoConfirmacao && fifoCount > 0) {
        float distanciaParaEnviar = fifo_peek();
        JsonDocument doc;
        char msgParaEnviar[64];
        doc["id"] = NODE_ID;
        doc["data"] = distanciaParaEnviar;
        serializeJson(doc, msgParaEnviar);
        digitalWrite(LED_PIN, HIGH);
        driver.send((uint8_t *)msgParaEnviar, strlen(msgParaEnviar));
        driver.waitPacketSent();
        digitalWrite(LED_PIN, LOW);
        Serial.print("-> Enviando da fila: ");
        Serial.println(msgParaEnviar);
        aguardandoConfirmacao = true;
        ultimoEnvio = millis();
    }

    // Passo 3: Processar recebimento de confirmação
    if (aguardandoConfirmacao) {
        uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
        uint8_t buflen = sizeof(buf);

        if (driver.recv(buf, &buflen)) {
            buf[buflen] = '\0';
            Serial.print("<- Mensagem recebida: '");
            Serial.print((char*)buf);
            Serial.println("'");
            
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, (char*)buf);

            if (!error) {
                int idRecebido = doc["id"];
                
                // ===== MUDANÇA: Lógica de verificação mais específica =====
                if (idRecebido == LINK_CONFIRMATION_ID) {
                    // O ID está correto, é uma tentativa de confirmação. Agora vamos checar o dado.
                    float dadoConfirmado = doc["data"];
                    float dadoEsperado = fifo_peek();
                    const float epsilon = 0.01f;

                    if (abs(dadoConfirmado - dadoEsperado) < epsilon) {
                        // SUCESSO! ID e Dado conferem.
                        Serial.println(">>> SUCESSO! Confirmacao valida recebida.");
                        Serial.println("--- Ciclo Completo. Removendo item da fila. ---");
                        fifo_pop();
                        digitalWrite(LED_PIN, HIGH);
                        delay(500);
                        digitalWrite(LED_PIN, LOW);
                        aguardandoConfirmacao = false;
                    } else {
                        // ERRO REAL! O ID de confirmação veio, mas o dado é de outra mensagem.
                        Serial.print("!!! ERRO: ID de confirmacao correto (4), mas o dado nao confere. Esperado: ");
                        Serial.print(dadoEsperado);
                        Serial.print(" | Recebido: ");
                        Serial.println(dadoConfirmado);
                    }
                } else {
                    // INFORMATIVO: A mensagem não era uma confirmação. Apenas ignoramos.
                    Serial.print("--- INFO: Mensagem ignorada. Nao e uma confirmacao (ID recebido: ");
                    Serial.print(idRecebido);
                    Serial.print(", esperado: ");
                    Serial.print(LINK_CONFIRMATION_ID);
                    Serial.println(").");
                }

            } else {
                Serial.print("!!! ERRO: Falha ao interpretar JSON de confirmacao: ");
                Serial.println(error.c_str());
            }
        }

        // Passo 4: Verificar Timeout (sem alterações)
        if (aguardandoConfirmacao && (millis() - ultimoEnvio > TIMEOUT_CONFIRMACAO)) {
            Serial.println("!! TIMEOUT! Reenviando o mesmo item...");
            JsonDocument doc;
            char msgParaEnviar[64];
            doc["id"] = NODE_ID;
            doc["data"] = fifo_peek(); 
            serializeJson(doc, msgParaEnviar);
            digitalWrite(LED_PIN, HIGH);
            driver.send((uint8_t *)msgParaEnviar, strlen(msgParaEnviar));
            driver.waitPacketSent();
            digitalWrite(LED_PIN, LOW);
            ultimoEnvio = millis();
        }
    }
}