#include <SoftwareSerial.h>

// Definimos pines virtuales en NodeMCU: RX (Pin D2 / GPIO4), TX (Pin D3 / GPIO0)
// El Pin D2 (RX) es el que recibe el cable corregido del divisor de tensión
SoftwareSerial serialDesdeArduino(4, 0); 

void setup() {
  Serial.begin(9600);              // Monitor Serie para la PC
  serialDesdeArduino.begin(9600);  // Escucha al Arduino
  Serial.println("NodeMCU Receptor listo y escuchando...");
}

void loop() {
  // Verificamos si el Arduino mandó algo
  if (serialDesdeArduino.available() > 0) {
    // Leemos la cadena de texto hasta el salto de línea
    String datoRecibido = serialDesdeArduino.readStringUntil('\n');
    
    // Lo mostramos en el Monitor Serie de la PC
    Serial.print("¡NodeMCU recibió con éxito!: ");
    Serial.println(datoRecibido);
  }
}