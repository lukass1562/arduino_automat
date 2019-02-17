#include <ArduinoJson.h>

#include <LiquidCrystal_I2C.h>  //biblioteka do obsługi wyswietlacza LCD
#include <Keypad.h>             //biblioteka do obslugi klawiatury
#include <Wire.h>  

 //serwa
#define SERW1 9
#define SERW2 10
#define SERW3 11
#define SERW4 12
#define CZUJ 13

//wyswietlacz
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

//klawiatura
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3' },
  {'4','5','6' },
  {'7','8','9' },
  {'*','0','#' }
};
byte rowPins[ROWS] = {24, 26, 28, 30};  //Piny, do których podłączamy wyprowadzenia od rzędów
byte colPins[COLS] = {32, 34, 36 }; //Piny, do których kolumn wyprowadzenia od rzędów
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//deklaracja zmiennych
char produkt[]={'0','0'};
bool klawisz=0;
char cyfra;
char id_uzytk[4];
char pin[4];
boolean flag;
int stat;
int nr_automat=1;
char server_pin[4];

//deklaracja funkcji//
void wybprodukt();
int nrprodukt();
void Pin();
void awaria();
void wydprodukt();
void uzytkownik ();
void transakcja();
void serwo();

//odmierzanie czasu serwo
unsigned long akt_czas=0;
unsigned long stary_czas=0;

//zmienne do timera odliczajaego czas //
bool przerwanie;
unsigned long aktualnyczas=0;
unsigned long suma=0;
unsigned long staryczas=0;

void setup() {
 Serial.begin(9600);
 lcd.begin(20,4);   // Inicjalizacja LCD 4x20
 lcd.clear();       //czyszczenie LCD
 lcd.backlight(); // zalaczenie podwietlenia LCD

 pinMode(SERW1,OUTPUT);
 pinMode(SERW2,OUTPUT);
 pinMode(SERW3,OUTPUT);
 pinMode(SERW4,OUTPUT);
 pinMode(CZUJ,INPUT);
}
void loop() {
   przerwanie=false;
   wybprodukt();//LCD "wybierz produkt"
   nrprodukt();//LCD "podaj nr produktu"
   if(!przerwanie){//jesli czas minie, powroc do okna "wybierz produkt"
    uzytkownik ();   //LCD "podaj id uzytkownika"
   if(!przerwanie){//jesli czas minie ,powroc do okna "wybierz produkt"
    send1();     //wysyłanie danych ID_uzytkownika,numer ptoduktu, numer automatu
   int count=0; 
    while(count<=3){
    if(Serial.available()){
     delay(10);
      server_pin[count] = Serial.read();
      count++;
      }
    }
    Pin();       //wprowadzanie pinu uzytkownika
    if(!przerwanie){
     control();   //sprawdzanie czy pin jest poprawny
     transakcja(); //wyswietlanie napisu "tranzakcja zaakceptowana" lub "tranzakcja anulowana"
    }else{Serial.println("anulowana");}
   }  
  }
}
 void wybprodukt()
{
  lcd.clear();
  do{
  lcd.setCursor(0,1); // Ustawienie kursora w pozycji 0,0 (pierwszy wiersz, pierwsza kolumna)
  lcd.print("Wybierz produkt");
  cyfra=keypad.getKey(); //czytanie cyfry z klawiatury
  produkt[0]='0';
  produkt[1]='0'; ////usuwanie zawartosci nuemru poduktu
  }while(!cyfra);
} 
int nrprodukt()
{
 staryczas=millis();
 int j=0;//zmienna do obslugi numeru produktu
  while(cyfra!='*')
  {
  if(cyfra) //jezeli wcisnieto klawisz
    {
      j=j+1;  //zmienna do zliczania ilosc wcisnietych klawiszy
      if(j==1) //jezeli klawiatura była wcisniety pierwszy raz
       {
         produkt[1]=cyfra;
         staryczas=millis();//tablica do przechowywania numeru produktu max 2 cyfry
       }else if(j==2)       //jeżeli zostały wcisniety drugi klawisz
        {
          produkt[0]=produkt[1];  //zapisywanie dwoch cyfr do tanlicy
          produkt[1]=cyfra;
          staryczas=millis();      
        }else if(j>2)          //jezlei zostałowcisniete wiecej niz dwa klawisze
        {
        produkt[0]='0';       //zerowanie tablicy produktu
        produkt[1]='0';       
        j=0;//przyjmoiwanie cyfr rozpoczyna sie od poczatku
        staryczas=millis();
        }
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Wybrano produkt");
  for(int i=0;i<2;i++)
   {
    lcd.setCursor(8+i,1);
    lcd.print(produkt[i]);
    delay(10);
    }
  lcd.setCursor(1,3);
  lcd.print("* - zatwierdz");
   }else
   {
   aktualnyczas=millis();
   if((aktualnyczas-staryczas)>=8000){
    przerwanie=true;
    break;
    }
   }
   cyfra=keypad.getKey(); 
  }
}
void Pin()
{
  staryczas=millis();
  int i=0;
  int correct;
  lcd.clear();
  lcd.setCursor(3,1);
  lcd.print("Wpisz PIN:");
   for(i=0;i<4;)
    {  
      cyfra=keypad.getKey(); //czytanie cyfry z klawiatury
      if(cyfra)
      {
        lcd.setCursor(i+4,3);
        lcd.print(cyfra);
        pin[i]=int(cyfra);
        delay(500);
        lcd.setCursor(i+4,3);
        lcd.print('*');
        staryczas=millis();
        i++;   
      }
      aktualnyczas=millis();
      if((aktualnyczas-staryczas)>=9000){
       przerwanie=true;
       break;
      }
     }
 }
 void transakcja(){
     if(stat==0) {
        lcd.clear();
        lcd.setCursor(4,1);
        lcd.print("Transakcja");
        lcd.setCursor(5,2);
        lcd.print("anulowana");
        Serial.println("anulowana");
        delay(3000);
     
      }else if(stat==1){
        lcd.clear();
        lcd.setCursor(3,1);
        lcd.print("Transakcja");
        lcd.setCursor(2,2);
        lcd.print("zakceptowana");
        delay(3000);
        wydprodukt();
        }
  }
 void wydprodukt()
 {
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Wydawanie produktu");
  for(int j=5;j<15;j++){
  lcd.setCursor(j,2);
  lcd.print("*");
  delay(1000);
  }
  Serial.println("zakonczona"); ///transakcja zakonczona
  lcd.clear(); 
 }
void awaria()
{
  lcd.clear();
  lcd.setCursor(3,1);
  lcd.print("ERROR!@#");
  delay(5000);
  }


void empty ()
{
  lcd.clear();
  lcd.setCursor(3,1);
  lcd.print("Brak produktu");
  delay(4000);
  }
void uzytkownik ()
{ 
  lcd.clear();
  lcd.setCursor(0,1); // Ustawienie kursora w pozycji 0,0 (pierwszy wiersz, pierwsza kolumna)
  lcd.print("Podaj swoje ID:");
   for(int i=0;i<4;)
    {  
      cyfra=keypad.getKey(); //czytanie cyfry z klawiatury
      if(cyfra)
      {
        lcd.setCursor(i+4,2);
        lcd.cursor();
        lcd.print(cyfra);
        id_uzytk[i]=cyfra;
        staryczas=millis();
        i++;    
      }
      aktualnyczas=millis();
      if((aktualnyczas-staryczas)>=9000)
      { przerwanie=true;
        break;
      }
     }
     lcd.noCursor();
     delay(100);
  }
 void serwo(char a)
{
  switch(a){
      case '1':
      stary_czas=millis();
      digitalWrite(SERW1,HIGH);
      while(digitalRead(CZUJ)==LOW)
      {
        akt_czas=millis();
       if(akt_czas-stary_czas>=5000){
        break;
       }
      } 
      digitalWrite(SERW1,LOW);
    break;
      case '2':
      stary_czas=millis();
      digitalWrite(SERW2,HIGH);
      while(digitalRead(CZUJ)==LOW)
      {
        akt_czas=millis();
       if(akt_czas-stary_czas>=5000){
        break;
       }
      } 
      digitalWrite(SERW2,LOW);
    break;
      case '3':
      stary_czas=millis();
      digitalWrite(SERW3,HIGH);
      while(digitalRead(CZUJ)==LOW)
      {
        akt_czas=millis();
       if(akt_czas-stary_czas>=5000){
        break;
       }
      } 
      digitalWrite(SERW3,LOW);
    break;
      case '4':
      stary_czas=millis();
      digitalWrite(SERW4,HIGH);
      while(digitalRead(CZUJ)==LOW)
      {
        akt_czas=millis();
       if(akt_czas-stary_czas>=5000){
        break;
       }
      } 
      digitalWrite(SERW4,LOW);
    break;
    }
  }
void send1()
{
  //wysyłanie numeru automatu
  Serial.println(nr_automat);
  //wysyłannie numeru pin do serwera
  for (int i=0;i<=1;i++)
  {
   Serial.println(produkt[i]);
   }
  //wysyłanie id_uzytkownika do automatu 
  for (int j=0;j<=3;j++)
  {
   Serial.println(id_uzytk[j]);
   }  
}
void control()
{
     int licznik=0;
     for(int k=0;k<=3;k++)
      {
        if(pin[k]==server_pin[k])
        {
          licznik++;
          }  
      }
      if(licznik!=4){stat=0;}
      else stat=1; 
}
   





 



