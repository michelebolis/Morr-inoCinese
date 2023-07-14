# DOCUMENTAZIONE PROGETTUALE MORR-INO CINESE - Michele Bolis & Andrea Galliano

## Indice
- [Descrizione](#descrizione-del-progetto)
- [Materiali utilizzati](#materiali-utilizzati)
- [Funzionamento](#funzionamento-dettagliato)
    - [Rilevamento](#rilevamento)
    - [Traduzione](#traduzione)
    - [Analisi statistica](#analisi-statistica)
- [Uso del Task Scheduler](#uso-del-task-scheduler)
- [Demo di funzionamento](#demo-di-funzionamento)

### Descrizione del progetto:
Il progetto prevede il rilevamento di una mossa del gioco della __*morra cinese*__, tramite appositi **sensori fotosensibili**, la stampa a video del rilevamento e la conferma o la smentita dello stesso tramite la pressione di 1 di 3 bottoni corrispondenti alle mosse del gioco.  
In seguito a questo, la mossa selezionata verrà opportunamente *"tradotta"* mediante l'uso di una mano meccanica manovrata da **attuatori** dotati di motore.  
Vi è inoltre una parte di progetto relativa all'analisi statistica dei rilevamenti passati, per stabilire l'accuratezza dei vari algoritmi usati.

### Materiali utilizzati:
I materiali utilizzati per la realizzazione del progetto sono diversi e comprendono sia i **componenti elettronici** (come ad esempio i vari sensori e attuatori) sia i **componenti non elettronici** (è il caso dei materiali usati per la realizzazione della mano meccanica *artigianale*).   

**COMPONENTI ELETTRONICI**:
- ELEGOO UNO R3
- 4 fotoresistenze (per il palmo della mano, per il rilevamento dell'indice, del medio e dell'anulare)
- 5 Servomotori (uno per ogni dito da muovere)
- 2 breadboard da 30 pin
- Cavi, resistenze e jumper  

**COMPONENTI NON ELETTRONICI**:
- Cartoncino 
- Foglio in gomma (dello spessore di mezzo cm circa)
- Cannucce in plastica e carta
- Filo di cotone

### Funzionamento dettagliato:
Il funzionamento dettagliato del progetto si può dividere in 3 porzioni ben definite:  
1. Rilevamento della mossa
2. Traduzione della mossa
3. Analisi statistica degli algoritmi di rilevamento

#### Rilevamento:
La parte di progetto relativa al rilevamento di una mossa prevede l'utilizzo di **4 fotoresistenze**, le cui variazioni di lettura della luce devono essere inviate per poter stabilire se l'utente ha scelto *carta*, *forbice* o *sasso*.  
Le fotoresistenze, infatti, sono poste in modo tale da percepire 4 punti fondamentali quando si intende effettuare una mossa:
- Il palmo della mano;
- L'indice
- Il medio
- L'anulare  

Ogni mossa è codificata nel seguente formato:  
0 --> **Sasso**  
1 --> **Carta**  
2 --> **Forbice**  

Inoltre per ogni mossa effettuata vi sono più algoritmi che stabiliscono cosa stampare a video.  
- Lo **stimatore della moda** emette la mossa in base alla moda tra quelle rilevate.
```C++
  int countMax= (max(max(max(nonRiconosciuto, carta), sasso), forbice));
```

- Lo **stimatore random** emette la mossa scegliendo in maniera pseudo-casuale 1 delle 3 disponibili.
```C++
long randomSegno = random(3);
```

In seguito a ciò che gli stimatori rilevano, è prevista una stampa a video delle mosse. Questo gioca un ruolo fondamentale dapprima per la parte relativa alla traduzione e, in seguito, anche per quella riguardante le analisi statistiche dell'accuratezza degli stimatori stessi.

#### Traduzione:
La parte di traduzione della mossa è possibile dividerla ulteriormente in altre 2 sotto-fasi:  
1. Selezione della mossa corretta tramite bottone
2. Riproduzione della mossa tramite mano meccanica  

In seguito alla stampa a video dei risultati degli stimatori, l'utente è tenuto a premere 1 dei 3 bottoni disponibili per confermarli o smentirli, grazie alla funzione **digitalRead()**.

```C++
int sasso = digitalRead(pins.bottone_sasso);
int carta = digitalRead(pins.bottone_carta);
int forbice = digitalRead(pins.bottone_forbice);
```

Dopo la pressione del bottone, i motori (che hanno un raggio movimento da 0 180 gradi) che controllano le dita della mano meccanica vengono mossi per riprodurre la scelta dell'utente, per poi ritornare in posizione *neutra* e attendere una nuova mossa.  
Prendiamo, per esempio, la mossa della forbice:

![Schema_collegamento](/documentation/Collegamento_servi.jpeg)

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

Codice completo relativo all'intero circuito consultabile [qui](../morr/morr.ino).

#### Analisi statistica
La terza ed ultima parte riguarda l'analisi statistica sui risultati emessi, atta a valutare la bontà degli stimatori scelti.  
I dati, stampati sulla porta seriale, vengono prelevati e inseriti all'interno di un [file di testo](../Statistiche/log.txt) grazie a [PuTTY](https://www.putty.org/), un particolare tipo di **Client** che permette di accedere da remoto a sistemi informatici selezionando il tipo di connessione desiderata (nel nostro caso la connessione __*serial*__).  

A questo punto è possbile comporre i grafici necessari ed effettuare tutte le analisi del caso.  
Per quanto riguarda il grafico, la scelta è ricaduta sulla **mappa dei calori**, che grazie alla **matrice di confusione** permette di capire quanto uno stimatore sia stato accurato nel rilevamento delle 3 mosse della morra cinese.

### Uso del Task Scheduler:


### Demo di funzionamento