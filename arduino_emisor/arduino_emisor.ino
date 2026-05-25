#include <SoftwareSerial.h>

// Definimos los pines virtuales: RX (Pin 2), TX (Pin 3)
// El Pin 3 (TX) va conectado al divisor de tensión (1.8k y 3.3k)
SoftwareSerial serialHaciaNode(2, 3); 

int contador = 0;

void setup() {
  Serial.begin(9600);           // Monitor Serie para la PC
  serialHaciaNode.begin(9600);  // Velocidad de comunicación con el NodeMCU
  Serial.println("Arduino Emisor Inicializado...");
}

void loop() {
  // Enviamos el número al NodeMCU seguido de un salto de línea (\n)
  serialHaciaNode.println(contador); 
  
  // Lo mostramos también en la PC para llevar el control
  Serial.print("Arduino envió: ");
  Serial.println(contador);
  
  contador++;   // Incrementamos el número
  delay(1000);  // Esperamos 1 segundo entre envíos
}