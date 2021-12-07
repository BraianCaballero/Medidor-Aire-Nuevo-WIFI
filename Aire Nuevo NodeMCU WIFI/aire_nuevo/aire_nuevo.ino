/*----------------------------------------------------------
    Medidor de CO2 en Arduino Nano utilizando OOP
  ----------------------------------------------------------*/
#include "Medidor.h"


Medidor medidor;
long loops = 0;

void setup() {
     
  Serial.begin(9600);
  medidor.conectar();
  delay (2000);
  medidor.iniciar();

}

void loop() {
  medidor.reconectar();
  medidor.verificarEstadoPulsador();
  if(loops == 30) {
    medidor.presentarMedidor();
    loops = 0;
  }
  medidor.sensarCO2();
  loops++;
}
