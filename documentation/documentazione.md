# DOCUMENTAZIONE MORR-INO CINESE - Michele Bolis & Andrea Galliano

## Indice

- [DOCUMENTAZIONE MORR-INO CINESE - Michele Bolis \& Andrea Galliano](#documentazione-morr-ino-cinese---michele-bolis--andrea-galliano)
  - [Indice](#indice)
    - [Descrizione del progetto](#descrizione-del-progetto)
    - [Materiali utilizzati](#materiali-utilizzati)
    - [Funzionamento dettagliato](#funzionamento-dettagliato)
      - [Rilevamento della mossa](#rilevamento-della-mossa)
      - [Feedback e traduzione della mossa](#feedback-e-traduzione-della-mossa)
    - [Dettagli implementativi: Task Scheduler](#dettagli-implementativi-task-scheduler)
    - [Analisi statistica](#analisi-statistica)
    - [Demo di funzionamento](#demo-di-funzionamento)

### Descrizione del progetto

Il progetto prevede il rilevamento di una mossa del gioco della __*morra cinese*__, riprodotta con la propria mano, con conseguente stima di quest'ultima da parte di diversi classificatori. L'utente dovrà poi confermare la mossa effettuata attraverso tre pulsanti. Infine la mossa verrà *"tradotta"* da una mano meccanica.

Grazie al feedback dell'utente sulla mossa che ha effettuato, è stato possibile analizzare l'accuratezza dei classificatori utilizzati per predire la mossa effettuata.

### Materiali utilizzati

I materiali utilizzati per la realizzazione del progetto sono diversi e comprendono sia i __componenti elettronici__ (come ad esempio i sensori e gli attuatori) sia i __componenti non elettronici__ (è il caso dei materiali usati per la realizzazione della mano meccanica *artigianale*).  

**COMPONENTI ELETTRONICI**:

- ELEGOO UNO R3
- 4 fotoresistenze (per il palmo della mano, per il rilevamento dell'indice, del medio e dell'anulare)
- 5 servomotori (uno per ogni dito della mano da muovere)
- 3 breadboard da 830 contatti
- 7 resistenze da 10k Ohm
- Cavi e jumper  

**COMPONENTI NON ELETTRONICI**:

- Cartoncino  
- Foglio in gomma (dello spessore di mezzo *cm* circa)
- Cannucce in plastica e carta
- Filo di cotone

### Funzionamento dettagliato

Il funzionamento dettagliato del progetto si può dividere in 3 porzioni ben definite:  

1. Rilevamento della mossa
2. Feedback e traduzione della mossa

#### Rilevamento della mossa

La parte relativa al rilevamento di una mossa prevede l'utilizzo di __4 fotoresistenze__, poste in modo tale da percepire 4 punti fondamentali quando si intende effettuare una mossa:

- Il palmo della mano
- L'indice
- Il medio
- L'anulare  

Durante la fase di setup, in cui si presuppone che non ci siano "ostacoli" tra le fotoresistenze e la luce, vengono settate i valori massimi che le fotoresistenze possono rilevare (per aumentare l'accuratezza della stima, questa è risultate dalla media di 20 rilevazioni).  

Per rilevare SE è presente o meno un dito su una fotoresistenza, applichiamo una diminuzione (5%-10%) alla luce massima rilevata e SE la luce rilevata attualente è minore, allora si considera tale fotoresistenza "occupata".  
Chiaramente se la luce ambientale cambiasse, sia nel caso in cui aumenti la luce (non si rileverebbe quasi mai una mossa) sia nel caso in cui diminuisca (si rileverebbe quasi sempre carta), sarebbe necessario un riavvio per rilevare la nuova luce massima.  

I diversi classificatori, date in input il numero di rilevazioni per mossa, restituiscono in output la codifica della mossa, in particolare:  
0 --> __Sasso__  
1 --> __Carta__  
2 --> __Forbice__  

Gli stimatori proposti sono:

- Lo __stimatore della moda__ emette la mossa in base alla moda tra quelle rilevate.

```C++
  int countMax= (max(max(max(nonRiconosciuto, carta), sasso), forbice));
```

- Lo __stimatore random__ emette la mossa scegliendo in maniera pseudo-casuale 1 delle 3 disponibili.

```C++
long randomSegno = random(3);
```

A seguito della previsione di ciascuno stimatore, le mosse predette vengono stampate in output sulla console.

#### Feedback e traduzione della mossa

La fase di traduzione della mossa è possibile dividerla ulteriormente in altre 2 sotto-fasi:  

1. Selezione della mossa corretta tramite bottone
2. Riproduzione della mossa grazie alla mano meccanica  

In seguito alla stampa a video dei risultati degli stimatori, l'utente deve tener premuto uno dei 3 bottoni disponibili per segnalare la mossa che aveva effettuato.

```C++
int sasso = digitalRead(pins.bottone_sasso);
int carta = digitalRead(pins.bottone_carta);
int forbice = digitalRead(pins.bottone_forbice);
```

Dopo la pressione del bottone, i servomotori di controllo delle dita della mano vengono mossi per riprodurre la scelta dell'utente, per poi ritornare in posizione *neutra* e attendere una nuova mossa.  

Prendiamo, ad esempio, il caso della mossa della forbice:

```C++
void servo_forbicePosition() {
  for (int pos = 180; pos >= 1; pos --) {
    mano.pollice.write(pos);
    mano.anulare.write(pos);
    mano.mignolo.write(pos);
    delay(15);
  }
}
```

Infine viene stampato un riassunto contenente la mossa che si è effettuata con le relative mosse predette dagli stimatori. Ora ricomincia tutto dalla lettura della luce per il rilevamento della mossa.

Il codice completo relativo all'intero progetto è consultabile [qui](../morr/morr.ino).

### Dettagli implementativi: Task Scheduler

Per evitare l'utilizzo di delay, è stata utilizzata la libreria __<TaskScheduler.h>__, la cui documentazione è consultabile [qui](https://github.com/arkhipenko/TaskScheduler).  
Grazie a questa libreria, è possibile istanziare un oggetto __*Scheduler*__, che permette la creazione di più task evitando di utilizzare la funzione __*delay()*__, che causerebbe un __blocco totale di tutti i processi__.  
Tutti i task sono stati istanziati basandosi sul __flusso di esecuzione__ del *setup* e sul __flusso di esecuzione__ del *loop*.  

Flusso di esecuzione del *setup*:

![setup path](/documentation/setup_path.png)

Flusso di esecuzione del *loop*:

![loop path](/documentation/execution_path.png)

In base a questo punto, è possibile definire anche il flusso di esecuzione dell'oggetto __Scheduler__ istanziato:

![scheduler path](/documentation/scheduler_path.png)

### Analisi statistica

La terza ed ultima parte riguarda l'analisi statistica sui risultati emessi, atta a valutare la bontà degli stimatori scelti.  
I dati, stampati sulla porta seriale, vengono prelevati e inseriti all'interno di un [file di testo](../Statistiche/log.txt) grazie a [PuTTY](https://www.putty.org/), un particolare tipo di __Client__ che permette di accedere da remoto a sistemi informatici selezionando il tipo di connessione desiderata (nel nostro caso la connessione __*serial*__).  

Come dataset consideriamo il riassunto finale che viene stampato dopo aver premuto uno dei tre pulsanti. Ciò risulta necessario in quanto le mosse effettuate e predette (da ogni stimatore)devono essere della stessa dimensione.  

Come strumento teorico per la verifica della bontà di uno stimatore, utilizziamo la *matrice di confusione*, nella sua versione quadrata 2x2.  
Consideriamo infatti una matrice per ogni stimatore per ogni mossa:
|                            |        |Mossa effettuata       | Mossa effettuata       |                                          |  
|----------------------------|--------|-----------------------|-----------------------|------------------------------------------|
|                            |        |__Positivo__           |__Negativo__           |                                          |
|__Mossa predetta__|Positivo|__VP__, Veri Positivi  |__FP__, Falsi Positivi |__TOT CP__, totale classificati positivi  |
|__Mossa predetta__|Negativo|__FN__, Falsi negativi |__VN__, Veri Negativi  |__TOT CN__, totale classificatori negativi|
|                            |        |__TP__, Totale Positivi|__TN__, Totale Negativi|                                          |

Per ogni mossa, dato uno stimatore, è possibile calcolarne la __sensibilità__ e la __specificità__.  

- __Sensibilità__ --> Capacità del classificatore di lavorare con i positivi.  
$$Sensibilità = {VP \over TP} $$
- __Specificità__ --> Capacità del classificatore di lavorare con i negativi.
$$Specificità = {VN \over TN} $$

Dal punto di vista grafico, rappresentiamo la matrice di confusione con una __mappa di calore__ per evidenziare eventuali concentrazioni di errori.

### Demo di funzionamento
