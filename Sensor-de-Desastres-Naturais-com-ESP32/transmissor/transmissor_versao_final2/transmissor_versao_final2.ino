const int outputPin = 17; // GPIO 21 na ESP32
const int bitDuration = 200; // 200 milissegundos por bit (0.2s)
byte mensagem = 0b10101010;

#define FREQ 125000   // Frequência da portadora
#define RESOLUTION 1 // 8 bits de resolução do PWM

void setup() {
  Serial.begin(115200);

  // Configura PWM para o pino 17 (canal 0)
  bool success1 = ledcAttach(outputPin, FREQ, RESOLUTION);
  if (!success1) {
    Serial.println("Falha ao configurar o PWM no pino 17");
  }

}

void loop() {
  for (int i = 7; i >= 0; i--) {
    int bit = (mensagem >> i) & 1;

    if (bit == 1) {
      ledcWrite(outputPin, 1); // Liga PWM
    } else {
      ledcWrite(outputPin, 0); // Desliga PWM 
    }

    delay(bitDuration); // tempo de cada bit para facilitar a visualização
  }

  //delay(1000); // espera 1 segundo antes de repetir a sequência
}
