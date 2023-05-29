#include <LiquidCrystal.h>
#include <AccelStepper.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define STEP_PIN_X 2
#define DIR_PIN_X 3
#define STEP_PIN_E 4
#define DIR_PIN_E 5
#define BUTTON_PIN 6

AccelStepper stepperX(AccelStepper::DRIVER, STEP_PIN_X, DIR_PIN_X);
AccelStepper stepperE(AccelStepper::DRIVER, STEP_PIN_E, DIR_PIN_E);
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

int buttonState = HIGH;
int lastButtonState = HIGH;
bool recyclingStarted = false;
bool heatingInProgress = false;
float targetTemperature = 35.0; // Temperatura target desiderata
float currentTemperature = 0.0; // Temperatura corrente dell'estrusore
bool temperatureDetected = false; // Flag per indicare se il sensore di temperatura è stato rilevato
bool lcdNeedsUpdate = false; // Flag per indicare se l'LCD ha bisogno di essere aggiornato

// Configurazione del sensore di temperatura
#define ONE_WIRE_BUS 13 // Collegamento del sensore di temperatura al pin digitale 13
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  stepperX.setMaxSpeed(20 * 360); // 20 steps per mm
  stepperX.setAcceleration(500);

  stepperE.setMaxSpeed(500);
  stepperE.setAcceleration(200);

  lcd.begin(16, 2);
  lcd.print("    Ricicla    ");
  lcd.setCursor(0, 1);
  lcd.print("   Bottiglie   ");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  delay(2000);
  lcd.clear();
  lcd.print("Premi il bottone");
  lcd.setCursor(0, 1);
  lcd.print("per avviare");

  // Inizializzazione del sensore di temperatura
  sensors.begin();
  if (sensors.getDeviceCount() > 0) {
    temperatureDetected = true;
  }
}

void loop() {
  buttonState = digitalRead(BUTTON_PIN);

  if (buttonState != lastButtonState) {
    if (buttonState == LOW) {
      if (!recyclingStarted && !heatingInProgress) {
        startRecycling();
      } else if (recyclingStarted || heatingInProgress) {
        stopRecycling();
      }
    }

    lastButtonState = buttonState;
    delay(50); // Aggiunta di un piccolo ritardo per evitare il rimbalzo del pulsante
  }

  if (recyclingStarted) {
    if (lcdNeedsUpdate) {
      lcd.clear();
      lcd.print("Riciclaggio...");
      lcd.setCursor(0, 1);
      lcd.print("Temp: ");

      // Aggiornamento della temperatura corrente solo se il sensore è stato rilevato
      if (temperatureDetected) {
        currentTemperature = readTemperature();
      }

      if (temperatureDetected && currentTemperature != -127.0) {
        int temperatureInt = static_cast<int>(currentTemperature);
        lcd.print(temperatureInt);
        lcd.write(byte(0));
        lcd.print("C|35C");
      } else {
        lcd.print("-1C|35C"); // Scrive 0C se il sensore di temperatura non è rilevato
      }

      lcdNeedsUpdate = false;
    }

    stepperX.run();
    stepperE.run();

    // Regolazione automatica della temperatura solo se il sensore è stato rilevato
    if (temperatureDetected && currentTemperature < targetTemperature) {
      // Accendi l'estrusore
      // ...
    } else {
      // Spegni l'estrusore
      // ...
    }
  } else if (heatingInProgress) {
    if (lcdNeedsUpdate) {
      lcd.clear();
      lcd.print("Riscaldamento...");
      lcd.setCursor(0, 1);
      lcd.print("Temp: ");

      // Aggiornamento della temperatura corrente solo se il sensore è stato rilevato
      if (temperatureDetected) {
        currentTemperature = readTemperature();
      }

      if (temperatureDetected && currentTemperature != -127.0) {
        int temperatureInt = static_cast<int>(currentTemperature);
        lcd.print(temperatureInt);
        lcd.write(byte(0));
        lcd.print("C|");
        lcd.print(targetTemperature);
        lcd.write(byte(0));
        lcd.print("C");
      } else {
        lcd.print("0C|");
        lcd.print(targetTemperature);
        lcd.write(byte(0));
        lcd.print("C"); // Scrive 0C se il sensore di temperatura non è rilevato
      }

      lcdNeedsUpdate = false;
    }

    stepperX.run();
    stepperE.run();

    // Controllo se il riscaldamento è terminato
    if (temperatureDetected && currentTemperature >= targetTemperature) {
      recyclingStarted = true; // Avvio del riciclaggio
      heatingInProgress = false;
      stepperX.move(1000);
      stepperE.move(1000);
      lcdNeedsUpdate = true; // Aggiorna l'LCD quando il riciclaggio inizia
    }
  } else {
    if (lcdNeedsUpdate) {
      lcd.clear();
      lcd.print("Premi il bottone");
      lcd.setCursor(0, 1);
      lcd.print("per avviare");
      lcdNeedsUpdate = false;
    }

    stepperX.stop();
    stepperE.stop();
  }
}

void startRecycling() {
  heatingInProgress = true;
  lcdNeedsUpdate = true; // Aggiorna l'LCD quando il riscaldamento inizia
  lcd.clear();
  lcd.print("Riscaldamento...");
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");

  // Aggiornamento della temperatura corrente solo se il sensore è stato rilevato
  if (temperatureDetected) {
    currentTemperature = readTemperature();
  }

  if (temperatureDetected && currentTemperature != -127.0) {
    int temperatureInt = static_cast<int>(currentTemperature);
    lcd.print(temperatureInt);
    lcd.print("C|");
    lcd.print(targetTemperature);
    lcd.print("C");
  } else {
    lcd.print("-1C|");
    lcd.print(targetTemperature);
    lcd.print("C"); // Scrive 0C se il sensore di temperatura non è rilevato
  }

  // Riscaldamento dell'estrusore fino alla temperatura desiderata
  while (currentTemperature < targetTemperature && heatingInProgress) {
    if (temperatureDetected) {
      currentTemperature = readTemperature();
    }
    stepperX.run();
    stepperE.run();
    delay(1000); // Attendi 1 secondo tra le letture della temperatura
  }

  if (heatingInProgress) {
    recyclingStarted = true; // Avvio del riciclaggio
    heatingInProgress = false;
    stepperX.move(1000);
    stepperE.move(1000);
    lcd.clear();
    lcd.print("Riciclaggio...");
    lcd.setCursor(0, 1);
    lcd.print("Temp: ");
    lcdNeedsUpdate = true; // Aggiorna l'LCD quando il riciclaggio inizia
  }
}

void stopRecycling() {
  recyclingStarted = false;
  heatingInProgress = false;
  stepperX.stop();
  stepperE.stop();
  lcd.clear();
  lcd.print("Premi il bottone");
  lcd.setCursor(0, 1);
  lcd.print("per avviare");
  lcdNeedsUpdate = false;
}

float readTemperature() {
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  return temperatureC;
}
