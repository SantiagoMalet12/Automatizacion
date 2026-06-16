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
  
  // Corrección de acción