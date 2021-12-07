#include "Medidor.h"
#include <MHZ19_uart.h>     
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>


MHZ19_uart sensor;
LiquidCrystal_I2C display(0x27,16,2);
WiFiClient client;
WiFiManager wifiManager;

unsigned long myChannelNumber = 1592487;//1520536; //Número de canal de thinkspeak
const char * myWriteAPIKey =  "7JO5BIQLDJNLAX2K"; //"B2LWRROGC39KW4YR"; //API key de thinkspeak


Medidor::Medidor(void) {
  
  /*
   *Los pines D1 (GPIO 05) y D2 (GPIO 04) están reservados para el display I2C. 
   *El Pin D3 (GPIO 00) no se puede utilizar por que esta desisgnado para la memoria flash.
  */
  
  rx_pin = 14; //GPIO
  tx_pin = 12; //GPIO
  ledR_pin = 02; //GPIO
  ledG_pin = 10; //GPIO
  ledB_pin = 16; //GPIO
  buzzer_pin = 13; //GPIO
  pulsador_pin = 15; //GPIO (Viene preconfigurado como pulldown)
  numero_de_serie = "0000";
}

void Medidor::conectar() {
   pinMode(ledR_pin, OUTPUT);
   pinMode(buzzer_pin, OUTPUT);
   rgb('a');
   display.begin();
   display.backlight();
   alarma(1, 250, 'r');
   
  //wifiManager.resetSettings(); //<- Descomentar para resetear configuración

   Serial.println("Buscando Señal WIFI / Punto De Acceso");
   displayPrint(0, 0, "Buscando WIFI");
   displayPrint(0, 1, "Punto De Acceso");
   wifiManager.autoConnect("ESP8266Temp");
   display.clear();
   Serial.println("Conectando..."); 
   displayPrint(0, 0, "Conectando...");
   delay(10000);
   display.clear();
   Serial.println("Ya estás conectado");
   displayPrint(0, 0, "Estas Conectado");
   
   ThingSpeak.begin(client);
}

void Medidor::reconectar() { 
  display.clear();
  while(WiFi.status() != WL_CONNECTED){
    alarma(4, 250, 'r');
    Serial.println("No hay conexión Buscando WIFI");
    displayPrint(0, 0, "No hay Conexion");
    displayPrint(0, 1, "Buscando WIFI");
    delay(60000); 
    display.clear();
    Serial.println("No hay conexión Reiniciar");
    displayPrint(0, 0, "No hay Conexion");
    displayPrint(0, 1, "Reiniciar");
    wifiManager.autoConnect("ESP8266Temp");
    display.clear(); 
    alarma(2, 250, 'g');
    Serial.println("Conectado");
    displayPrint(0, 0, "Conectado"); 
    delay(5000); 
   }
}


void Medidor::iniciar() {
  pinMode(ledR_pin, OUTPUT);
  pinMode(ledG_pin, OUTPUT);
  pinMode(ledB_pin, OUTPUT);
  rgb('a');
  pinMode(buzzer_pin, OUTPUT);
  pinMode(pulsador_pin, INPUT);
  display.begin();          
  display.clear();
  display.backlight();
  alarma(1, 250, 'b');
  Serial.print("N° de serie " + numero_de_serie + "\n");
  Serial.print("INICIANDO \n");
  displayPrint(0, 0, "N/S: " + numero_de_serie);
  displayPrint(0, 1, "INICIANDO");       
  logoUNAHUR();
  delay(10000);
  calentar();
}

void Medidor::calentar() {
  display.clear();
  sensor.begin(rx_pin, tx_pin); 
  sensor.setAutoCalibration(false);
  Serial.print("Calentando, espere 1 minuto \n");
  displayPrint(0, 0, "Calentando");       
  displayPrint(0, 1, "Espere 1 minuto"); 
  delay(60000); 
  display.clear();
  alarma(3, 250,'g');
}

void Medidor::verificarEstadoPulsador() {
  if (digitalRead(pulsador_pin) == HIGH) {
    alarma(1, 250, 'b');
    calibrar();
  }
}

void Medidor::presentarMedidor() {
  Serial.print("AireNuevo UNAHUR \n"); 
  Serial.print("MEDIDOR de CO2 \n");  
  display.clear();
  displayPrint(0, 0, "AireNuevo UNAHUR");     
  displayPrint(0, 1, "MEDIDOR de CO2");       
  delay(5000);
}

void Medidor::sensarCO2() {
  display.clear();
  displayPrint(0, 0, "Aire Nuevo");
  while(sensor.getPPM() >= 1200) {  
    alarmaCO2(1, 250);                             
    imprimirCO2(sensor.getPPM());
  }
  int co2ppm = sensor.getPPM();
  imprimirCO2(co2ppm);
  if(co2ppm >= 1000){
    alarmaCO2(4, 500);             
  }
  else if(co2ppm >= 800) {
    alarmaCO2(2, 1000);
  }
  else if(co2ppm >= 600) {
    rgb('y');
  }
  else if(co2ppm < 600) {
    rgb('g');
  }
  displayPrint (13,1," ");
  displayPrint (14,1," ");
  displayPrint (15,1," ");
  scrollAireNuevo();
  delay(5000);  
}

void Medidor::displayPrint(int posicion, int linea, String texto) {
  display.setCursor(posicion, linea);
  display.print(texto);
}

void Medidor::imprimirCO2(int co2ppm) {
  Serial.print("CO2: " + String(co2ppm) + "ppm \n");     
  displayPrint(0, 1, "          ");                      
  displayPrint(0, 1, "CO2: " + String(co2ppm) + "ppm"); 
  delay (100);  
  logoUNAHUR(); 
  delay (5000);
  ThingSpeak.setField(1, co2ppm);         
  if (co2ppm >0){
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful.");
  }
  else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }                                 
}
}

void Medidor::logoUNAHUR() {
  // Primero creamos todos los caracteres necesarios para dibujar el logo
  byte UNAHUR1[] = { 
    B11100,
    B11110,
    B11111,
    B11111,
    B11111,
    B11111,
    B01111,
    B00111
  };
  byte UNAHUR2[] = { 
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111
  };
  byte UNAHUR3[] = { 
    B00111,
    B01111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11110,
    B11100
  };
  display.createChar(0, UNAHUR1); 
  display.createChar(1, UNAHUR2); 
  display.createChar(2, UNAHUR3); 
  display.setCursor(13, 0);       
  display.write(0);               
  display.setCursor(14, 0);      
  display.write(1);
  display.setCursor(15, 0);       
  display.write(2);
  display.setCursor(13, 1);      
  display.write(2);
  display.setCursor(14, 1);      
  display.write(1);
  display.setCursor(15, 1);       
  display.write(0);
}

void Medidor::scrollingText(uint8_t scrolled_by) {
  for (uint8_t i=0;i<16;i++) {
    display.setCursor(i,0);
    if (scrolled_by>=16) scrolled_by=0;   
    if (scrolled_by<15) display.print(str_to_print[scrolled_by]);       
    else display.print(' ');                       
    scrolled_by++;
  }
}

void Medidor::scrollAireNuevo() {
  for (uint8_t i=0;i<STR_LEN;i++) {
    scrollingText(i);
    delay(500);
  }
}

void Medidor::calibrar()
{
  const long segundosEspera = 1800;        
  long segundosPasados = 0;                 
  // Print por serial
  Serial.print("COMIENZA CALIBRACION \n");  
  // Print por display
  display.clear();                       
  displayPrint(0, 0, "COMIENZA");
  displayPrint(0, 1, "CALIBRACION");
  delay(10000);                             
 
  while(segundosPasados <= segundosEspera) {                      
    if (++segundosPasados % 60 == 0) {                                                                
      Serial.print(String(segundosPasados / 60) + " minutos \n");  
    }
    display.clear();                                             
    displayPrint(0, 0, "CALIBRANDO");                  
    displayPrint(0, 1, String(segundosPasados / 60) + " minutos");
    delay(1000);
    }
  sensor.calibrateZero();                   
  Serial.print("PRIMERA CALIBRACION \n");   
  display.clear();                          
  displayPrint(0, 0, "PRIMERA");             
  displayPrint(0, 1, "CALIBRACION");     
  alarma(1, 250, 'b');                        
  delay(60000);                               
  sensor.calibrateZero();                    
  Serial.print("SEGUNDA CALIBRACION \n");
  display.clear();                            
  displayPrint(0, 0, "SEGUNDA");        
  displayPrint(0, 1, "CALIBRACION");    
  alarma(1, 250, 'b');                     
  delay(10000);
  Serial.print("CALIBRACION TERMINADA \n");
  display.clear();                         
  displayPrint(0, 0, "CALIBRACION");       
  displayPrint(0, 1, "TERMINADA");    
  alarma(5, 250, 'g');                      
  delay(10000);
}

void Medidor::rgb(char color) {
  switch (color) {
    case 'r': //red
      digitalWrite(ledR_pin, LOW);
      digitalWrite(ledG_pin, HIGH);
      digitalWrite(ledB_pin, HIGH);
      break;
    case 'g': //green
      digitalWrite(ledR_pin, HIGH);
      digitalWrite(ledG_pin, LOW);
      digitalWrite(ledB_pin, HIGH);
      break;
    case 'y': //yellow
      digitalWrite(ledR_pin, LOW);
      digitalWrite(ledG_pin, LOW);
      digitalWrite(ledB_pin, HIGH);
      break;
    case 'b': //blue
      digitalWrite(ledR_pin, HIGH);
      digitalWrite(ledG_pin, HIGH);
      digitalWrite(ledB_pin, LOW);
      break;
    case 'a': //apagado
      digitalWrite(ledR_pin, HIGH);
      digitalWrite(ledG_pin, HIGH);
      digitalWrite(ledB_pin, HIGH);
      break;
  }
}

void Medidor::alarma(int veces, int duracionNota, char color) {
  rgb(color);                                 
  for(int i=0; i<veces; i++) {
    sonarAlarma(duracionNota); 
    delay(duracionNota);
  }
}

void Medidor::alarmaCO2(int veces, int duracionNota) {
  for(int i=0; i<veces; i++) {
    rgb('r');                                   
    sonarAlarma(duracionNota); 
    if(i<veces-1 or veces==1) {                 
      rgb('a');                                                  
    }
    delay(duracionNota); 
  }
}

void Medidor::sonarAlarma(int duracionNota) {
  tone(buzzer_pin, NOTE_C7, duracionNota);   
  delay(duracionNota);
  noTone(buzzer_pin);
}
