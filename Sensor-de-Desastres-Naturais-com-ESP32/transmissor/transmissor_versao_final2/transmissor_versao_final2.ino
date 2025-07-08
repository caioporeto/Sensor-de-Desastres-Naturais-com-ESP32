#define INPUT_PIN 13

void setup() {
  Serial.begin(9600);  // Inicializa a comunicação serial
  pinMode(INPUT_PIN, INPUT); // Configura o pino 13 como entrada
}

void loop() {
  int bitValue = !digitalRead(INPUT_PIN); // Lê o estado do pino
 
    delay(200); // Espera 200ms
    Serial.print(bitValue);
}