#include <Servo.h>
#include <LiquidCrystal.h>

// Dichiarazione degli oggetti
Servo myservo;                            // Oggetto per il controllo del servo motore
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);     // Oggetto per il controllo dello schermo LCD 16x2

// Dichiarazione dei pin di Arduino
const int buttonPin = 6;                   // Pin del pulsante
const int servoPin = 9;                    // Pin del servo motore
const int relayPin = 10;                   // Pin del relè
const int potPin = A0;                     // Pin del potenziometro

// Dichiarazione delle variabili
bool isRelayActive = false;                // Flag per indicare se il relè è attivo
bool isButtonPressed = false;              // Flag per indicare se il pulsante è stato premuto
unsigned long relayStartTime = 0;          // Tempo di inizio dell'avvio del relè
int menuOption = 0;                        // Opzione del menu selezionata
int prevMenuOption = 0;                    // Opzione del menu precedente
int potValue = 0;                          // Valore del potenziometro
unsigned long timerDuration = 0;           // Durata del timer in millisecondi
unsigned long timerStartTime = 0;          // Tempo di inizio del timer

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);

  myservo.attach(servoPin);                // Collegamento dell'oggetto servo al pin del servo motore

  lcd.begin(16, 2);                        // Inizializzazione dello schermo LCD
  lcd.print("PET MAKER");
  lcd.setCursor(0, 1);
  lcd.print("in avvio...");
  
  delay(5000);
  displayMenu();                           // Visualizza il menu di default
}

void loop() {
  // Leggi il valore del potenziometro
  potValue = analogRead(potPin);

  // Determina l'opzione del menu in base al valore del potenziometro
  menuOption = map(potValue, 0, 1023, 0, 2);

  // Verifica se l'opzione del menu è cambiata
  if (menuOption != prevMenuOption) {
    prevMenuOption = menuOption;
    displayMenu();
  }

  // Verifica se il pulsante è stato premuto
  if (digitalRead(buttonPin) == LOW && !isButtonPressed) {
    isButtonPressed = true;

    // Esegui l'azione corrispondente all'opzione del menu
    switch (menuOption) {
      case 0:
        toggleRelay();
        break;
      case 1:
        setTimer();
        break;
      case 2:
        // Aggiungi qui l'azione desiderata per l'opzione 2
        break;
    }
  }

  // Verifica se il pulsante è stato rilasciato
  if (digitalRead(buttonPin) == HIGH && isButtonPressed) {
    isButtonPressed = false;
  }

  // Calcola il tempo trascorso dall'avvio del relè
  if (isRelayActive) {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - relayStartTime;

    unsigned long seconds = (elapsedTime / 1000) % 60;
    unsigned long minutes = (elapsedTime / 60000) % 60;
    unsigned long hours = (elapsedTime / 3600000) % 24;

    lcd.setCursor(0, 1);
    lcd.print("Tempo: ");
    if (hours < 10) {
      lcd.print("0");
    }
    lcd.print(hours);
    lcd.print(":");
    if (minutes < 10) {
      lcd.print("0");
    }
    lcd.print(minutes);
    lcd.print(":");
    if (seconds < 10) {
      lcd.print("0");
    }
    lcd.print(seconds);

    // Verifica se il timer è scaduto
    if (timerDuration > 0 && elapsedTime >= timerDuration) {
      toggleRelay();  // Spegni il relè
    }
  }
}

// Funzione per attivare/disattivare il relè
void toggleRelay() {
  if (!isRelayActive) {
    lcd.clear();
    lcd.print("Stato: [ON]");
    lcd.setCursor(0, 1);
    lcd.print("PET MAKER");
    digitalWrite(relayPin, HIGH);        // Attiva il relè
    isRelayActive = true;
    relayStartTime = millis();            // Registra il tempo di inizio dell'avvio del relè
  } else {
    digitalWrite(relayPin, LOW);         // Disattiva il relè
    isRelayActive = false;
    lcd.clear();
    lcd.print("Processo");
    lcd.setCursor(0,1);
    lcd.print("Terminato");
    delay(3000);
    displayMenu();  // Visualizza nuovamente il menu
  }
}

// Funzione per impostare il timer
void setTimer() {
  lcd.clear();
  lcd.print("Timer: ");
  lcd.setCursor(0, 1);
  lcd.print("Rot. pot.");
  
  while (digitalRead(buttonPin) == HIGH) {
    // Leggi il valore del potenziometro per impostare la durata del timer
    potValue = analogRead(potPin);
    timerDuration = map(potValue, 0, 1023, 0, 3600000); // Mappa il valore del potenziometro da 0 a 1023 a una durata di 0 a 1 ora
    
    // Aggiorna il display con la durata del timer impostata
    unsigned long minutes = (timerDuration / 60000) % 60;
    unsigned long seconds = (timerDuration / 1000) % 60;

    lcd.setCursor(0, 1);
    lcd.print("Durata: ");
    lcd.print(minutes);
    lcd.print("m ");
    lcd.print(seconds);
    lcd.print("s ");
  }

  // Avvia il timer
  timerStartTime = millis();
  lcd.clear();
  lcd.print("Timer avviato");
  lcd.setCursor(0,1);
  lcd.print("Tempo: 10s");

  digitalWrite(relayPin, HIGH);  // Attiva il relè
  delay(10000);
  digitalWrite(relayPin, LOW);  // Disattiva il relè

    lcd.clear();
    lcd.print("Processo");
    lcd.setCursor(0,1);
    lcd.print("Terminato");
    delay(3000);
    displayMenu();  // Visualizza nuovamente il menu
}

// Funzione per visualizzare il menu
void displayMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Menu Principale:");

  switch (menuOption) {
    case 0:
      lcd.setCursor(0, 1);
      lcd.print("> ON/OFF");
      break;
    case 1:
      lcd.setCursor(0, 1);
      lcd.print("> Timer 10s");
      break;
    case 2:
      lcd.setCursor(0, 1);
      lcd.print("> Opzione 2");
      break;
  }
}
