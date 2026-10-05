#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

//DECLARARE PINI
const int senzorSol = 34;
const int senzorTemperatura = 18;
const int senzorReleu = 26;

//DECLARARE SCREEN
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

//VARIABILE SCHIMBAREA ECRANELOR
int ecranCurent = 0;
unsigned long ultimulTimpSchimbare = 0;
const unsigned long intervalSchimbare = 6000; // 6000ms = 6 SECUNDE

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//DECLARARE SENZOR TEMPERATURA SI UMIDITATE
#define DHTYPE DHT11
DHT dht(senzorTemperatura, DHTYPE);
float temperatura = 0;
float umiditateAer = 0;

//VARIABILE SOL
int valoareUdSol = 1800;
int valoareUscatSol = 3400;

//VARIABILE PENTRU UDARE
int pragUdare = 30;
const unsigned long durataUdare = 3000;
const unsigned long pauzaUdare = 60000;
bool udareActiva = false;
unsigned long startUdare = 0;
unsigned long ultimaUdare = 0;

void setup(){
    Serial.begin(115200);

    analogReadResolution(12); //SETEZ CITIRE ANALOGICA INTRE 0 SI 4095 (CITIRE ANALOGICA PE 12 BITI)

    pinMode(senzorReleu, OUTPUT); // FACE PINUL 26, PIN DE IESIRE
    digitalWrite(senzorReleu, HIGH ); // FACE PINUL 26 1 LOGIC, RELEU NU CONDUCE
    
    dht.begin(); // INITIALIZEZ SENZORUL DE TEMPERATURA SI UMIDITATE
    
    Wire.begin(21,22); // PORNESTE COMUNICATIA I2C PENTRU OLED

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED-ul nu a fost gasit!");
    while (true) {
        delay(10);
    }
}

    display.clearDisplay(); //STERCE CE ERA PE ECRAN
    display.display(); 

    ultimulTimpSchimbare = millis();
    
}

//FUNCTIA CARE CITETE SENZORUL DE SOL
int citireMedieSol(int pin){
    int n = 10; // CITESTE 10 VALORI DE PE SENZOR
    long suma = 0;

    for(int i = 0; i<n; i++){
        suma += analogRead(pin);
        delay(5);
    }
    return suma/n;
}

//FUNCTIE CARE TANSFORMA VALORILE DE SOL IN PROCENTE
int calculareProcentSol(int valoriPrimite){
    int procent = map(valoriPrimite, valoareUscatSol, valoareUdSol, 0, 100);

   if (procent < 0) procent = 0;
   if (procent > 100) procent = 100;

    return procent;
}

//FUNCTIE CARE DESCHIDE RELEUL (PORNESTE RELEUL)
void pornesteUdare(){
    udareActiva = true;
    startUdare = millis();
    ultimaUdare =  millis();

    digitalWrite(senzorReleu, LOW);
}

//FUNCTIA CARE VEDE DACA TREBUIE PORNITA UDAREA
void verificareUdare(int procentSol){
    if(udareActiva == false){
        if (procentSol < pragUdare && millis() - ultimaUdare >= pauzaUdare) {
      pornesteUdare();
      Serial.print("UDAREA A INCEPUT;");
    }
     } else {
    if (millis() - startUdare >= durataUdare) {
      udareActiva = false;
      digitalWrite(senzorReleu, HIGH);  // OFF
      Serial.print("UDAREA S-A OPRIT;");
    }
  }
}

// FUNCTIE CITIRE TEMPERATURA SI UMIDITATE
void citesteDHT11(){
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if(!isnan(h)){
        umiditateAer = h;
    }
    if(!isnan(t)){
        temperatura = t;
    }
}

//FUNCTII AFISARE ECRANE

void deseneazaIndicatorEcran() {
  int y = 58;          // pozitia pe verticala, jos
  int raza = 2;        // marimea bulinei
  int spatiu = 14;     // distanta intre buline
  int centruX = 64;    // mijlocul ecranului OLED (128/2)

  // pozitia primei buline astfel incat toate 3 sa fie centrate
  int startX = centruX - spatiu;

  for (int i = 0; i < 3; i++) {
    int x = startX + i * spatiu;

    if (i == ecranCurent) {
      display.fillCircle(x, y, raza, SSD1306_WHITE);   // bulina plina
    } else {
      display.drawCircle(x, y, raza, SSD1306_WHITE);   // bulina goala
    }
  }
}

void deseneazaBaraOrizontala(int x, int y, int w, int h, int procent) {
  display.drawRect(x, y, w, h, SSD1306_WHITE);
  int umplere = map(procent, 0, 100, 0, w - 2);
  display.fillRect(x + 1, y + 1, umplere, h - 2, SSD1306_WHITE);
}

void afiseazaEcran1(int proccentSol){
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(25, 4);
  display.println("STATUS PLANTA");
  display.drawLine(8, 16, 119, 16, SSD1306_WHITE);

display.setTextSize(1);
display.setCursor(14, 20);
display.print("SOL: ");
display.print(proccentSol);
display.println("%");

   deseneazaBaraOrizontala(14, 33, 100, 8, proccentSol);

  display.setTextSize(1);
  if (proccentSol < 30) {
    display.setCursor(22, 45);
    display.println("STARE: USCATA");
  } else if (proccentSol < 60) {
    display.setCursor(34, 45);
    display.println("STARE: OK");
  } else {
    display.setCursor(31, 45);
    display.println("STARE: UDA");
  }
  
  deseneazaIndicatorEcran();
  display.display();
}
void afiseazaEcran2(){
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(25, 4);
  display.println("STATUS CAMERA");
  display.drawLine(8, 16, 119, 16, SSD1306_WHITE);

  //TEMPERATURA AFISARE
  display.setTextSize(1);
  display.setCursor(10, 24);
  display.print("TEMP: ");
  display.print(temperatura, 1);
  display.println(" C");

   //UMIDITATE
  display.setCursor(10, 40);
  display.print("UMID: ");
  display.print(umiditateAer, 1);
  display.println(" %");

  deseneazaIndicatorEcran();
  display.display();
}
void afiseazaEcran3(){
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(34, 4);
  display.println("INFORMATII");
  display.drawLine(8, 16, 119, 16, SSD1306_WHITE);

  deseneazaIndicatorEcran();
  display.display();
}


void loop(){
    int valoriSol = citireMedieSol(senzorSol);
    int proccentSol = calculareProcentSol(valoriSol);

    verificareUdare(proccentSol);


    citesteDHT11();

    Serial.print("Valori sol: ");
    Serial.print(valoriSol);
    Serial.print("Procent Sol: ");
    Serial.print(proccentSol);
    Serial.print(" %");
    Serial.print(" Temperatura: ");
    Serial.print(temperatura);
    Serial.print(" Umiditate Aer: ");
    Serial.println(umiditateAer);


    if(millis()-ultimulTimpSchimbare >= intervalSchimbare){
        ultimulTimpSchimbare = millis();

        ecranCurent++;
        if(ecranCurent>2){
            ecranCurent = 0;
        }
        if(ecranCurent==0){
            afiseazaEcran1(proccentSol);
        }
        if(ecranCurent==1){
            afiseazaEcran2();
        }
        if(ecranCurent==2){
           afiseazaEcran3();
        }
    }



    delay(300);
}

