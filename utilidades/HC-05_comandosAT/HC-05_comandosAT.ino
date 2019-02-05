
/* 
 
Programa para enviar comandos AT Modulo HC-05

 */
 
#include <SoftwareSerial.h>

#define BT_RX     4
#define BT_TX     5

SoftwareSerial BTSerial(BT_RX, BT_TX); 

char c;
 
void setup() 
{
    Serial.begin(9600);
    Serial.println("Inicializado Monitor Serie de Arduino a 9600 baudios");
 
    BTSerial.begin(38400);  
    Serial.println("Inicializado comunicación serie con el HC-05 a 38400 baudios");
}
 
void loop()
{
  
  if (BTSerial.available())
    Serial.write(BTSerial.read());
  
  if (Serial.available())
  {
    c =  Serial.read();
    Serial.write(c);   
    BTSerial.write(c);  
  }
   
    
}

