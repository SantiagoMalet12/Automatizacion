#include <SoftwareSerial.h>

SoftwareSerial nodoSerial(10, 11); // Rx, Tx (El pin 11 va al divisor de tensión hacia el NodeMCU)

const int LDR = A0;
const int LED = 5;

// Variables de control PID (Valores iniciales conservadores para evitar oscilaciones)
float kp = 0.15;        
float ki = 0.01;        
float kd = 0.00;         
float referencia = 500; // Valor inicial del slider
bool modoAuto = true;   // Arranca en automático por defecto

float error = 0;
float U = 0;

float errorIntegral = 0;
float errorDerivativo = 0;
float errorAnterior = 0;
const float Ts = 0.1;   // Período de muestreo fijo = 100ms (0.1 segundos)

void setup() {
  Serial.begin(9600);       // Monitor serie de la PC
  nodoSerial.begin(9600);   // Comunicación con NodeMCU
  pinMode(LED, OUTPUT);
}

void loop() {
  // 1. Escuchar comandos RPC desde el NodeMCU (Dashboard Web de ThingsBoard)
  if (nodoSerial.available() > 0) {
    String comando = nodoSerial.readStringUntil('\n');
    comando.trim();
    
    if (comando.startsWith("SET_R:")) {
      referencia = comando.substring(6).toFloat();
      Serial.print("Setpoint actualizado: "); Serial.println(referencia);
    }
    else if (comando.startsWith("SET_KP:")) {
      kp = comando.substring(7).toFloat();
      Serial.print("Kp actualizado: "); Serial.println(kp, 2);
    }
    else if (comando.startsWith("SET_KI:")) {
      ki = comando.substring(7).toFloat();
      Serial.print("Ki actualizado: "); Serial.println(ki, 2);
    }
    else if (comando.startsWith("SET_KD:")) {
      kd = comando.substring(7).toFloat();
      Serial.print("Kd actualizado: "); Serial.println(kd, 2);
    }
    else if (comando.startsWith("SET_MODE:")) {
      String modoStr = comando.substring(9);
      if (modoStr == "AUTO") {
        modoAuto = true;
      } else if (modoStr == "MANUAL") {
        modoAuto = false;
      }
      Serial.print("Modo de control: "); Serial.println(modoAuto ? "AUTOMÁTICO" : "MANUAL");
    }
  }

  // 2. Lazo de Control Digital PID
  int valorLuz = analogRead(LDR);
  
  // Corrección de acción inversa: Si falta luz (referencia > valorLuz), el error es positivo
  // y la acción U debe aumentar para encender más el LED.
  error = referencia - valorLuz; 

  int pwmOut = 0;

  if (modoAuto) {
    // Cálculo del término derivativo discreto
    errorDerivativo = (error - errorAnterior) / Ts;
    
    // Ecuación temporal del controlador PID
    U = (kp * error) + (ki * errorIntegral) + (kd * errorDerivativo);
    pwmOut = (int)U;

    // --- ANTI-WINDUP INDUSTRIAL (Saturación condicional de la integral) ---
    // Solo permitimos que el acumulador sume error si la salida no está saturada (0 a 255)
    if (pwmOut >= 0 && pwmOut <= 255) {
      errorIntegral = errorIntegral + (error * Ts);
    }
  } else {
    // Modo MANUAL: Mapea el slider directo al rango del LED (0-255)
    pwmOut = map((int)referencia, 0, 1023, 0, 255);
    errorIntegral = 0; 
  }

  // Acotación estricta del actuador (Hardware PWM)
  if (pwmOut > 255) pwmOut = 255;
  if (pwmOut < 0)   pwmOut = 0;

  analogWrite(LED, pwmOut);

  // 3. Envío local al Serial Plotter de la PC para diagnóstico rápido
  Serial.print(referencia); Serial.print(",");
  Serial.print(valorLuz);   Serial.print(",");
  Serial.println(pwmOut);

  // 4. Envío de Telemetría JSON Estructurada hacia el NodeMCU
  nodoSerial.print("{");
  nodoSerial.print("\"y\":" + String(valorLuz) + ",");
  nodoSerial.print("\"r\":" + String(referencia) + ",");
  nodoSerial.print("\"e\":" + String(error) + ",");
  nodoSerial.print("\"u\":" + String(pwmOut) + ",");
  nodoSerial.print("\"kp\":" + String(kp, 2) + ",");
  nodoSerial.print("\"ki\":" + String(ki, 2) + ",");
  nodoSerial.print("\"kd\":" + String(kd, 2) + ",");
  nodoSerial.print("\"mode\":\"" + String(modoAuto ? "AUTO" : "MANUAL") + "\"");
  nodoSerial.println("}"); 

  errorAnterior = error;
  delay(100); // Ts = 100ms
}