#include <Servo.h>
#include <TaskScheduler.h>

/**
 * Struttura della mano meccanica le cui dita vengono mosse tramite appositi Servo-Motori
*/
struct mano_servo
{
    Servo pollice;
    Servo indice;
    Servo medio;
    Servo anulare;
    Servo mignolo;
};

/**
 * Struttura di interi che rappresenta la luce letta in 4 punti: il palmo della mano, il dito indice, il medio e l'anulare.
*/
struct light_detected
{
    int palmoMano;
    int indice;
    int medio;
    int anulare;
};

/**
 * Struttura di costanti intere rappresentati i pin della board adibiti al funzionamento dell'intero circuito.
*/
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

/** Risultati */
int moda;
int randomize;
int mossa;

/** Nuovo oggetto Scheduler istanziato per i Task che comporranno il circuito */
Scheduler scheduler;

/**
 * Aggiorna la lettura della luce corrente e controlla che non venga rilevato il palmo della mano.
 * Nel caso in cui lo rilevasse, comincia la lettura del segno.
*/
void checkInizioMossa(){
   update_currentLight();
   if(checkPalmo()){
     disattiva_inizioMossa();
     print_currentLight();
     readSegno();
   };
}

/** Task iniziale di inizio mossa */
Task inizioMossa(1*TASK_SECOND, TASK_SECOND, checkInizioMossa);

/**
 * Disattiva il Task predisposto all'inizio di una mossa.
*/
void disattiva_inizioMossa(){
  inizioMossa.disable();
}

/**
 * Funzione che posizione le dita della mano in posizione neutra e fa tornare lo stato dello Scheduler al Task "inizio mossa".
*/
void backto(){
  torna();
  disattiva_backto();
}

/** Task di ritorno allo stato di inizio mossa */
Task backto_inizioMossa(1*TASK_SECOND, TASK_SECOND, backto);

/**
 * Funzione che rileva, grazie alla pressione dell'utente di uno dei 3 bottoni preposti, la mossa effettuata segnalandolo all'utente tramite stampa di un messaggio ed il movimento della mano meccanica.
*/
void checkBottoni(){
  int sasso = digitalRead(pins.bottone_sasso);
  int carta = digitalRead(pins.bottone_carta);
  int forbice = digitalRead(pins.bottone_forbice);
  if(sasso==1){
      Serial.println("Hai detto sasso");
      disattiva_selezionaMossa();
      mossa_sasso();
      mossa=0;
      backto_inizioMossa.enableDelayed(5000);
      Serial.println("Guarda la tua mossa");
      print_risultati();
  }else if(carta==1){
      Serial.println("Hai detto carta");
      disattiva_selezionaMossa();
      mossa=1;
      backto_inizioMossa.enableDelayed(5000);
      Serial.println("Guarda la tua mossa");
      print_risultati();
   }else if(forbice==1){
      Serial.println("Hai detto forbice");
      disattiva_selezionaMossa();
      mossa_forbice();
      mossa=2;
      backto_inizioMossa.enableDelayed(5000);
      Serial.println("Guarda la tua mossa");
      print_risultati();  
   }
}

/** Task per la mossa da selezionare */
Task selezionaMossa(1*TASK_SECOND, TASK_SECOND, checkBottoni);

/**
 * Disattiva il Task predisposto alla selezione di una mossa.
*/
void disattiva_selezionaMossa(){
  selezionaMossa.disable();
}

/**
 * Disattiva il Task predisposto al ritorno in stato di "inizio mossa" (backto_inizioMossa) ed attiva quello di inizio di una nuova mossa.
*/
void disattiva_backto(){
  backto_inizioMossa.disable();
  inizioMossa.enable();
  Serial.println("Lettura luce");
}


/** Variabile di tipo intero il cui valore sarà la posizione, in gradi, da impartire al servo */
int pos = 0;


/** Variabili di tipo float necessarie per il calcolo più accurato dei valori riguardanti la luce rilevata durante l'esecuzione di una mossa */
float variazione = 0.1;
float variazioneDita = 0.05;


void setup(){
    mano.indice.attach(pins.servo_indice); // Lega l'oggetto servo_indice al pin a cui abbiamo collegato il nostro servo, in questo caso il pin 8
    mano.medio.attach(pins.servo_medio);
    mano.anulare.attach(pins.servo_anulare);
    mano.mignolo.attach(pins.servo_mignolo);
    mano.pollice.attach(pins.servo_pollice);

    torna(); // Fa tornare sempre la mano in posizione neutra
    
    setupPinMode(); // Inizializzazione dei pin di input
    setup_maxLight(20);

    Serial.begin(9600);
    print_maxLight(); // Stampa della luce massima (corrispondente alla luce iniziale)

    /** Gestione dello Scheduler e dei Task che lo compongono */
    scheduler.init();
    scheduler.addTask(inizioMossa);
    scheduler.addTask(selezionaMossa);
    scheduler.addTask(backto_inizioMossa);
    inizioMossa.enable(); // Attivazione del Task di inizio mossa

    Serial.println("Lettura luce");
}

void loop(){
  scheduler.execute();
}

/**
 * Inizializza i pin del circuito predisposti agli input.
*/
void setupPinMode(){
  pinMode(pins.bottone_sasso, INPUT);
  pinMode(pins.bottone_carta, INPUT);
  pinMode(pins.bottone_forbice, INPUT);
  pinMode(pins.palmoMano, INPUT);
  pinMode(pins.indice, INPUT);
  pinMode(pins.medio, INPUT);
  pinMode(pins.anulare, INPUT);
}

/**
 * Inizializza la luce "massima", ovvero la luce iniziale letta appena il circuito viene acceso.
*/
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

/**
 * Funzione predisposta a far tornare in posizione neutra la mano.
*/
void torna(){
    for (pos = 0; pos < 180; pos ++) // Viene impostato un ciclo con valori che vanno da 180 a 0 gradi
  {
    mano.indice.write(pos);
    mano.medio.write(pos);
    mano.anulare.write(pos);
    mano.mignolo.write(pos);
    mano.pollice.write(pos);
    delay(15);
  }
}

/**
 * Stampa i risultati della mossa effettuata dopo la pressione del bottone e stampa anche i rilevamenti degli stimatori moda e random.
*/
void print_risultati(){
  Serial.println();
  Serial.print("Risultati: Mossa effettuata: ");
  switch (mossa){
    case 0: Serial.print("sasso"); break;
    case 1: Serial.print("carta"); break;
    case 2: Serial.print("forbice"); break;
  }
  Serial.println("!");
  Serial.print("Mossa rilevata: ");
  switch (moda){
    case 0: Serial.print("sasso"); break;
    case 1: Serial.print("carta"); break;
    case 2: Serial.print("forbice"); break;
  }
  Serial.print(", ");
  switch (randomize){
    case 0: Serial.print("sasso"); break;
    case 1: Serial.print("carta"); break;
    case 2: Serial.print("forbice"); break;
  }
  Serial.println("!");
  Serial.println();
}

/**
 * Rileva la mossa avvalendosi di 5 secondi per farlo correttamente e ne stampa i risultati dei vari stimatori.
*/
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
  moda = checkSegno_moda(nonRiconosciuto, carta, sasso, forbice);
  Serial.print("Lo stimatore random dice: ");
  randomize = checkSegno_random();
  selezionaMossa.enable();
}

/**
 * Funzione predisposta al controllo della variazione di luce sul palmo della mano.
*/
bool checkPalmo(){
  return current_light.palmoMano < max_light.palmoMano*(1-variazione);
}

/**
 * Funzione predisposta al controllo della variazione di luce sul dito indice.
*/
bool checkIndice(){
  return current_light.indice < max_light.indice*(1-variazioneDita);
}

/**
 * Funzione predisposta al controllo della variazione di luce sul dito medio.
*/
bool checkMedio(){
  return current_light.medio < max_light.medio*(1-variazioneDita);
}

/**
 * Funzione predisposta al controllo della variazione di luce sul dito anulare.
*/
bool checkAnulare(){
  return current_light.anulare < max_light.anulare*(1-variazioneDita);
}

/**
 * Stabilisce il segno effettuato dall'utente confrontando il massimo tra i valori che possono essere rilevati (non riconosciuto, carta, forbice e sasso) e lo stampa.
*/
int checkSegno_moda(int nonRiconosciuto, int carta, int sasso, int forbice){
  int countMax= (max(max(max(nonRiconosciuto, carta), sasso), forbice));
  if (countMax = nonRiconosciuto) {
    Serial.print("Segno non riconosciuto ma a caso dico ");
    return checkSegno_random();
  }
  if (countMax = carta){
    Serial.println("Carta!");
    return 1;
  }
  if (countMax = sasso){
    Serial.println("Sasso!");
    return 0;
  }
  if (countMax = forbice){
    Serial.println("Forbice!");
    return 2;
  }
}

/**
 * Stabilisce randomicamente un segno fra i 3 possibili e lo stampa.
*/
int checkSegno_random(){
  long randomSegno = random(3);
  switch (randomSegno){
      case 0: Serial.println("Sasso!"); return 0;
      case 1: Serial.println("Carta!"); return 1;
      case 2: Serial.println("Forbice!"); return 2;
  }  
}

/**
 * Aggiorna i valori riguardanti la luce letta al momento.
*/
void update_currentLight(){
  current_light.palmoMano = analogRead(pins.palmoMano);
  current_light.indice = analogRead(pins.indice);
  current_light.medio = analogRead(pins.medio);
  current_light.anulare = analogRead(pins.anulare);
}

/**
 * Stampa i valori correnti della luce letta per il palmo della mano e le 3 dita interessate (indice, medio ed anulare).
*/
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

/**
 * Stampa i valori massimi della luce letta per il palmo della mano e le 3 dita interessate (indice, medio ed anulare).
*/
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

/**
 * Sposta, in gradi, le alette dei servo-motori in modo tale da far muovere la mano e ricreare la mossa della forbice.
*/
void mossa_forbice() {
  for (pos = 180; pos >= 1; pos --) // Viene impostato un ciclo con valori che vanno da 180 a 0 gradi
  {
    mano.pollice.write(pos);
    mano.anulare.write(pos);
    mano.mignolo.write(pos);
    delay(15);
  }
}

/**
 * Sposta, in gradi, le alette dei servo-motori in modo tale da far muovere la mano e ricreare la mossa del sasso.
*/
void mossa_sasso() {
  for (pos = 180; pos >= 1; pos -= 1) // Viene impostato un ciclo con valori che vanno da 180 a 0 gradi
  {
    mano.indice.write(pos);
    mano.medio.write(pos);
    mano.anulare.write(pos);
    mano.mignolo.write(pos);
    mano.pollice.write(pos);
    delay(15);
  }
}
