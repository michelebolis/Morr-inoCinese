#include <Servo.h>
#include <TaskScheduler.h>

/**
 * Struttura della mano meccanica le cui dita vengono mosse tramite appositi Servo-Motori
*/
struct mano_servo {
    Servo pollice;
    Servo indice;
    Servo medio;
    Servo anulare;
    Servo mignolo;
};

/**
 * Struttura di interi che rappresenta la luce letta da 4 fotoresistenze: il palmo della mano, il dito indice, il medio e l'anulare.
*/
struct light_detected {
    int palmoMano;
    int indice;
    int medio;
    int anulare;
};

/**
 * Costanti intere rappresentati i GPIO della board adibiti al funzionamento dell'intero circuito.
*/
#define pinLightPalmoMano A4
#define pinLightIndice A3
#define pinLightMedio A2
#define pinLightAnulare A1
  
#define pinButtonSasso 8
#define pinButtonCarta 9
#define pinButtonForbice 10
    
#define pinServoIndice 6
#define pinServoMedio 5
#define pinServoAnulare 4
#define pinServoMignolo 3
#define pinServoPollice 2

mano_servo mano;
light_detected max_light;
light_detected current_light;

/*  Risultati 
  Codifica:
    0 = sasso
    1 = carta
    2 = forbice
*/
byte moda;
byte randomize;
byte alwaysCarta;
byte modaPesata;
byte mossa; 

Scheduler scheduler;

const float secondi_countdown = 5;
int secondi_rimasti = secondi_countdown;

/**
 * Funzione che stampa su porta Serial il countdown durante il rilevamento della mossa per "secondi_countdown" secondi.
*/
void print_countdown() {
  if(secondi_rimasti == 0) {
    disable_countdown();
  } else{
    Serial.print("Inizio Gesto: resta con il gesto per "); Serial.println(secondi_rimasti);
    secondi_rimasti --;
  }
}

/** Task di countdown durante il rilevamento della mossa */
Task countdown(1*TASK_SECOND, TASK_SECOND, print_countdown);

/**
 * Funzione che disabilita il Task del countdown resettando il valore dei secondi rimasti a quelli del countdown.
*/
void disable_countdown() {
  secondi_rimasti = secondi_countdown;
  countdown.disable();
}

/** Numero di rilevazioni totali che ho intenzione di effettuare */
const float taglia_campione = 10;

int rilevazioni_rimaste = taglia_campione;
int nonRiconosciuto;
int carta;
int sasso;
int forbice;

/**
 * Funzione che rileva una mossa finche non si è arrivati al numero di rilevazioni desiderato con una velocita di campionamento ottenuta dal numero di rilevazioni / il numero dei secondi del countdown. Una volta ottenuto il numero di rilevazioni richieste, il segno viene stimato.
*/
void readSegno() {
  if(rilevazioni_rimaste>0){
    update_currentLight();
    if (!isCovered(current_light.palmoMano, max_light.palmoMano)) {
      nonRiconosciuto ++;
    } else if (isCoveredDito(current_light.indice, max_light.indice) && isCoveredDito(current_light.medio, max_light.medio)){
      if (isCoveredDito(current_light.anulare, max_light.anulare)) {
        carta ++;
      } else {
        forbice ++;
      }
    } else if (isCoveredDito(current_light.indice, max_light.indice) || isCoveredDito(current_light.medio, max_light.medio) || isCoveredDito(current_light.anulare, max_light.anulare)) {
      nonRiconosciuto ++;
    } else {
      sasso ++;
    }
    rilevazioni_rimaste --;
  } else {
    stimaSegno();
  }
}

/**
 * Date il numero di rilevazioni per ogni possibile mossa, il segno viene stabilito attraverso diversi stimatori. Viene poi stampata la mossa predetto per ognunno.
*/
void stimaSegno() {
  Serial.println();
  Serial.print("Lo stimatore moda dice: ");
  moda = stimatoreModa(nonRiconosciuto, carta, sasso, forbice);
  print_mossa(moda);
  Serial.println("!");
  
  Serial.print("Lo stimatore random dice: ");
  randomize = stimatoreRandom();
  print_mossa(randomize);
  Serial.println("!");
  
  Serial.print("Lo stimatore carta dice: ");
  alwaysCarta = stimatoreCarta();
  print_mossa(alwaysCarta);
  Serial.println("!");

  Serial.print("Lo stimatore moda pesata dice: ");
  modaPesata = stimatoreModaPesata(nonRiconosciuto, carta, sasso, forbice);
  print_mossa(modaPesata);
  Serial.println("!");

  Serial.println();
  Serial.println("Premi il buttone corrispondente alla mossa che hai effettuato");
  disable_sampling();
  enable_idle_waitButton();
}

/** Valore corrispondente al delay del task del campionamento */
const float velocita_campionamento = secondi_countdown/taglia_campione;

/** Task in esecuzione durante il campionamento della mossa */
Task sampling(velocita_campionamento*TASK_SECOND, TASK_SECOND, readSegno);

/**
 * Funzione che disabilita il Task "sampling" resettando i conteggi delle mosse e delle rilevazioni rimaste
*/
void disable_sampling() {
  rilevazioni_rimaste = taglia_campione;
  nonRiconosciuto = 0;
  carta = 0;
  sasso = 0;
  forbice = 0;
  sampling.disable();
}

/**
 * Aggiorna la lettura della luce corrente e controlla che non venga coperto la fotoresistenza del palmo della mano.
 * Nel caso in cui lo rilevasse, comincia la lettura del segno.
*/
void checkidle_waitMossa()
{
   update_currentLight();
   if (isCovered(current_light.palmoMano, max_light.palmoMano)) {
     disable_idle_waitMossa();
     print_currentLight();
     countdown.enable();
     sampling.enable();
   };
}

/** Task iniziale di inizio mossa */
Task idle_waitMossa(1*TASK_SECOND, TASK_SECOND, checkidle_waitMossa);

/**
 * Disabilita il Task predisposto all'inizio di una mossa.
*/
void disable_idle_waitMossa() {
  idle_waitMossa.disable();
}

/**
 * Funzione che fa ritornare i servomotori con un'angolazione di 0 gradi se la mossa effettuata era diverso da "carta"
*/
void restart() {
  if(mossa != 1){
    servo_defaultPosition();
  }
  disable_restart();
}

/** Task di ritorno allo stato di inizio mossa */
Task restart_idle_waitMossa(1*TASK_SECOND, TASK_SECOND, restart);

/**
 * Funzione che rileva, grazie alla pressione dell'utente di uno dei 3 bottoni preposti, la mossa effettuata. Vengono quindi richiamate le funzioni per tradurre la mossa con i servomotori, vengono stampati i risultati, viene rilevata nuovamente la lucemassima e infine viene abilitato il task "restart_idle_waitMossa" posticipandolo di 5s
*/
void checkButtons()
{
  bool sasso = digitalRead(pinButtonSasso)==1;
  bool carta = digitalRead(pinButtonCarta)==1;
  bool forbice = digitalRead(pinButtonForbice)==1;
  if (!(sasso || forbice || carta)){
    return;
  }
  if(sasso) {
    mossa = 0; // Assegno il risultato della mossa
    Serial.print("Hai detto ");
    print_mossa(mossa);
    Serial.println();
    disable_idle_waitButton();
    servo_sassoPosition();
  }else if(carta) {
    mossa = 1;
    Serial.print("Hai detto ");
    print_mossa(mossa);
    Serial.println();
    disable_idle_waitButton();
  }else if(forbice) {
    mossa = 2;
    Serial.print("Hai detto ");
    print_mossa(mossa);
    Serial.println();
    disable_idle_waitButton();
    servo_forbicePosition(); 
  }
  restart_idle_waitMossa.enableDelayed(5000);
  Serial.println("Guarda la tua mossa");
  print_risultati();
  setup_maxLight(20);
}

/** Task per la mossa da selezionare */
Task idle_waitButton(1*TASK_SECOND, TASK_SECOND, checkButtons);

/**
 * Disabilita il Task predisposto alla selezione di una mossa.
*/
void disable_idle_waitButton() {
  idle_waitButton.disable();
}

/**
 * Abilita il Task predisposto all'attesa della pressione del bottone relativo alla mossa effettuata dall'utente.
*/
void enable_idle_waitButton() {
  idle_waitButton.enable();
}

/**
 * Disabilita il Task predisposto al ritorno in stato di "inizio mossa" (restart_idle_waitMossa) ed attiva quello di inizio di una nuova mossa.
*/
void disable_restart() {
  restart_idle_waitMossa.disable();
  idle_waitMossa.enable();
  Serial.println("Lettura luce");
}

void setup() {
  setup_servoAttach();
  servo_defaultPosition(); // Fa servo_defaultPositionre sempre la mano in posizione neutra
  setup_pinMode(); // Inizializzazione dei pin di input
  setup_maxLight(20);
  Serial.begin(9600);
  print_maxLight(); // Stampa della luce massima (corrispondente alla luce iniziale)
  Serial.println();
  setup_scheduler();
  Serial.println("Lettura luce");
}

void loop() {
  scheduler.execute();
}

/*
 * Associa ad ogni variabile Servo presente in mano, il corrispettivo pin
*/
void setup_servoAttach(){
  mano.indice.attach(pinServoIndice); 
  mano.medio.attach(pinServoMedio);
  mano.anulare.attach(pinServoAnulare);
  mano.mignolo.attach(pinServoMignolo);
  mano.pollice.attach(pinServoPollice);
}

/**
 * Inizializza i GPIO della board.
*/
void setup_pinMode() {
  pinMode(pinButtonSasso, INPUT);
  pinMode(pinButtonCarta, INPUT);
  pinMode(pinButtonForbice, INPUT);
  pinMode(pinLightPalmoMano, INPUT);
  pinMode(pinLightIndice, INPUT);
  pinMode(pinLightMedio, INPUT);
  pinMode(pinLightAnulare, INPUT);
}

/**
 * Inizializza la luce "massima", ovvero la luce iniziale letta appena il circuito viene acceso.
*/
void setup_maxLight(int n) {
  for (int i=0; i<n; i++){
    max_light.palmoMano += analogRead(pinLightPalmoMano);
    max_light.indice += analogRead(pinLightIndice);
    max_light.medio += analogRead(pinLightMedio);
    max_light.anulare += analogRead(pinLightAnulare);
  }
  max_light.palmoMano = max_light.palmoMano/n;
  max_light.indice = max_light.indice/n;
  max_light.medio = max_light.medio/n;
  max_light.anulare = max_light.anulare/n;
}


/*
  Funzione che inizializza lo scheduler e aggiunge i relativi Task, attivando solo quello di idle_waitMossa
*/
void setup_scheduler(){
  scheduler.init();
  scheduler.addTask(idle_waitMossa);
  scheduler.addTask(idle_waitButton);
  scheduler.addTask(restart_idle_waitMossa);
  scheduler.addTask(countdown);
  scheduler.addTask(sampling);
  idle_waitMossa.enable(); 
}

/**
 * Funzione predisposta a far tornare i servomotori in posizione neutra.
*/
void servo_defaultPosition() {
  for (int pos = 0; pos < 180; pos ++) // Viene impostato un ciclo con valori che vanno da 180 a 0 gradi
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
 * Stampa i risultati della mossa effettuata dopo la pressione del bottone e stampa anche i rilevamenti degli stimatori moda, random, carta e moda pesata.
*/
void print_risultati() {
  Serial.println();
  Serial.println("Risultati:");
  Serial.print("Mossa effettuata: ");
  print_mossa(mossa);
  Serial.println("!");
  
  Serial.print("Mossa rilevata: ");
  printStimatore(moda);
  printStimatore(randomize);
  printStimatore(alwaysCarta);
  printStimatore(modaPesata);
  Serial.println("!");
  Serial.println();
}

/**
  Funzione che, data la codifica della mossa, stampa la mossa corrispondente seguita da una ','
*/
void printStimatore(byte mossa){
  print_mossa(mossa);
  Serial.print(", ");
}

/*
  Funzione che, data la cofica della mossa, stampa la mossa corrispondente all intero passato come argomento.
*/
void print_mossa(int mossa) {
  switch (mossa) {
    case 0: Serial.print("sasso"); break;
    case 1: Serial.print("carta"); break;
    case 2: Serial.print("forbice"); break;
    default: Serial.print("ERRORE");
  }
}

/**
 * Funzione che ritorna un valore booleano corrispondente alla copertura (o non copertura) del sensore del palmo della mano
*/
bool isCovered(int light, int maxLight) {
  float variazione = 0.1;
  return light < maxLight*(1 - variazione);
}

/**
 * Funzione che ritorna un valore booleano corrispondente alla copertura (o non copertura) di uno dei sensori delle dita della mano 
*/
bool isCoveredDito(int light, int maxLight) {
  float variazioneDita = 0.05;
  return light < maxLight*(1 - variazioneDita);
}

/**
 * Stabilisce il segno effettuato dall'utente scegliendo il conteggio con la frequenza assoluta maggiore e lo stampa.
*/
byte stimatoreModa(int nonRiconosciuto, int carta, int sasso, int forbice) {
  int countMax = (max(max(max(nonRiconosciuto, carta), sasso), forbice));
  if (countMax = nonRiconosciuto) {
    Serial.print("Segno non riconosciuto ma a caso dico ");
    return stimatoreRandom();
  }
  if (countMax = carta) {return 1;}
  if (countMax = sasso) {return 0;}
  if (countMax = forbice) {return 2;}
}

/**
 * Stabilisce randomicamente un segno fra i 3 possibili e lo stampa.
*/
byte stimatoreRandom() {
  return random(3); 
}

/*
 * Stabilisce sempre che il segno effettuato sia carta
*/
byte stimatoreCarta() {
  return 1;
}

/**
 * Stabilisce il segno effettuato associando un peso ad ogni mossa e applicandole alle frequenze assolute, richiamando poi lo stimatore moda.
*/
byte stimatoreModaPesata(int nonRiconosciuto, int carta, int sasso, int forbice) {
  float pesoNonRiconosciuto = 0.5; //ci possono essere molti false rilevazioni
  float pesoCarta = 0.9; 
  float pesoSasso = 0.8;
  float pesoForbice = 1; // segno rilevato con maggior difficolta
  
  return stimatoreModa(nonRiconosciuto * pesoNonRiconosciuto, carta * pesoCarta, pesoSasso * sasso, pesoForbice * forbice);
}

/**
 * Aggiorna i valori riguardanti la luce letta al momento.
*/
void update_currentLight() {
  current_light.palmoMano = analogRead(pinLightPalmoMano);
  current_light.indice = analogRead(pinLightIndice);
  current_light.medio = analogRead(pinLightMedio);
  current_light.anulare = analogRead(pinLightAnulare);
}

/**
 * Stampa i valori correnti della luce letta per il palmo della mano e le 3 dita interessate (indice, medio ed anulare).
*/
void print_currentLight() {
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
void print_maxLight() {
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
void servo_forbicePosition() {
  for (int pos = 180; pos >= 1; pos --) // Viene impostato un ciclo con valori che vanno da 180 a 0 gradi
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
void servo_sassoPosition() {
  for (int pos = 180; pos >= 1; pos --) // Viene impostato un ciclo con valori che vanno da 180 a 0 gradi
  {
    mano.indice.write(pos);
    mano.medio.write(pos);
    mano.anulare.write(pos);
    mano.mignolo.write(pos);
    mano.pollice.write(pos);
    delay(15);
  }
}