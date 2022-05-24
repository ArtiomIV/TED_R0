#include <OneWire.h>
#include <DallasTemperature.h>

/*Definizione pin*/
/*Sonde*/
OneWire oneWireL1(14);
OneWire oneWireL2(15);
OneWire oneWireL3(16);
DallasTemperature senL1(&oneWireL1);
DallasTemperature senL2(&oneWireL2);
DallasTemperature senL3(&oneWireL3);

/*definizione variabili gloabali*/
bool okstartsys = false;
float velmos = 0; /*variable di appoggio per monitoraggio di mosfet*/

/*parametri di impostazione*/
float tempstopF1 = 4.0; /*al di sotto di questa temperatura l'impianto passa in standby*/
float tempmaxven1F2 = 35.0; /*temperatura massima per la ventola 1*/
float tempminven1F3 = 20.0; /*temperatura minima per la venotla 2*/
float tempmaxven2F4 = 45.0; /*temperatura massima per la ventola 2*/
float tempminven2F5 = 35.0; /*temperatura minima per la ventola 2*/
float tempstartPF6 = 8.0; /*temperatura al raggiungimento della quale partono le pompe*/
float tempmaxmosfF7 = 20.0; /*al raggiungimento di questa temperatura parte il mosfet al 100%*/
float tempminmosfF8 = 8.0; /*al raggiungimento di questa temperatura parte il mosfet al 30%*/


/*definizione uscite*/
#define cMosPwm 13
#define cVen1Pwm 9 /*9*/
#define cVen2Pwm 10/*10*/
#define cp1 11
#define cp2 12

void setup() {
  Serial.begin(9600);
  
  /*porta lo stato del sistema in standby*/
  okstartsys = false;
  
  /*definizione modalità dei pin*/
  pinMode(cVen1Pwm, OUTPUT);
  pinMode(cVen2Pwm, OUTPUT);
  pinMode(cMosPwm, OUTPUT);
  pinMode(cp1, OUTPUT);
  pinMode(cp2, OUTPUT);

  /*inizializzazione sonde*/
  senL1.begin();
  senL2.begin();
  senL3.begin();
}

void loop() {
  /*ricaviamo le temperature in celsius*/
  float tcSen1 = getTemperature(senL1);
  float tcSen2 = getTemperature(senL2);
  float tcSen3 = getTemperature(senL3);

  /*Serial.print("Sonda L1: ");
  Serial.println(tcSen1);
  Serial.print("Sonda L2: ");
  Serial.println(tcSen2);
  Serial.print("Sonda L3: ");
  Serial.println(tcSen3);
  Serial.print("Mosfet: ");
  Serial.println(velmos);
  Serial.print("Stato Sitema: ");
  Serial.println(okstartsys);*/
  
 
  /*consenso da parte della sonda L3*/
  if (tcSen3 <= tempstopF1) {
    okstartsys = false;
    }
  else {
    okstartsys = true;
    }

  /*istruzione switch che in base allo stato attiva o arresta il ciclo del programma*/
  switch (okstartsys){
    case false:
      analogWrite(cVen1Pwm, 0);
      analogWrite(cVen2Pwm, 0);
      analogWrite(cMosPwm, 0);
      digitalWrite(cp1, LOW);
      digitalWrite(cp2, LOW);
      break;
        
    case true:
      /*comando ventole*/
      float velven1 = cVen(tcSen1, cVen1Pwm, tempmaxven1F2, tempminven1F3, 100, 30);
      float velven2 = cVen(tcSen1, cVen2Pwm, tempmaxven2F4, tempminven2F5, 100, 30);
      
      /*Serial.print("Ventola1: ");
      Serial.println(velven1);
      Serial.print("Ventola2: ");
      Serial.println(velven2);*/
      
      /*comando pompe*/
      if (tcSen3 >= tempstartPF6) {
        digitalWrite(cp1, HIGH);
        digitalWrite(cp2, HIGH);
        }
        
      /*comando mosfet*/
      if (tcSen2 >= tempmaxmosfF7) {
        velmos = genPwm(100.0, cMosPwm);
        }
      else if (tcSen2 <= tempminmosfF8) {
        velmos = genPwm(30.0, cMosPwm);
        }
      break;    
    }
}

/*funzoine che ricava la temperatura*/
float getTemperature(DallasTemperature device) {
   device.requestTemperatures();
   float tc = device.getTempCByIndex(0);
   return tc;
  }

/*funzione che genera il pwm*/
float genPwm(float val, int pin) {
  float duty = map(val, 0, 100, 0, 255);
  analogWrite(pin, duty);
  return(val);
  }

/*funzione che comanda la ventola in pwm*/
float cVen(float temp, int pin, float maxx1, float minx1, float maxx2, float minx2) {
  if (temp >= minx1 and temp <= maxx1) {
    float normX = (temp - minx1)/(maxx1-minx1); /*normalizzazione da 0 a 1*/
    float scaleX = ((maxx2-minx2)*normX) + minx2; /*scalatura da 0 a 100*/
    float duty = map(scaleX, 0, 100, 0, 255);
    analogWrite(pin, duty);
    return(scaleX);
  } 
  else if (temp < minx1){
    float duty = map(minx2, 0, 100, 0, 255);
    analogWrite(pin, duty);
    return(minx);
    }
  else if (temp > maxx1) {
    float duty = map(maxx2, 0, 100, 0, 255);
    analogWrite(pin, duty);
    return(maxx2);
    }
  }
