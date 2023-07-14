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

/*  Risultati 
  Codifica:
    0 = sasso
    1 = carta
    2 = forbice
*/
int moda;
int randomize;
int mossa; 

/** Nuovo oggetto Scheduler istanziato per i Task che comporranno il circuito */
Scheduler scheduler;

const float secondi_countdown = 5;
int secondi_rimasti=secondi_countdown;
void print_countdown(){
  if(secondi_rimasti==0){disable_countdown();}
  else{
    Serial.print("Inizio Gesto: resta con il gesto per "); Serial.println(secondi_rimasti);
    secondi_rimasti--;
  }
}

Task countdown(1*TASK_SECOND, TASK_SECOND, print_countdown);

void disable_countdown(){
  secondi_rimasti=secondi_countdown;
  countdown.disable();
}

const float taglia_campione = 10;
int rilevazioni_rimaste = taglia_campione;
int nonRiconosciuto;
int carta;
int sasso;
int forbice;

/**
 * Rileva la mossa avvalendosi di 5 secondi_countdown per farlo correttamente e ne stampa i risultati dei vari stimatori.
*/
void readSegno() {
  if(rilevazioni_rimaste>0){
    update_currentLight();
    if (checkIndice() && checkMedio() && checkPalmo() && checkAnulare()) {
      carta ++;
    }else if (checkIndice() && checkMedio() && checkPalmo()){
      forbice ++;
    }else if (checkPalmo()) {
      sasso ++;
    }else{
      nonRiconosciuto ++;
    }
    rilevazioni_rimaste--;
  }else{
    Serial.println();
    Serial.print("Lo stimatore moda dice: ");
    moda = checkSegno_moda(nonRiconosciuto, carta, sasso, forbice);
    Serial.print("Lo stimatore random dice: ");
    randomize = checkSegno_random();

    Serial.println();
    Serial.println("Premi il buttone corrispondente alla mossa che hai effettuato");
    disable_sampling();
    enable_idle_waitButton();
  }
}

const float adjustment = secondi_countdown/taglia_campione;
Task sampling(adjustment*TASK_SECOND, TASK_SECOND, readSegno);

void disable_sampling(){
  rilevazioni_rimaste=taglia_campione;
  nonRiconosciuto=0;
  carta=0;
  sasso=0;
  forbice=0;
  sampling.disable();
}

/**
 * Aggiorna la lettura della luce corrente e controlla che non venga rilevato il palmo della mano.
 * Nel caso in cui lo rilevasse, comincia la lettura del segno.
*/
void checkidle_waitMossa()
{
   update_currentLight();
   if (checkPalmo()) {
     disable_idle_waitMossa();
     print_currentLight();
     countdown.enable();
     sampling.enable();
   };
}

/** Task iniziale di inizio mossa */
Task idle_waitMossa(1*TASK_SECOND, TASK_SECOND, checkidle_waitMossa);

/**
 * disable il Task predisposto all'inizio di una mossa.
*/
void disable_idle_waitMossa() {
  idle_waitMossa.disable();
}

/**
 * Funzione che posizione le dita della mano in posizione neutra e fa servo_defaultPositionre lo stato dello Scheduler al Task "inizio mossa".
*/
void restart()
{
  if(mossa!=1){
    servo_defaultPosition();
  }
  disable_restart();
}

/** Task di ritorno allo stato di inizio mossa */
Task restart_idle_waitMossa(1*TASK_SECOND, TASK_SECOND, restart);

/**
 * Funzione che rileva, grazie alla pressione dell'utente di uno dei 3 bottoni preposti, la mossa effettuata segnalandolo all'utente tramite stampa di un messaggio ed il movimento della mano meccanica.
*/
void checkButtons()
{
  int sasso = digitalRead(pins.bottone_sasso);
  int carta = digitalRead(pins.bottone_carta);
  int forbice = digitalRead(pins.bottone_forbice);
  if (sasso+forbice+carta==0){
    return;
  }
  if(sasso == 1) {
    Serial.println("Hai detto sasso");
    disable_idle_waitButton();
    servo_sassoPosition();
    mossa = 0; // Assegno il risultato della mossa
  }else if(carta == 1) {
    Serial.println("Hai detto carta");
    disable_idle_waitButton();
    mossa = 1;
  }else if(forbice == 1) {
    Serial.println("Hai detto forbice");
    disable_idle_waitButton();
    servo_forbicePosition();
    mossa = 2; 
  }
  restart_idle_waitMossa.enableDelayed(5000);
  Serial.println("Guarda la tua mossa");
  print_risultati();
}

/** Task per la mossa da selezionare */
Task idle_waitButton(1*TASK_SECOND, TASK_SECOND, checkButtons);

/**
 * disable il Task predisposto alla selezione di una mossa.
*/
void disable_idle_waitButton() {
  idle_waitButton.disable();
}

void enable_idle_waitButton(){
  idle_waitButton.enable();
}

/**
 * disable il Task predisposto al ritorno in stato di "inizio mossa" (restart_idle_waitMossa) ed attiva quello di inizio di una nuova mossa.
*/
void disable_restart() {
  restart_idle_waitMossa.disable();
  idle_waitMossa.enable();
  Serial.println("Lettura luce");
}

/** Variabili di tipo float necessarie per il calcolo più accurato dei valori riguardanti la luce rilevata durante l'esecuzione di una mossa */
float variazione = 0.1;
float variazioneDita = 0.05;


void setup() {
  setup_servoAttach();
  servo_defaultPosition(); // Fa servo_defaultPositionre sempre la mano in posizione neutra
  setup_pinMode(); // Inizializzazione dei pin di input
  setup_maxLight(20);
  Serial.begin(9600);
  print_maxLight(); // Stampa della luce massima (corrispondente alla luce iniziale)
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
  mano.indice.attach(pins.servo_indice); 
  mano.medio.attach(pins.servo_medio);
  mano.anulare.attach(pins.servo_anulare);
  mano.mignolo.attach(pins.servo_mignolo);
  mano.pollice.attach(pins.servo_pollice);
}

/**
 * Inizializza i pin del circuito predisposti agli input.
*/
void setup_pinMode() {
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
void setup_maxLight(int n) {
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
 * Funzione predisposta a far servo_defaultPositionre in posizione neutra la mano.
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
 * Stampa i risultati della mossa effettuata dopo la pressione del bottone e stampa anche i rilevamenti degli stimatori moda e random.
*/
void print_risultati() {
  Serial.println();
  Serial.print("Risultati: Mossa effettuata: ");
  print_mossa(mossa);
  Serial.println("!");
  Serial.print("Mossa rilevata: ");
  print_mossa(moda);
  Serial.print(", ");
  print_mossa(randomize);
  Serial.println("!");
  Serial.println();
}

/*
  Funzione che, seguendo la codifica delle mosse, stampa la mossa corrispondente all intero passato come argomento.
*/
void print_mossa(int mossa){
  switch (mossa) {
    case 0: Serial.print("sasso"); break;
    case 1: Serial.print("carta"); break;
    case 2: Serial.print("forbice"); break;
    default: Serial.print("ERRORE");
  }
}

/**
 * Funzione predisposta al controllo della variazione di luce sul palmo della mano.
*/
bool checkPalmo() {
  return current_light.palmoMano < max_light.palmoMano*(1-variazione);
}

/**
 * Funzione predisposta al controllo della variazione di luce sul dito indice.
*/
bool checkIndice() {
  return current_light.indice < max_light.indice*(1-variazioneDita);
}

/**
 * Funzione predisposta al controllo della variazione di luce sul dito medio.
*/
bool checkMedio() {
  return current_light.medio < max_light.medio*(1-variazioneDita);
}

/**
 * Funzione predisposta al controllo della variazione di luce sul dito anulare.
*/
bool checkAnulare() {
  return current_light.anulare < max_light.anulare*(1-variazioneDita);
}

/**
 * Stabilisce il segno effettuato dall'utente confrontando il massimo tra i valori che possono essere rilevati (non riconosciuto, carta, forbice e sasso) e lo stampa.
*/
int checkSegno_moda(int nonRiconosciuto, int carta, int sasso, int forbice) {
  int countMax= (max(max(max(nonRiconosciuto, carta), sasso), forbice));
  if (countMax = nonRiconosciuto) {
    Serial.print("Segno non riconosciuto ma a caso dico ");
    return checkSegno_random();
  }
  if (countMax = carta) {
    Serial.println("Carta!");
    return 1;
  }
  if (countMax = sasso) {
    Serial.println("Sasso!");
    return 0;
  }
  if (countMax = forbice) {
    Serial.println("Forbice!");
    return 2;
  }
}

/**
 * Stabilisce randomicamente un segno fra i 3 possibili e lo stampa.
*/
int checkSegno_random() {
  long randomSegno = random(3);
  switch (randomSegno) {
      case 0: Serial.println("Sasso!"); return 0;
      case 1: Serial.println("Carta!"); return 1;
      case 2: Serial.println("Forbice!"); return 2;
  }  
}

/**
 * Aggiorna i valori riguardanti la luce letta al momento.
*/
void update_currentLight() {
  current_light.palmoMano = analogRead(pins.palmoMano);
  current_light.indice = analogRead(pins.indice);
  current_light.medio = analogRead(pins.medio);
  current_light.anulare = analogRead(pins.anulare);
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
