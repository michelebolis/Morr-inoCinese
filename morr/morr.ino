#include <Servo.h> // include la Libreria Servo.h
#include <TaskManager.h>

struct mano_servo
{
    Servo pollice;
    Servo indice;
    Servo medio;
    Servo anulare;
    Servo mignolo;
};
struct light_detected
{
    int palmoMano;
    int indice;
    int medio;
    int anulare;
};
struct pin_list 
{
    const int palmoMano = A4;
    const int indice = A3;
    const int medio = A2;
    const int anulare = A1;
  
    const int bottone_sasso = 8;
    const int bottone_carta = 9;
    const int bottone_forbice = 10;
    
    const int servo_indice = 6;
    const int servo_medio = 5;
    const int servo_anulare = 4;
    const int servo_mignolo = 3;
    const int servo_pollice = 2;
};

pin_list pins;
mano_servo mano;
light_detected max_light;
light_detected current_light;

int pos = 0;    // inizializza una variabile di tipo intero pos il cui valore sarà la posizione da impartire al servo

float variazione = 0.1;
float variazioneDita = 0.05;

void setup(){
    mano.indice.attach(pins.servo_indice); // lega l'oggetto servo_indice al pin a cui abbiamo collegato il nostro servo, in questo caso il pin 8
    mano.medio.attach(pins.servo_medio);
    mano.anulare.attach(pins.servo_anulare);
    mano.mignolo.attach(pins.servo_mignolo);
    mano.pollice.attach(pins.servo_pollice);
    torna();
    setupPinMode();
    setup_maxLight(20);
    Serial.begin(9600);
    print_maxLight();
}
void loop(){
  
  update_currentLight();
    if(checkPalmo()){
      print_currentLight();
      readSegno();
      delay(6000);
    };
}

void setupPinMode(){
  pinMode(pins.bottone_sasso, INPUT);
  pinMode(pins.bottone_carta, INPUT);
  pinMode(pins.bottone_forbice, INPUT);
  pinMode(pins.palmoMano, INPUT);
  pinMode(pins.indice, INPUT);
  pinMode(pins.medio, INPUT);
  pinMode(pins.anulare, INPUT);
}

void setup_maxLight(int n){
  for (int i=0; i<n; i++){
    max_light.palmoMano += analogRead(pins.palmoMano);
    max_light.indice += analogRead(pins.indice);
    max_light.medio += analogRead(pins.medio);
    max_light.anulare += analogRead(pins.anulare);
  }
  max_light.palmoMano = max_light.palmoMano/n;
  max_light.indice = max_light.indice/n;
  max_light.medio = max_light.medio/n;
  max_light.anulare = max_light.anulare/n;
}

void torna(){
    for (pos = 0; pos < 180; pos ++) // In questo caso imposta un ciclo con valori che vanno da 180 a 0
  {
    mano.indice.write(pos);
    mano.medio.write(pos);
    mano.anulare.write(pos);
    mano.mignolo.write(pos);
    mano.pollice.write(pos);
    delay(15);
  }
}

void readSegno(){
  int nonRiconosciuto = 0;
  int carta = 0;
  int sasso = 0;
  int forbice = 0;
  int wait = 5;
  int i = 0;
  for (;i<wait; i++){
    Serial.print("Inizio Gesto: resta con il gesto per ");
    Serial.println(wait-i);
    update_currentLight();
    if (checkIndice() && checkMedio() && checkPalmo() && checkAnulare()){
      carta++;
    }else if (checkIndice() && checkMedio() && checkPalmo()){
      forbice++;
    }else if (checkPalmo()){
      sasso++;
    }else{
      nonRiconosciuto++;
    }
    
    delay(1000); // delay di 1 secondo
  }
  Serial.print("Lo stimatore moda dice: ");
  checkSegno_moda(nonRiconosciuto, carta, sasso, forbice);
  Serial.print("Lo stimatore random dice: ");
  checkSegno_random();
  while(true){
   int sasso = digitalRead(pins.bottone_sasso);
   int carta = digitalRead(pins.bottone_carta);
   int forbice = digitalRead(pins.bottone_forbice);
   Serial.println("sasso");
   Serial.println(sasso);
   Serial.println("carta");
   Serial.println(carta);
   Serial.println("forbice");
   Serial.println(forbice);
   Serial.println();
   if(sasso==1){
    Serial.println("Hai detto sasso");
    mossa_sasso();
    delay(10000);
    torna();
    break;
  }else if(carta==1){
    Serial.println("Hai detto carta");
        break;
  }else if(forbice==1){
    Serial.println("Hai detto forbice");
    mossa_forbice();
    delay(10000);
    torna();
        break;
  }
   delay(1000);
  };
  
  
}

bool checkPalmo(){
  return current_light.palmoMano < max_light.palmoMano*(1-variazione);
}

bool checkIndice(){
  return current_light.indice < max_light.indice*(1-variazioneDita);
}

bool checkMedio(){
  return current_light.medio < max_light.medio*(1-variazioneDita);
}

bool checkAnulare(){
  return current_light.anulare < max_light.anulare*(1-variazioneDita);
}

void checkSegno_moda(int nonRiconosciuto, int carta, int sasso, int forbice){
  int countMax= (max(max(max(nonRiconosciuto, carta), sasso), forbice));
  if (countMax = nonRiconosciuto) {
    Serial.print("Segno non riconosciuto ma a caso dico ");
    checkSegno_random();
    return;
  }
  if (countMax = carta){
    Serial.println("Carta!");
    return;
  }
  if (countMax = sasso){
    Serial.println("Sasso!");
    return;
  }
  if (countMax = forbice){
    Serial.println("Forbice!");
  }
}

void checkSegno_random(){
  long randomSegno = random(3);
  switch (randomSegno){
      case 0: Serial.println("Sasso!"); return;
      case 1: Serial.println("Carta!"); return;
      case 2: Serial.println("Forbice!"); return;
  }  
}

void update_currentLight(){
  current_light.palmoMano = analogRead(pins.palmoMano);
  current_light.indice = analogRead(pins.indice);
  current_light.medio = analogRead(pins.medio);
  current_light.anulare = analogRead(pins.anulare);
}

void print_currentLight(){
    update_currentLight();
    Serial.print("palmoMano: ");
    Serial.println(current_light.palmoMano);
    Serial.print("indice: ");
    Serial.println(current_light.indice);
    Serial.print("medio: ");
    Serial.println(current_light.medio);
    Serial.print("anulare: ");
    Serial.println(current_light.anulare);

    Serial.println();
    delay(1000);
}

void print_maxLight(){
  Serial.print("Max Luce palmo mano: ");
    Serial.println(max_light.palmoMano);
  Serial.print("Max Luce indice: ");
    Serial.println(max_light.indice);
  Serial.print("Max Luce medio: ");
    Serial.println(max_light.medio);
  Serial.print("Max Luce anulare: ");
    Serial.println(max_light.anulare);
} 

void mossa_forbice() {
  for (pos = 180; pos >= 1; pos --)
  {
    mano.pollice.write(pos);
    mano.anulare.write(pos);
    mano.mignolo.write(pos);
    delay(15);
  }
}

void mossa_sasso() {
  for (pos = 180; pos >= 1; pos -= 1) // In questo caso imposta un ciclo con valori che vanno da 180 a 0
  {
    mano.indice.write(pos);
    mano.medio.write(pos);
    mano.anulare.write(pos);
    mano.mignolo.write(pos);
    mano.pollice.write(pos);
    delay(15);
  }
}
