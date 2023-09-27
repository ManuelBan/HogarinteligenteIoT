#include <Fuzzy.h>
#include <DHT.h>// Definimos el pin digital donde se conecta el sensor
#include <LiquidCrystal.h>
#include <Servo.h>

#define DHTPIN 41// Dependiendo del tipo de sensor                                    
#define DHTTYPE DHT11
Servo servoMotor;

int motor = 13; //Conectar motor aqui
int motorInit = 34;
int ledPin = 12; // declaramos el led en el pin 12 del arduino
int sensorPin=2; // declaramos el sensor PIR en el pin 11
int pinServo=22;
int val = 0; //variable para asignar la lectura del sensor PIR
int LecturaLDR = 0;

// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

int myArray[15]; 
byte* ddata = reinterpret_cast<byte*>(&myArray); //Este bloque de código es para comunicacion entre py y arduino
size_t pcDataLen = sizeof(myArray);
bool newData=false;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//########################################################################################################## Controlador 1, ventilador
// Se crea una estructura de datos Fuzzy
Fuzzy *fuzzy1 = new Fuzzy();

//Se guardan en punteros a una estructura de datos fuzzy set
// Entrada 1: Temperatura interior 
FuzzySet *muyb_ti = new FuzzySet(5, 5, 5, 10);
FuzzySet *baja_ti = new FuzzySet(7, 13, 13, 19);
FuzzySet *media_ti = new FuzzySet(16, 22.5, 22.5, 29);
FuzzySet *alta_ti = new FuzzySet(26, 32, 32, 38);
FuzzySet *muya_ti = new FuzzySet(35, 40, 40, 40);

// Entrada 2: Temperatura exterior 
FuzzySet *baja_te = new FuzzySet(5, 5, 10, 15);
FuzzySet *mediab_te = new FuzzySet(11.5, 20.5, 20.5, 29.5);
FuzzySet *mediaa_te = new FuzzySet(25.5, 34.5, 34.5, 43.5);
FuzzySet *alta_te = new FuzzySet(40, 45, 50, 50);

// Salida: Voltaje
FuzzySet *muyb_v = new FuzzySet(0, 0, 1, 2);
FuzzySet *bajo_v = new FuzzySet(1.5, 2.5, 2.5, 3.5);
FuzzySet *medio_v = new FuzzySet(3, 4, 4, 4.9);
FuzzySet *alto_v = new FuzzySet(4.8, 5, 5, 5);

//########################################################################################################## Controlador 2, ventana
Fuzzy *fuzzy2 = new Fuzzy();

// Fuzzy  Humedad interna
FuzzySet *muybi = new FuzzySet(0, 0, 0, 8);
FuzzySet *bajai = new FuzzySet(5, 15, 15, 23);
FuzzySet *mediai = new FuzzySet(20, 30, 30, 40);
FuzzySet *altai = new FuzzySet(36, 50, 50, 65);
FuzzySet *muyai = new FuzzySet(58, 70, 80, 80); 

// FuzzyInput Humedad exterior
FuzzySet *muybe = new FuzzySet(0, 0, 3, 10);
FuzzySet *bajae = new FuzzySet(7, 17, 17, 25);
FuzzySet *mediae = new FuzzySet(20, 30, 30, 45);
FuzzySet *altae = new FuzzySet(40, 60, 60, 76);
FuzzySet *muyae = new FuzzySet(69, 76, 100, 100); 

// FuzzyOutput Grados de Ventana
FuzzySet *muy_poca = new FuzzySet(0, 0, 0, 14);
FuzzySet *poca = new FuzzySet(8, 25, 25, 42);
FuzzySet *media = new FuzzySet(36, 50, 50, 64);
FuzzySet *alta = new FuzzySet(58, 75, 75, 92);
FuzzySet *muy_alta = new FuzzySet(86, 100, 100, 100);



void setup() {

  Serial.begin(115200); //Inicializamos en 115200 porque es el baudrate necesario para la comunicacion con Arduino
  dht.begin();
  lcd.begin(16, 2);
  pinMode(ledPin, OUTPUT); //El pin 12 del arduino lo asignamos como salida para el led
  pinMode(sensorPin, INPUT);//El pin 11 lo asignamos como entrada para la señal del sensor
  pinMode(motor, OUTPUT);
  pinMode(motorInit, OUTPUT);
  servoMotor.attach(pinServo);
  // Ponemos una semilla random
  randomSeed(analogRead(0)); //Linea necesaria para libreria Fuzzy

//########################################################################################################## Controlador 1, ventilador
  // EntradaFuzzyTemperatura Interior
  FuzzyInput *temperaturaInt = new FuzzyInput(1);

  // asignacion de funciones de pertenencia a la entrada Temperatura interior
  temperaturaInt->addFuzzySet(muyb_ti);
  temperaturaInt->addFuzzySet(baja_ti);
  temperaturaInt->addFuzzySet(media_ti);
  temperaturaInt->addFuzzySet(alta_ti);
  temperaturaInt->addFuzzySet(muya_ti);
  fuzzy1->addFuzzyInput(temperaturaInt); //se añade la entrada al controlador

  // EntradaFuzzyTemperatura Exterior
  FuzzyInput *temperaturExt = new FuzzyInput(2);

  temperaturExt->addFuzzySet(baja_te);
  temperaturExt->addFuzzySet(mediab_te);
  temperaturExt->addFuzzySet(mediaa_te);
  temperaturExt->addFuzzySet(alta_te);
  fuzzy1->addFuzzyInput(temperaturExt);

    // FuzzyOutput
  FuzzyOutput *voltaje = new FuzzyOutput(1);

  voltaje->addFuzzySet(muyb_v);
  voltaje->addFuzzySet(bajo_v);
  voltaje->addFuzzySet(medio_v);
  voltaje->addFuzzySet(alto_v);
  fuzzy1->addFuzzyOutput(voltaje);

  // Construyendo reglas difusas
  // Metodo que guarda la estrutura de datos de la regla si esto y esto en un puntero
  // Regla difusa 1
  FuzzyRuleAntecedent *ifTemperaturaIntmuybAndTemperaturaExtbaja = new FuzzyRuleAntecedent();
  ifTemperaturaIntmuybAndTemperaturaExtbaja->joinWithAND(muyb_ti, baja_te);

  // Regla difusa 2
  FuzzyRuleAntecedent *ifTemperaturaIntmuybAndTemperaturaExtmediab = new FuzzyRuleAntecedent();
  ifTemperaturaIntmuybAndTemperaturaExtmediab->joinWithAND(muyb_ti, mediab_te);

  // Regla difusa 3
  FuzzyRuleAntecedent *ifTemperaturaIntmuybAndTemperaturaExtmediaa = new FuzzyRuleAntecedent();
  ifTemperaturaIntmuybAndTemperaturaExtmediaa->joinWithAND(muyb_ti, mediaa_te);

  // Regla difusa 4
  FuzzyRuleAntecedent *ifTemperaturaIntmuybAndTemperaturaExtalta = new FuzzyRuleAntecedent();
  ifTemperaturaIntmuybAndTemperaturaExtalta->joinWithAND(muyb_ti, alta_te);

  // Regla difusa 5
  FuzzyRuleAntecedent *ifTemperaturaIntbajaAndTemperaturaExtbaja = new FuzzyRuleAntecedent();
  ifTemperaturaIntbajaAndTemperaturaExtbaja ->joinWithAND(baja_ti, baja_te);

  // Regla difusa 6
  FuzzyRuleAntecedent *ifTemperaturaIntbajaAndTemperaturaExtmediab = new FuzzyRuleAntecedent();
  ifTemperaturaIntbajaAndTemperaturaExtmediab ->joinWithAND(baja_ti, mediab_te);

  // Regla difusa 7
  FuzzyRuleAntecedent *ifTemperaturaIntbajaAndTemperaturaExtmediaa = new FuzzyRuleAntecedent();
  ifTemperaturaIntbajaAndTemperaturaExtmediaa ->joinWithAND(baja_ti, mediaa_te);

  // Regla difusa 8
  FuzzyRuleAntecedent *ifTemperaturaIntbajaAndTemperaturaExtalta = new FuzzyRuleAntecedent();
  ifTemperaturaIntbajaAndTemperaturaExtalta ->joinWithAND(baja_ti, alta_te);

  // Regla difusa 9
  FuzzyRuleAntecedent *ifTemperaturaIntmediaAndTemperaturaExtbaja = new FuzzyRuleAntecedent();
  ifTemperaturaIntmediaAndTemperaturaExtbaja ->joinWithAND(media_ti, baja_te);

  // Regla difusa 10
  FuzzyRuleAntecedent *ifTemperaturaIntmediaAndTemperaturaExtmediab = new FuzzyRuleAntecedent();
  ifTemperaturaIntmediaAndTemperaturaExtmediab ->joinWithAND(media_ti, mediab_te);

  // Regla difusa 11
  FuzzyRuleAntecedent *ifTemperaturaIntmediaAndTemperaturaExtmediaa = new FuzzyRuleAntecedent();
  ifTemperaturaIntmediaAndTemperaturaExtmediaa ->joinWithAND(media_ti, mediaa_te);

  // Regla difusa 12
  FuzzyRuleAntecedent *ifTemperaturaIntmediaAndTemperaturaExtalta = new FuzzyRuleAntecedent();
  ifTemperaturaIntmediaAndTemperaturaExtalta ->joinWithAND(media_ti, alta_te);

  // Regla difusa 13
  FuzzyRuleAntecedent *ifTemperaturaIntaltaAndTemperaturaExtbaja = new FuzzyRuleAntecedent();
  ifTemperaturaIntaltaAndTemperaturaExtbaja ->joinWithAND(alta_ti, baja_te);

  // Regla difusa 14
  FuzzyRuleAntecedent *ifTemperaturaIntaltaAndTemperaturaExtmediab = new FuzzyRuleAntecedent();
  ifTemperaturaIntaltaAndTemperaturaExtmediab ->joinWithAND(alta_ti, mediab_te);

  // Regla difusa 15
  FuzzyRuleAntecedent *ifTemperaturaIntaltaAndTemperaturaExtmediaa = new FuzzyRuleAntecedent();
  ifTemperaturaIntaltaAndTemperaturaExtmediaa ->joinWithAND(alta_ti, mediaa_te);

  // Regla difusa 16
  FuzzyRuleAntecedent *ifTemperaturaIntaltaAndTemperaturaExtalta = new FuzzyRuleAntecedent();
  ifTemperaturaIntaltaAndTemperaturaExtalta ->joinWithAND(alta_ti, alta_te);
  // Regla difusa 17
  FuzzyRuleAntecedent *ifTemperaturaIntmuyaAndTemperaturaExtbaja = new FuzzyRuleAntecedent();
  ifTemperaturaIntmuyaAndTemperaturaExtbaja ->joinWithAND(muya_ti, baja_te);

  // Regla difusa 18
  FuzzyRuleAntecedent *ifTemperaturaIntmuyaAndTemperaturaExtmediab = new FuzzyRuleAntecedent();
  ifTemperaturaIntmuyaAndTemperaturaExtmediab ->joinWithAND(muya_ti, mediab_te);

  // Regla difusa 19
  FuzzyRuleAntecedent *ifTemperaturaIntmuyaAndTemperaturaExtmediaa = new FuzzyRuleAntecedent();
  ifTemperaturaIntmuyaAndTemperaturaExtmediaa ->joinWithAND(muya_ti, mediaa_te);

  // Regla difusa 20
  FuzzyRuleAntecedent *ifTemperaturaIntmuyaAndTemperaturaExtalta = new FuzzyRuleAntecedent();
  ifTemperaturaIntmuyaAndTemperaturaExtalta ->joinWithAND(muya_ti, alta_te);

  // Construyendo consecuencias
 // Metodo que guarda la estrutura de datos de la regla entonces esto en un puntero

  // Consecuencia 1
  FuzzyRuleConsequent *thenvoltajemuyb = new FuzzyRuleConsequent();
  thenvoltajemuyb->addOutput(muyb_v);

  // Consecuencia 2
  FuzzyRuleConsequent *thenvoltajebajo = new FuzzyRuleConsequent();
  thenvoltajebajo->addOutput(bajo_v);

  // Consecuencia 3
  FuzzyRuleConsequent *thenvoltajemedio = new FuzzyRuleConsequent();
  thenvoltajemedio->addOutput(medio_v);

  // Consecuencia 4
  FuzzyRuleConsequent *thenvoltajealto = new FuzzyRuleConsequent();
  thenvoltajealto->addOutput(alto_v);
  
  // Definiendo reglas
 // Se formula la regla completa en una estructura de datos y se agregan al controlador
  // Regla 1
  FuzzyRule *regla1 = new FuzzyRule(1, ifTemperaturaIntmuybAndTemperaturaExtbaja, thenvoltajemuyb);
  fuzzy1->addFuzzyRule(regla1);

  // Regla 2
  FuzzyRule *regla2 = new FuzzyRule(2, ifTemperaturaIntmuybAndTemperaturaExtmediab, thenvoltajemuyb);
  fuzzy1->addFuzzyRule(regla2);

  // Regla 3
  FuzzyRule *regla3 = new FuzzyRule(3, ifTemperaturaIntmuybAndTemperaturaExtmediaa, thenvoltajemuyb);
  fuzzy1->addFuzzyRule(regla3);

  // Regla 4
  FuzzyRule *regla4 = new FuzzyRule(4, ifTemperaturaIntmuybAndTemperaturaExtalta, thenvoltajebajo);
  fuzzy1->addFuzzyRule(regla4);

  // Regla 5
  FuzzyRule *regla5 = new FuzzyRule(5, ifTemperaturaIntbajaAndTemperaturaExtbaja, thenvoltajemuyb);
  fuzzy1->addFuzzyRule(regla5);

  // Regla 6
  FuzzyRule *regla6 = new FuzzyRule(6, ifTemperaturaIntbajaAndTemperaturaExtmediab, thenvoltajemuyb);
  fuzzy1->addFuzzyRule(regla6);

  // Regla 7
  FuzzyRule *regla7 = new FuzzyRule(7, ifTemperaturaIntbajaAndTemperaturaExtmediaa, thenvoltajebajo);
  fuzzy1->addFuzzyRule(regla7);

  // Regla 8
  FuzzyRule *regla8 = new FuzzyRule(8, ifTemperaturaIntbajaAndTemperaturaExtalta, thenvoltajebajo);
  fuzzy1->addFuzzyRule(regla8);

  // Regla 9
  FuzzyRule *regla9 = new FuzzyRule(9, ifTemperaturaIntmediaAndTemperaturaExtbaja, thenvoltajebajo);
  fuzzy1->addFuzzyRule(regla9);

  // Regla 10
  FuzzyRule *regla10 = new FuzzyRule(10, ifTemperaturaIntmediaAndTemperaturaExtmediab, thenvoltajemedio);
  fuzzy1->addFuzzyRule(regla10);

  // Regla 11
  FuzzyRule *regla11 = new FuzzyRule(11, ifTemperaturaIntmediaAndTemperaturaExtmediaa, thenvoltajemedio);
  fuzzy1->addFuzzyRule(regla11);

  // Regla 12
  FuzzyRule *regla12 = new FuzzyRule(12, ifTemperaturaIntmediaAndTemperaturaExtalta, thenvoltajealto);
  fuzzy1->addFuzzyRule(regla12);

  // Regla 13
  FuzzyRule *regla13 = new FuzzyRule(13, ifTemperaturaIntaltaAndTemperaturaExtbaja, thenvoltajemedio);
  fuzzy1->addFuzzyRule(regla13);

  // Regla 14
  FuzzyRule *regla14 = new FuzzyRule(14, ifTemperaturaIntaltaAndTemperaturaExtmediab, thenvoltajealto);
  fuzzy1->addFuzzyRule(regla14);

  // Regla 15
  FuzzyRule *regla15 = new FuzzyRule(15, ifTemperaturaIntaltaAndTemperaturaExtmediaa, thenvoltajealto);
  fuzzy1->addFuzzyRule(regla15);
  
  // Regla 16
  FuzzyRule *regla16 = new FuzzyRule(16, ifTemperaturaIntaltaAndTemperaturaExtalta, thenvoltajealto);
  fuzzy1->addFuzzyRule(regla16);

  // Regla 17
  FuzzyRule *regla17 = new FuzzyRule(17, ifTemperaturaIntmuyaAndTemperaturaExtbaja, thenvoltajealto);
  fuzzy1->addFuzzyRule(regla17);

  // Regla 18
  FuzzyRule *regla18 = new FuzzyRule(18, ifTemperaturaIntmuyaAndTemperaturaExtmediab, thenvoltajealto);
  fuzzy1->addFuzzyRule(regla18);

  // Regla 19
  FuzzyRule *regla19 = new FuzzyRule(19, ifTemperaturaIntmuyaAndTemperaturaExtmediaa, thenvoltajealto);
  fuzzy1->addFuzzyRule(regla19);

  // Regla 20
  FuzzyRule *regla20 = new FuzzyRule(20, ifTemperaturaIntmuyaAndTemperaturaExtalta, thenvoltajealto);
  fuzzy1->addFuzzyRule(regla20);

//########################################################################################################## Controlador 2, ventana
// FuzzyInput
  FuzzyInput *humint = new FuzzyInput(1);

  humint->addFuzzySet(muybi);
  humint->addFuzzySet(bajai);
  humint->addFuzzySet(mediai);
  humint->addFuzzySet(altai);
  humint->addFuzzySet(muyai);
  fuzzy2->addFuzzyInput(humint);

  // FuzzyInput
  FuzzyInput *humext = new FuzzyInput(2);

  humext->addFuzzySet(muybe);
  humext->addFuzzySet(bajae);
  humext->addFuzzySet(mediae);
  humext->addFuzzySet(altae);
  humext->addFuzzySet(muyae);
  fuzzy2->addFuzzyInput(humext);
  
  // FuzzyOutput
  FuzzyOutput *ventana = new FuzzyOutput(1);

  ventana->addFuzzySet(muy_poca);
  ventana->addFuzzySet(poca);
  ventana->addFuzzySet(media);
  ventana->addFuzzySet(alta);
  ventana->addFuzzySet(muy_alta);
  fuzzy2->addFuzzyOutput(ventana);

  // Regla difusa 1
  FuzzyRuleAntecedent *ifhummuybiAndhummuybe = new FuzzyRuleAntecedent();
  ifhummuybiAndhummuybe->joinWithAND(muybi, muybe);

  // Regla difusa 2
  FuzzyRuleAntecedent *ifhummuybiAndhumbajae = new FuzzyRuleAntecedent();
  ifhummuybiAndhumbajae->joinWithAND(muybi, bajae);

  // Regla difusa 3
  FuzzyRuleAntecedent *ifhummuybiAndhummediae = new FuzzyRuleAntecedent();
  ifhummuybiAndhummediae->joinWithAND(muybi, mediae);

  // Regla difusa 4
  FuzzyRuleAntecedent *ifhummuybiAndhumaltae = new FuzzyRuleAntecedent();
  ifhummuybiAndhumaltae->joinWithAND(muybi, altae);

  // Regla difusa 5
  FuzzyRuleAntecedent *ifhummuybiAndhummuyae = new FuzzyRuleAntecedent();
  ifhummuybiAndhummuyae->joinWithAND(muybi, muyae);
  
  // Regla difusa 6
  FuzzyRuleAntecedent *ifhumbajaiAndhummuybe = new FuzzyRuleAntecedent();
  ifhumbajaiAndhummuybe->joinWithAND(bajai, muybe);

  // Regla difusa 7
  FuzzyRuleAntecedent *ifhumbajaiAndhumbajae = new FuzzyRuleAntecedent();
  ifhumbajaiAndhumbajae->joinWithAND(bajai, bajae);

  // Regla difusa 8
  FuzzyRuleAntecedent *ifhumbajaiAndhummediae = new FuzzyRuleAntecedent();
  ifhumbajaiAndhummediae->joinWithAND(bajai, mediae);

  // Regla difusa 9
  FuzzyRuleAntecedent *ifhumbajaiAndhumaltae = new FuzzyRuleAntecedent();
  ifhumbajaiAndhumaltae->joinWithAND(bajai, altae);
  
  // Regla difusa 10
  FuzzyRuleAntecedent *ifhumbajaiAndhummuyae = new FuzzyRuleAntecedent();
  ifhumbajaiAndhummuyae->joinWithAND(bajai, muyae);

  // Regla difusa 11
  FuzzyRuleAntecedent *ifhummediaiAndhummuybe = new FuzzyRuleAntecedent();
  ifhummediaiAndhummuybe->joinWithAND(mediai, muybe);

  // Regla difusa 12
  FuzzyRuleAntecedent *ifhummediaiAndhumbajae = new FuzzyRuleAntecedent();
  ifhummediaiAndhumbajae->joinWithAND(mediai, bajae);

   // Regla difusa 13
  FuzzyRuleAntecedent *ifhummediaiAndhummediae = new FuzzyRuleAntecedent();
  ifhummediaiAndhummediae->joinWithAND(mediai, mediae);

  // Regla difusa 14
  FuzzyRuleAntecedent *ifhummediaiAndhumaltae = new FuzzyRuleAntecedent();
  ifhummediaiAndhumaltae->joinWithAND(mediai, altae);

  // Regla difusa 15
  FuzzyRuleAntecedent *ifhummediaiAndhummuyae = new FuzzyRuleAntecedent();
  ifhummediaiAndhummuyae->joinWithAND(mediai, muyae);

  // Regla difusa 16
  FuzzyRuleAntecedent *ifhumaltaiAndhummuybe = new FuzzyRuleAntecedent();
  ifhumaltaiAndhummuybe->joinWithAND(altai, muybe);

  // Regla difusa 17
  FuzzyRuleAntecedent *ifhumaltaiAndhumbajae = new FuzzyRuleAntecedent();
  ifhumaltaiAndhumbajae->joinWithAND(altai, bajae);

   // Regla difusa 18
  FuzzyRuleAntecedent *ifhumaltaiAndhummediae = new FuzzyRuleAntecedent();
  ifhumaltaiAndhummediae->joinWithAND(altai, mediae);

  // Regla difusa 19
  FuzzyRuleAntecedent *ifhumaltaiAndhumaltae = new FuzzyRuleAntecedent();
  ifhumaltaiAndhumaltae->joinWithAND(altai, altae);

  // Regla difusa 20
  FuzzyRuleAntecedent *ifhumaltaiAndhummuyae = new FuzzyRuleAntecedent();
  ifhumaltaiAndhummuyae->joinWithAND(altai, muyae);

  // Regla difusa 21
  FuzzyRuleAntecedent *ifhummuyaiAndhummuybe = new FuzzyRuleAntecedent();
  ifhummuyaiAndhummuybe->joinWithAND(muyai, muybe);

  // Regla difusa 22
  FuzzyRuleAntecedent *ifhummuyaiAndhumbajae = new FuzzyRuleAntecedent();
  ifhummuyaiAndhumbajae->joinWithAND(muyai, bajae);

   // Regla difusa 23
  FuzzyRuleAntecedent *ifhummuyaiAndhummediae = new FuzzyRuleAntecedent();
  ifhummuyaiAndhummediae->joinWithAND(muyai, mediae);

  // Regla difusa 24
  FuzzyRuleAntecedent *ifhummuyaiAndhumaltae = new FuzzyRuleAntecedent();
  ifhummuyaiAndhumaltae->joinWithAND(muyai, altae);

  // Regla difusa 25
  FuzzyRuleAntecedent *ifhummuyaiAndhummuyae = new FuzzyRuleAntecedent();
  ifhummuyaiAndhummuyae->joinWithAND(muyai, muyae);
  

  // Reglas consecuentes de salida
  FuzzyRuleConsequent *thenventanamuy_poca = new FuzzyRuleConsequent();
  thenventanamuy_poca->addOutput(muy_poca);

  FuzzyRuleConsequent *thenventanapoca = new FuzzyRuleConsequent();
  thenventanapoca->addOutput(poca);

  FuzzyRuleConsequent *thenventanamedia = new FuzzyRuleConsequent();
  thenventanamedia->addOutput(media);

  FuzzyRuleConsequent *thenventanaalta = new FuzzyRuleConsequent();
  thenventanaalta->addOutput(alta);

  FuzzyRuleConsequent *thenventanamuy_alta = new FuzzyRuleConsequent();
  thenventanamuy_alta->addOutput(muy_alta);
  
  // Definiendo las reglas difusas propias del sistema

  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, ifhummuybiAndhummuybe, thenventanamuy_poca);
  fuzzy2->addFuzzyRule(fuzzyRule1);

  FuzzyRule *fuzzyRule2 = new FuzzyRule(2, ifhummuybiAndhumbajae, thenventanamuy_poca);
  fuzzy2->addFuzzyRule(fuzzyRule2);

  FuzzyRule *fuzzyRule3 = new FuzzyRule(3, ifhummuybiAndhummediae,thenventanamuy_poca);
  fuzzy2->addFuzzyRule(fuzzyRule3);

  FuzzyRule *fuzzyRule4 = new FuzzyRule(4, ifhummuybiAndhumaltae, thenventanamuy_poca);
  fuzzy2->addFuzzyRule(fuzzyRule4);

  FuzzyRule *fuzzyRule5 = new FuzzyRule(5, ifhummuybiAndhummuyae, thenventanamuy_poca);
  fuzzy2->addFuzzyRule(fuzzyRule5);

  FuzzyRule *fuzzyRule6 = new FuzzyRule(6, ifhumbajaiAndhummuybe, thenventanapoca);
  fuzzy2->addFuzzyRule(fuzzyRule6);

  FuzzyRule *fuzzyRule7 = new FuzzyRule(7, ifhumbajaiAndhumbajae,thenventanamedia);
  fuzzy2->addFuzzyRule(fuzzyRule7);

  FuzzyRule *fuzzyRule8 = new FuzzyRule(8, ifhumbajaiAndhummediae, thenventanapoca);
  fuzzy2->addFuzzyRule(fuzzyRule8);

  FuzzyRule *fuzzyRule9 = new FuzzyRule(9, ifhumbajaiAndhumaltae, thenventanamuy_poca);
  fuzzy2->addFuzzyRule(fuzzyRule9);

  FuzzyRule *fuzzyRule10 = new FuzzyRule(10, ifhumbajaiAndhummuyae,thenventanamuy_poca);
  fuzzy2->addFuzzyRule(fuzzyRule10);

  FuzzyRule *fuzzyRule11 = new FuzzyRule(11, ifhummediaiAndhummuybe, thenventanamuy_alta);
  fuzzy2->addFuzzyRule(fuzzyRule11);

  FuzzyRule *fuzzyRule12 = new FuzzyRule(12, ifhummediaiAndhumbajae, thenventanaalta);
  fuzzy2->addFuzzyRule(fuzzyRule12);

  FuzzyRule *fuzzyRule13 = new FuzzyRule(13, ifhummediaiAndhummediae, thenventanamedia);
  fuzzy2->addFuzzyRule(fuzzyRule13);

  FuzzyRule *fuzzyRule14 = new FuzzyRule(14, ifhummediaiAndhumaltae, thenventanapoca);
  fuzzy2->addFuzzyRule(fuzzyRule14);

  FuzzyRule *fuzzyRule15 = new FuzzyRule(15, ifhummediaiAndhummuyae, thenventanamuy_poca);
  fuzzy2->addFuzzyRule(fuzzyRule15);

  FuzzyRule *fuzzyRule16 = new FuzzyRule(16, ifhumaltaiAndhummuybe, thenventanamuy_alta);
  fuzzy2->addFuzzyRule(fuzzyRule16);

  FuzzyRule *fuzzyRule17 = new FuzzyRule(17, ifhumaltaiAndhumbajae, thenventanamuy_alta);
  fuzzy2->addFuzzyRule(fuzzyRule17);

  FuzzyRule *fuzzyRule18 = new FuzzyRule(18, ifhumaltaiAndhummediae, thenventanaalta);
  fuzzy2->addFuzzyRule(fuzzyRule18);

  FuzzyRule *fuzzyRule19 = new FuzzyRule(19, ifhumaltaiAndhumaltae, thenventanapoca);
  fuzzy2->addFuzzyRule(fuzzyRule19);

  FuzzyRule *fuzzyRule20 = new FuzzyRule(20, ifhumaltaiAndhummuyae, thenventanamuy_poca);
  fuzzy2->addFuzzyRule(fuzzyRule20);

  FuzzyRule *fuzzyRule21 = new FuzzyRule(21, ifhummuyaiAndhummuybe, thenventanamuy_alta);
  fuzzy2->addFuzzyRule(fuzzyRule21);

  FuzzyRule *fuzzyRule22 = new FuzzyRule(22, ifhummuyaiAndhumbajae, thenventanamuy_alta);
  fuzzy2->addFuzzyRule(fuzzyRule22);

  FuzzyRule *fuzzyRule23 = new FuzzyRule(23, ifhummuyaiAndhummediae, thenventanamuy_alta);
  fuzzy2->addFuzzyRule(fuzzyRule23);

  FuzzyRule *fuzzyRule24 = new FuzzyRule(24, ifhummuyaiAndhumaltae, thenventanapoca);
  fuzzy2->addFuzzyRule(fuzzyRule24);

  FuzzyRule *fuzzyRule25 = new FuzzyRule(25, ifhummuyaiAndhummuyae, thenventanamuy_poca);
  fuzzy2->addFuzzyRule(fuzzyRule25);


}


void loop() {
  
  checkForNewData(); //Llama a la funcion que nos permite comunicar python con Arduino
    if (newData == true) {
        newData = false;
    }

  toPy(myArray[0],myArray[1],myArray[2],myArray[3],myArray[4],myArray[5],myArray[6],myArray[7],myArray[8],
    myArray[9],myArray[10],myArray[11],myArray[12],myArray[13],myArray[14],0,0,0,random(100)); //here write the send data 

  // Leemos la humedad relativa
  float hi = dht.readHumidity();
  // Leemos la temperatura en grados centígrados (por defecto)
  float t = dht.readTemperature();
    
  // Comprobamos si ha habido algún error en la lectura
  if (isnan(hi) || isnan(t)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
  }

  float temperaturaExt = myArray[0];//Asigna el valor temperatura Exterior al arreglo 0, el valor en donde está la temperatuira descargada de internet

  fuzzy1->setInput(1, t); //le asignas al controlador la entrada
  fuzzy1->setInput(2, temperaturaExt);

  fuzzy1->fuzzify(); //fuzzificar
  float voltaje = fuzzy1->defuzzify(1); //Defuzzificar la salida
  
  int PWM = (voltaje*255)/5;
  analogWrite(motor, PWM);
  digitalWrite(motorInit, LOW);

/////Fuzzy 2
  float he = myArray[2];
  fuzzy2->setInput(1, hi); //le asignas al controlador la entrada
  fuzzy2->setInput(2, he);
  fuzzy2->fuzzify(); //fuzzificar
  float grados = fuzzy2->defuzzify(1); //Defuzzificar la salida

  lcd.setCursor(0, 0);
  lcd.print(t, 0);

  lcd.setCursor(4, 0);
  lcd.print(temperaturaExt, 0);

  lcd.setCursor(8, 0);
  lcd.print(hi, 0);

  lcd.setCursor(12, 0);
  lcd.print(he, 0);

  lcd.setCursor(0, 1);
  lcd.print(PWM);

  lcd.setCursor(4, 1);
  lcd.print(grados, 0);

  lcd.setCursor(7, 1);
  lcd.print(LecturaLDR, 1);

  servoMotor.write(grados);
  LecturaLDR = analogRead(0);
  val = digitalRead(sensorPin); //Lee el valor de la variable (PIR)
  if (LecturaLDR <= 300){
    if (val == HIGH) //Si detecta que hay movimiento manda activar el led que hay conectado en el pin 12 del arduino
    {
    digitalWrite(ledPin, HIGH);
    }
    else //Si la condición anterior no se cumple manda apagar el led
    {
    digitalWrite(ledPin, LOW);
    }
  }
  else
  digitalWrite(ledPin, LOW);
  
  delay(1000);



}

void checkForNewData () {             
    if (Serial.available() >= pcDataLen && newData == false) {
        byte inByte;
        for (byte n = 0; n < pcDataLen; n++) {
            ddata [n] = Serial.read();
        }
        while (Serial.available() > 0) { // now make sure there is no other data in the buffer
             byte dumpByte =  Serial.read();
             Serial.println(dumpByte);
        }
        newData = true;
    }
}


void toPy(int a,int b,int c,int d,int e,int f,
          int g,int h,int i,int j,int k,int l,
          int m,int n,int o,int p,int q,int r,int s)//19 datas
{
  //rpidata="[1,2,3,4,5,6,7,8,9,10,111,0]";
String data="["+String(a)+","+String(b)+","+String(c)+","+String(d)+","+String(e)+","+String(f)+","+String(g)+","+String(h)+","+String(i)+","+
String(j)+","+String(k)+","+String(l)+","+String(m)+","+String(n)+","+String(o)+","+String(p)+","+String(q)+","+String(r)+","+String(s)+"]";
  delay(2000);Serial.println(data);
}
