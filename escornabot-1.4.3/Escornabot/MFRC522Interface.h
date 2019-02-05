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

#include <Arduino.h>

#ifndef MFRC522INTERFACE_H_
#define MFRC522INTERFACE_H_


class MFRC522Interface {
  
public:

	MFRC522Interface();
	virtual ~MFRC522Interface();
  bool cardDetected();
  bool matchingCard();

private:
  
	void initialize();
  void finalize();
  bool compareArray(byte array1[],byte array2[]);

};


#endif /* MFRC522INTERFACE_H_ */
