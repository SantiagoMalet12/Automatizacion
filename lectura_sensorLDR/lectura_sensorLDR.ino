int sensorLuz = A0;  // Pin analógico conectado al LDR
int pinLED = 5;      // Pin digital PWM conectado al LED (¡Asegurate de usar uno con el símbolo ~!)

void setup() {
  Serial.begin(9600);       // Inicia comunicación serial
  pinMode(pinLED, OUTPUT);  // Configura el pin del LED como salida [cite: 157]
  
  // Arrancamos con el LED totalmente apagado (Brillo = 0)
  analogWrite(pinLED, 0);   
  
  Serial.println("--- INICIANDO ENSAYO EN LAZO ABIERTO ---");
  Serial.println("Tiempo(ms),ValorLDR");
}

void loop() {
  unsigned long tiempoActual = millis(); // Registra el tiempo desde que arrancó el Arduino

  // 1. FASE DE OSCURIDAD: Durante los primeros 3 segundos (3000 ms) el LED se queda apagado
  if (tiempoActual < 3000) {
    analogWrite(pinLED, 0);
  }
  
  // 2. EL ESCALÓN DE PWM: A los 3 segundos exactos, el LED salta de golpe a un brillo de 150 [cite: 97, 111]
  else {
    analogWrite(pinLED, 150); 
  }

  // 3. REGISTRO DE DATOS: Leemos el LDR e imprimimos en formato CSV para poder graficarlo después
  int valor = analogRead(sensorLuz); 
  
  Serial.print(tiempoActual);
  Serial.print(",");
  Serial.println(valor);

  delay(50); // Muestreo más rápido (50ms) para capturar bien la curva de subida [cite: 106]
}