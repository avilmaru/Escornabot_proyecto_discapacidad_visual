// ---------------------------------------------------------------------------
// Escornabot: Proyecto para niños con discapacidad visual o ceguera
//
// AUTOR/LICENCIA:
//
// Creado por Angel Villanueva - @avilmaru
// Licencia Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0).
//
// LINKS:
// Blog: http://www.mecatronicalab.es
//
//
// HISTORICO:
// 12/01/2019 v1.0 - Release inicial.
//
// ---------------------------------------------------------------------------


/**************************************************************************
 * Libreria  MFRC522 
 * https://github.com/miguelbalboa/rfid
 *
**************************************************************************/

#include <MFRC522.h>  

#define MFRC522_SS  7
#define MFRC522_RST 6

MFRC522 mfrc522(MFRC522_SS, MFRC522_RST);

/**************************************************************************
 * Libreria Adafruit para el TFT
 * https://github.com/adafruit/Adafruit-GFX-Library/
 * https://github.com/adafruit/Adafruit-ST7735-Library
 * https://learn.adafruit.com/adafruit-1-44-color-tft-with-micro-sd-socket
**************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735

#define TFT_SS     10
#define TFT_RST    9 
#define TFT_DC     8

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_SS, TFT_DC, TFT_RST);


/**************************************************************************
 * Libreria Adafruit NeoPixel 
 * https://github.com/adafruit/Adafruit_NeoPixel
 * https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use
 
**************************************************************************/

#include <Adafruit_NeoPixel.h>

#define LEDS_PIN   1
#define LEDS_NUM   12

Adafruit_NeoPixel leds = Adafruit_NeoPixel(LEDS_NUM, LEDS_PIN, NEO_GRB + NEO_KHZ800);


/**************************************************************************
 * Libreria SPI 
 * https://www.arduino.cc/en/Reference/SPI
**************************************************************************/

#include <SPI.h>

/**************************************************************************
 * SoftwareSerial 
 * https://www.arduino.cc/en/Reference/SoftwareSerial
**************************************************************************/

#include <SoftwareSerial.h>

#define MP3_RX     2
#define MP3_TX     3

SoftwareSerial DFPlayerSerial(MP3_RX, MP3_TX); 

#define BT_RX     4
#define BT_TX     5

SoftwareSerial BluetoothSerial(BT_RX, BT_TX); 

/**************************************************************************
 *  Libreria DFplayer_Mini_Mp3 
 *  https://github.com/DFRobot/DFRobotDFPlayerMini/archive/1.0.3.zip
 *  https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299?gclid=Cj0KCQiAgMPgBRDDARIsAOh3uyJ0khC-JAQJAYE9RPfLJ2Yn3PMi5aKbI7p3jodMRn66rii3eGafgx8aAnpNEALw_wcB

**************************************************************************/

#include "DFRobotDFPlayerMini.h"

DFRobotDFPlayerMini myDFPlayer;


/**************************************************************************
 * Identificadores de tarjeta
**************************************************************************/

// NOTA: Cambiar por los códigos de las tarjetas que se tengan (6 tarjetas son necesarias)

byte tarjetaGirarIzquierda[4]  = {0x71, 0x9C, 0x07, 0x01};   
byte tarjetaGirarDerecha[4]    = {0xB4, 0xBD, 0xD9, 0x13};
byte tarjetaAvanzar[4]         = {0x01, 0xB5, 0x9A, 0x00}; 
byte tarjetaRetroceder[4]      = {0x71, 0x3C, 0x27, 0x49}; 
byte tarjetaEjecutar[4]        = {0xC3, 0x5B, 0xED, 0x16};  
byte tarjetaResetear[4]        = {0x66, 0x7A, 0xCA, 0x1F}; 


/**************************************************************************/

bool instruccionesEnCurso = false;
bool msgErrorMostrado = false;

/**************************************************************************/

void setup(void) {

    
    // Inicialización TFT
    tft.initR(INITR_144GREENTAB);  
    tft.fillScreen(ST77XX_BLACK);  

    escribeTexto(5,40,3,"ESPERA.....",ST77XX_GREEN);
    
    // Inicialización LEDS  
    leds.begin(); 
    leds.setBrightness(200);
    leds.clear();
    leds.show(); 

    //Inicializacion puerto serie Bluetooth 
    BluetoothSerial.begin(9600);
   
    //Inicializacion puerto serie para DFPlayer
    DFPlayerSerial.begin(9600);
    
    // Inicialización MP3
    while (!myDFPlayer.begin(DFPlayerSerial)) 
    { 
      if (!msgErrorMostrado)
      {
        msgErrorMostrado = true;
        tft.fillScreen(ST77XX_BLACK);
        escribeTexto(0,0,2,"MP3 ERROR! Verifica conexiones y que la tarjeta SD este insertada",ST77XX_RED);
      }
    }
    
    myDFPlayer.volume(25);
    
    // Inicialización protocolo SPI
    SPI.begin();   
    
    // Inicialización MFRC522 
    mfrc522.PCD_Init();    

    // Ya estamos listos para empezar!
    tft.fillScreen(ST77XX_BLACK); 
    escribeTexto(21,55,3,"LISTO",ST77XX_GREEN);
    delay(2000);
    tft.fillScreen(ST77XX_BLACK); 

}

void loop() {

  /* 
   * ---------------------
   * LECTURA TARJETAS RFID
   * ---------------------
   */
  
    lecturaTarjetasRFID();
  
  /*
   * ------------------------------
   * LECTURA MÓDULO BLUETOOTH HC-05
   * ------------------------------
   */

    lecturaBluetooth();
  

}



/**************************************************************************
 * 
 * FUNCIONES
 * 
**************************************************************************/

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/* 
 *  Función que lee el código de la tarjeta RFID y la compara con los códigos de movimientos y acciones almacenados
 *  Si hay coincidencia se enviará al escornabot vía bluetooth las acción a realizar
 */
void lecturaTarjetasRFID()
{

  byte codigoLeido[4] = {0,0,0,0};        // Almacenará el código leído
   
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
  
    for (byte i = 0; i < mfrc522.uid.size; i++) {
       
            codigoLeido[i]=mfrc522.uid.uidByte[i];          
    }
    
    // Terminamos la lectura de la tarjeta tarjeta actual
    mfrc522.PICC_HaltA(); 
    mfrc522.PCD_StopCrypto1();

    
    // Comparamos el código leído para ver si coincide con los códigos de movimientos y acciones
    if (compareArray(codigoLeido,tarjetaGirarIzquierda))
    {
      if (instruccionesEnCurso)
        accion("stop");
      else
        movimiento("izquierda",false);  
    }
    else if (compareArray(codigoLeido,tarjetaGirarDerecha))
    {
      if (instruccionesEnCurso)
        accion("stop");
      else
        movimiento("derecha",false);  
    }
    else if (compareArray(codigoLeido,tarjetaAvanzar))
    {
      if (instruccionesEnCurso)
        accion("stop");
      else
        movimiento("avanzar",false);  
    }
    else if (compareArray(codigoLeido,tarjetaRetroceder))
    {
      if (instruccionesEnCurso)
        accion("stop");
      else
        movimiento("retroceder",false);  
    }
    else if (compareArray(codigoLeido,tarjetaEjecutar))
    {
      if (instruccionesEnCurso)
        accion("stop");
      else
        accion("ejecutar");  
    }
          
    else if (compareArray(codigoLeido,tarjetaResetear))
    {
      if (instruccionesEnCurso)
        accion("stop");
      else
        accion("resetear"); 
    }
               
   
  }
  
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/*
 * Función que lee los datos recibidos por bluetooth procedentes del escornabot 
 */
void lecturaBluetooth()
{

  BluetoothSerial.listen();
  
  if (BluetoothSerial.available() > 0)    // Si llega un dato por el puerto Bluetooth
  {
      char comando =  BluetoothSerial.read();
  
      switch (comando) {      
      case 'N':
        movimiento("avanzar",true);
        break;
      case 'S':
        movimiento("retroceder",true);
        break;
      case 'W':
        movimiento("izquierda",true);
        break;   
      case 'E':
        movimiento("derecha",true);
        break;
      case 'Z':
        accion("error");
        break;
      case 'X':
        accion("fin ok");
        break;    
      case 'Y':
        accion("fin ko");
        break;       
     
      } 
  }
  
}

void enviarInstruccion(String instruccion)
{
    
  if (instruccion == "izquierda")
      BluetoothSerial.write("w\n");
  else if (instruccion == "derecha")
      BluetoothSerial.write("e\n");
  else if (instruccion == "avanzar")
      BluetoothSerial.write("n\n");
   else if (instruccion == "retroceder")
      BluetoothSerial.write("s\n");
   else if (instruccion =="ejecutar")
      BluetoothSerial.write("g\n");
   else if (instruccion == "resetear" || instruccion == "stop")
      BluetoothSerial.write("r\n");
    
}


// Función para comparar dos arrays
boolean compareArray(byte array1[],byte array2[])
{
  if(array1[0] != array2[0])
    return(false);
  if(array1[1] != array2[1])
    return(false);
  if(array1[2] != array2[2])
    return(false);
  if(array1[3] != array2[3])
    return(false);
    
  return(true);
}


void escribeTexto(uint16_t x, uint16_t y, uint8_t ts, char *text, uint16_t color) 
{
  tft.setCursor(x, y);
  tft.setTextSize(ts);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}


void efectoLeds(String instruccion)
{

  uint32_t color;
  uint32_t rojo = leds.Color(255, 0, 0);
  uint32_t verde = leds.Color(0, 255, 0);
  uint32_t azul = leds.Color(0, 0, 255);
  uint32_t amarillo = leds.Color(255, 255, 0);
  uint32_t negro = leds.Color(0, 0, 0);
  uint32_t blanco = leds.Color(255, 255, 255);
  uint32_t naranja = leds.Color(255, 050, 0);
  uint32_t morado = leds.Color(150, 0, 220);

  if (instruccion == "izquierda")
    color = rojo;
  else if (instruccion == "derecha")
    color = verde;
  else if (instruccion == "avanzar")
    color = azul;  
  else if (instruccion == "retroceder")
    color = amarillo;  
  else if (instruccion == "ejecutar")
  {
     efectoRuleta(morado, 50, 10); 
     return;
  }
  else if (instruccion == "resetear")
  {
     efectoRuleta(naranja, 50, 10); 
     return;
  }     
  else if (instruccion == "stop")
  {
     efectoRuleta(rojo, 50, 10); 
     return;
  }     
  else if (instruccion == "error")
  {
     efectoRuleta(rojo, 50, 15); 
     return;
  }     
  else if (instruccion == "fin ok")
  {
     efectoRuleta(verde, 50, 15); 
     return;
  }    
  else if (instruccion == "fin ko")
  {
     efectoRuleta(rojo, 50, 15); 
     return;
  }    
  
  for(int i=0;i<LEDS_NUM;i++)
  { 
    leds.setPixelColor(i, color); 
  }
  
  leds.show(); 
  
}

void efectoRuleta(uint32_t c, uint8_t wait, uint8_t num) {
  
  for (int j=0; j<num; j++) 
  {  
    for (int q=0; q < 3; q++) 
    {
      for (uint16_t i=0; i < leds.numPixels(); i=i+3) {
        leds.setPixelColor(i+q, c);    
      }
      
      leds.show();
      
      delay(wait);

      for (uint16_t i=0; i < leds.numPixels(); i=i+3) {
        leds.setPixelColor(i+q, 0);       
      }
      
    }
    
  }
  
}  

void decirFrase(String instruccion, bool ejecutando = false)
{

  /*  
  Locuciones presentes en los archivos mp3:
  
  0001.mp3: "Izquierda"
  0002.mp3: "Derecha"
  0003.mp3: "Avanzar"
  0004.mp3: "Retroceder"
  0005.mp3: "Ejecutar"
  0006.mp3: "Resetear"
  
  0007.mp3: "Girando a la izquierda"
  0008.mp3: "Girando a la derecha"
  0009.mp3: "Avanzando"
  0010.mp3: "Retrocediendo"
  
  0011.mp3: "¡comenzados!  
  0012.mp3 :"¡Vaya!, creo que me he perdido, ¿volvemos a intentarlo?
  0013.mp3 : "¡Hemos terminado!, ¡lo has hecho genial!"
  0014.mp3 : "¡Vaya!, no he llegado al final, ¿volvemos a intentarlo?
  0015.mp3 : "Parando el robot y reseteando movimientos"

  */

  DFPlayerSerial.listen();
  
  byte indice = 0;
 
  if (instruccion == "izquierda")
  {
    if (ejecutando)
      indice=7;
    else
      indice=1;
  }
  else if (instruccion == "derecha")
  {
    if (ejecutando)
      indice=8;
    else
      indice=2;
  }
  else if (instruccion == "avanzar")
  {
    if (ejecutando)
      indice=9;
    else
      indice=3;
  }
  else if (instruccion == "retroceder")
  {
    if (ejecutando)
      indice=10;
    else
      indice=4;
  }
  else if (instruccion == "ejecutar")
      indice=5;
  else if (instruccion == "resetear")
      indice=6;
  else if (instruccion == "comenzar")
      indice=11;
  else if (instruccion == "error")
      indice=12;    
  else if (instruccion == "fin ok")
      indice=13; 
  else if (instruccion == "fin ko")
      indice=14; 
  else if (instruccion == "stop")
      indice=15; 
          
  if (indice > 0)
    myDFPlayer.play(indice);
       
}

void dibujarBoton(String instruccion) 
{
  
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRoundRect(5, 5, 118, 118, 5, ST77XX_WHITE);
  
  if (instruccion == "izquierda")
    tft.fillTriangle(100, 28, 100, 100, 28, 64, ST77XX_RED);
  else if (instruccion == "derecha")
    tft.fillTriangle(28, 28, 28, 100, 100, 64, ST77XX_GREEN); 
  else if (instruccion == "avanzar")
    tft.fillTriangle(28, 100, 100, 100, 64, 28, ST77XX_BLUE);
  else if (instruccion == "retroceder")
    tft.fillTriangle(28, 28, 100, 28, 64, 100, ST77XX_YELLOW);
  else if (instruccion == "ejecutar")
  {
    tft.fillRoundRect(28, 28, 72, 72, 3, ST77XX_MAGENTA);
    escribeTexto(48,55,3,"OK",ST77XX_BLACK);
  }
  else if (instruccion == "resetear")
  {
    tft.fillRoundRect(14, 28, 100, 72, 3, ST77XX_ORANGE);
    escribeTexto(21,55,3,"RESET",ST77XX_BLACK);
  }
  else if (instruccion == "stop")
  {
    tft.fillRoundRect(14, 28, 100, 72, 3, ST77XX_RED);
    escribeTexto(30,55,3,"STOP",ST77XX_BLACK);
  }
  else if (instruccion == "error")
  {
    tft.fillRoundRect(14, 28, 100, 72, 3, ST77XX_RED);
    escribeTexto(21,55,3,"ERROR",ST77XX_BLACK);
  }
  else if (instruccion == "fin ok")
  {
    tft.fillRoundRect(14, 28, 100, 72, 3, ST77XX_GREEN);
    escribeTexto(35,55,3,"FIN",ST77XX_BLACK);
  }
  else if (instruccion == "fin ko")
  {
    tft.fillRoundRect(14, 28, 100, 72, 3, ST77XX_RED);
    escribeTexto(21,45,3,"FINAL NO OK",ST77XX_BLACK);
  }
    
}

void movimiento(String movimiento, bool ejecutando)
{
    dibujarBoton(movimiento);
    efectoLeds(movimiento);
    
    if (ejecutando)
    {
      decirFrase(movimiento, ejecutando); 
    }
    else
    {
      enviarInstruccion(movimiento);
      decirFrase(movimiento, ejecutando); 
      delay(1500); 
      reset();
    }
   
}


void accion(String instruccion)
{
  
  if (instruccion == "ejecutar"){
  
    decirFrase(instruccion);
    dibujarBoton(instruccion);
    efectoLeds(instruccion);
    reset();
    
    decirFrase("comenzar"); 
    delay(1200);
    enviarInstruccion(instruccion);
    
    instruccionesEnCurso = true;
  
  }else if (instruccion == "resetear" || instruccion == "stop"){

    if (instruccion == "stop")
      instruccionesEnCurso = false;
    
    enviarInstruccion(instruccion);
    decirFrase(instruccion);
    dibujarBoton(instruccion);
    efectoLeds(instruccion);   
    reset();
  
  }else if (instruccion == "error" || instruccion == "fin ok" || instruccion == "fin ko"){

    instruccionesEnCurso = false;
    decirFrase(instruccion);
    dibujarBoton(instruccion);
    efectoLeds(instruccion); 
    reset();
  
  }
    
}

void reset()
{
  leds.clear();
  leds.show();
  tft.fillScreen(ST77XX_BLACK);
}
