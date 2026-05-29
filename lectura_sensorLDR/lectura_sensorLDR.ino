// Fotoresistencia en A0
const int LDR = A0;
unsigned long  tiempo1, tiempo2;

float kp = 0.05;
float ki = 0.10;
float kd = 0;
float referencia = 500;
float error = 0;
float U;

float errorIntegral = 0;
float errorDerivativo = 0;
float errorAnterior = 0;

// LED en pin 5
const int LED = 5;

void setup() {
  Serial.begin(9600);


  

  // Configurar LED como salida
  pinMode(LED, OUTPUT);
}

void loop() {

  // Leer valor de la fotoresistencia
  int valorLuz = analogRead(LDR);

  error = referencia - valorLuz; //error que recibo desd e

  errorDerivativo = (error -errorAnterior) / 0.1;
  errorIntegral = errorIntegral + (ki*error);
  U = (kp * error) + errorIntegral + (kd*errorDerivativo); //accion correctora




  int pwmOut = (int)U;
  if (pwmOut > 255) pwmOut = 255;
  if (pwmOut < 0)   pwmOut = 0;


  analogWrite(LED, pwmOut);

  Serial.print(referencia);
  Serial.print(",");
  Serial.print(valorLuz);
  Serial.print(",");
  Serial.println(pwmOut);

  errorAnterior = error;

  delay(100);
}