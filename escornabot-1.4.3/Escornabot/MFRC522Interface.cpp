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

#include "MFRC522Interface.h"
#include "Configuration.h"

#include <MFRC522.h>

MFRC522 mfrc522(MFRC522_SS_PIN, MFRC522_RST_PIN);

#include <SPI.h>

byte _uidForMatch[4]  = {UID1, UID2, UID3, UID4}; 

MFRC522Interface::MFRC522Interface() {

}

MFRC522Interface::~MFRC522Interface() {

}

void MFRC522Interface::initialize() {

  // init SPI bus
  SPI.begin();
  
  // init MFRC522
  mfrc522.PCD_Init();
  
}

void MFRC522Interface::finalize() {
  
  mfrc522.PICC_HaltA(); 
  mfrc522.PCD_StopCrypto1();
  SPI.end();
  
}


bool  MFRC522Interface::compareArray(byte array1[],byte array2[])
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


bool MFRC522Interface::cardDetected() {

  this->initialize();

  // Intentamos hasta 3 veces la lectura de la tarjeta
  for (int i=1; i <= 3; i++)
  {
  	if (mfrc522.PICC_IsNewCardPresent())
    {
      this->finalize();
      return true;
    }
  }

  this->finalize();
  return false;
}

bool MFRC522Interface::matchingCard() {

  byte _uid[4] = {0,0,0,0};       
  
  this->initialize();

  // Intentamos hasta 3 veces la lectura de la tarjeta
  for (int i=1; i <= 3; i++)
  { 
      if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
      {
          for (byte i = 0; i < mfrc522.uid.size; i++)
          {
                  _uid[i]=mfrc522.uid.uidByte[i];          
          }
      
          this->finalize();
       
          if (compareArray(_uid,_uidForMatch))
            return true;
          else
            return false;          
      } 
  }

  this->finalize();
  return false;
  
}
