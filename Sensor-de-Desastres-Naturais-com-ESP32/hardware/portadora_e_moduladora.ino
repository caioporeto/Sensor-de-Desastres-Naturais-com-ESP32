#define OUTPUT_PIN1 17 
#define OUTPUT_PIN2 16
#define FREQ 125000
#define RESOLUTION 8

/* Portadora -> gera uma onda quadrada de 125kHz 
que vai ser transformada em uma onda senoidal de 125kHz 
na saída do filtro */
void setup() {
  // Inicializa a serial a 115200 bps
  Serial.begin(115200);
  bool sucess = ledcAttach(OUTPUT_PIN1, FREQ, RESOLUTION); // Usa PWM para gerar uma onda quadrada de exatamente 125kHz
  if (!sucess) {
    Serial.println(" Falha ao configurar o PWM ");
  }
  ledcWrite(OUTPUT_PIN1, 128); // Duty cicle de 50% (128 é metade de 256 [2^8])
  Serial.println(" Onda quadrada de 125 kHz gerada! ");
  pinMode(16, OUTPUT);
}

/* Moduladora -> onda quadrada que vai modular o sinal 
da portadora no CD4016 */
void loop() {
  digitalWrite(OUTPUT_PIN2, HIGH);
  delay(5000);
  digitalWrite(OUTPUT_PIN2, LOW);
  delay(5000);
}

