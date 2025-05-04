#include <RH_ASK.h>
#include <SPI.h>

#define LED_BUILTIN 2 // Led interno da ESP32
#define trigPin 19 // Pino 19 como trigger do sensor ultrassônico
#define echoPin 18 // Pino 18 como echo do sensor ultrassônico

int confirmacao = 1; // Flag para controlar quando medir a distância

// Configura o driver para a comunicação RF
RH_ASK driver(2000, 4, 17, -1);

// Função para calcular a distância (em cm) entre o objeto e o sensor ultrassônico 
float distancia(){
  long duration;
  float distance = 20;

  while(distance >= 20){ // Enquanto a distância não for menor que 20 cm
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2); // Aguarda 2 us
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10); // Aguarda 10 us

    digitalWrite(trigPin, LOW);

    // Lê o tempo do pulso recebido no pino ECHO
    duration = pulseIn(echoPin, HIGH);

    // Calcula a distância (em cm)
    // 0.034 -> velocidade do som (cm/us)
    distance = duration * 0.034 / 2;
    Serial.println(distance);
  }
  return distance;
}


void setup() {  
  //Inicializa a serial a 9600 bps
  Serial.begin(9600);

  // Inicializa o driver RF e avisa se falhar
  if (!driver.init()) {
    Serial.println("Erro ao iniciar RF");
  } else {
    Serial.println("RF iniciado com sucesso");
  }

  // driver.setThisAddress(1);  // Define o ID deste dispositivo como 1

  // Configura o led interno da ESP32
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // LED começa desligado

  // Configura pinos do sensor ultrassônico
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

float dado;

void loop() {
  uint8_t buf[32]; 
  uint8_t buflen = sizeof(buf);

  char dadoString[10];

  int lastSendTime = millis();
  int sendInterval = 400; // Janela de recepção: 400 ms

  if (confirmacao == 1){
    dado = distancia(); // Dado recebe a distância do sensor ao objeto
    dtostrf(dado, 0, 4, dadoString); // Transforma float (dado) em char[] (dadoString)
    confirmacao = 0; // Reseta flag para não medir novamente até receber a confirmação
  }

  // Escuta por um intervalo de tempo definido
  while (millis() - lastSendTime <= sendInterval) {
    if (driver.recv(buf, &buflen)) {
      buf[buflen] = '\0';  // Garante que o buffer seja uma string válida
      // Serial.print("[");
      // Serial.print(millis());
      // Serial.print(" ms] Recebido: ");
      // Serial.println((char*)buf);
      
      char bufferCopia[32];
      strcpy(bufferCopia, (char*)buf);

      char *id = strtok(bufferCopia, ":");      // Pega a parte antes dos dois-pontos
      char *dado_recebido = strtok(nullptr, ""); // Pega o restante da string

      digitalWrite(LED_BUILTIN, HIGH);
      // Se a mensagem recebida for "V", imprime "ACABOU" e interrompe o loop

      if (id != nullptr && dado_recebido != nullptr) {
        if (strcmp(id, "4") == 0 && strcmp(dado_recebido, dadoString) == 0) {
          Serial.print("[");
          Serial.print(millis());
          Serial.println(" ms] ACABOU");
          confirmacao = 1; // Para a função distância ser chamada novamente
        }
      }
    }
  }
  
  digitalWrite(LED_BUILTIN, LOW); // Apaga o led
  
  char id[50] = "1";
  char dadoStr[10];
  dtostrf(dado, 0, 4, dadoStr);
  strcat(id, ":");         // Adiciona separador
  strcat(id, dadoStr); 
  const char *msg = id;

  driver.send((uint8_t *)msg, strlen(msg));
  driver.waitPacketSent();  // Aguarda o envio completo do pacote
}