#include <VirtualWire.h>

// definiujemy piny
#define led_pin 13
#define transmit_pin 10
int sec = 1;
int pSilnik;
int lSilnik;
int kpSilnika = 0;
int klSilnika = 0;
int pierscien = 0;
int wlacz = 3;
int wylacz = 2;
void setup()
{
  // przygotowujemy potrzebne informacje dla biblioteki
  vw_set_tx_pin(transmit_pin);
  vw_setup(2000);
  pinMode(led_pin, OUTPUT);
  pinMode(wlacz, INPUT_PULLUP);
  pinMode(wylacz, INPUT_PULLUP);
}

void loop()
{
  //********PRAWY************
  if ( analogRead(A0) >= 520) {
    pSilnik = map(analogRead(A0), 520, 1023, 0, 255);
    kpSilnika = 2;
  } else if (analogRead(A0) <= 510) {
    pSilnik = map(analogRead(A0), 510, 0, 0, 255);
    kpSilnika = 1;
  } else {
    pSilnik = 0;
    kpSilnika = 0;

  }
  //********LEWY************
  if ( analogRead(A1) >= 530) {
    lSilnik = map(analogRead(A1), 530, 1023, 0, 255);
    klSilnika = 1;
  } else if (analogRead(A1) <= 500) {
    lSilnik = map(analogRead(A1), 500, 0, 0, 255);
    klSilnika = 2;
  } else {
    lSilnik = 0;
    klSilnika = 0;
  }
    //*************wirnik************
    if (digitalRead(wlacz)==0){
      pierscien=1;
    }
    if(digitalRead(wylacz)==0){
      pierscien=0;
  }
  String toSend = (String(klSilnika) + ";" + String(lSilnik) + ";" + String(kpSilnika) + ";" + String(pSilnik) + ";" + String(pierscien)); // tekst wiadomości
  char msg[50]; // tworzymy tablicę typu char
  toSend.toCharArray(msg, toSend.length() + 1); // konwertujemy nasz tekst do tablicy charów
  Serial.println(toSend);
  digitalWrite(led_pin, HIGH); // zapalamy LED
  vw_send((uint8_t *)msg, strlen(msg));// wysyłamy
  vw_wait_tx();
  digitalWrite(led_pin, LOW); // gasimy LED
  sec++;
  delay(10); // czekamy 1 sekundę

}

