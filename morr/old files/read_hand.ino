//Constants
const int palmoMano = A4; // Photoresistor at Arduino analog pin A0
const int indice = A3;
const int medio = A2;
const int anulare = A1;

//Variables
int luce_palmoMano=0; 
int luce_indice=0;
int luce_medio=0;
int luce_anulare=0;

int luceMax_palmoMano;
int luceMax_indice;
int luceMax_medio;
int luceMax_anulare;

float variazione=0.1;
float variazioneDita=0.05;

void setup(){
 setupPinMode();
 setupPhotoresistancesMax();
 Serial.begin(9600);
}

void setupPhotoresistancesMax(){
  luceMax_palmoMano=analogRead(palmoMano);
  luceMax_indice=analogRead(indice);
  luceMax_medio=analogRead(medio);
  luceMax_anulare=analogRead(anulare);
}

void setupPinMode(){
  pinMode(palmoMano, INPUT);
  pinMode(indice, INPUT);
  pinMode(medio, INPUT);
  pinMode(anulare, INPUT);
}


void loop(){
  if (checkPalmo()){
    readSegno();
  }
  delay(200);
}

void readSegno(){
  int nonRiconosciuto=0;
  int carta=0;
  int sasso=0;
  int forbice=0;
  int wait=5;
  int i=0;
  for (;i<wait; i++){
    Serial.print("Inizio Gesto: resta con il gesto per ");
    Serial.println(wait-i);
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
  checkSegno(nonRiconosciuto, carta, sasso, forbice);
}

bool checkPalmo(){
  int current_palmoMano=analogRead(palmoMano);
  return current_palmoMano < luceMax_palmoMano*(1-variazione);
}

bool checkIndice(){
  int current_indice=analogRead(indice);
  return current_indice < luceMax_indice*(1-variazioneDita);
}

bool checkMedio(){
  int current_medio=analogRead(medio);
  return current_medio < luceMax_medio*(1-variazioneDita);
}

bool checkAnulare(){
  int current_anulare=analogRead(anulare);
  return current_anulare < luceMax_anulare*(1-variazioneDita);
}

void checkSegno(int nonRiconosciuto, int carta, int sasso, int forbice){
  int countMax= (max(max(max(nonRiconosciuto, carta), sasso), forbice));
  if (countMax=nonRiconosciuto) {
    Serial.println("Segno non riconosciuto");
    return;
  }
  if (countMax=carta){
    Serial.println("Carta!");
    return;
  }
  if (countMax=sasso){
    Serial.println("Sasso!");
    return;
  }
  if (countMax=forbice){
    Serial.println("Forbice!");
  }
}

void detectLight(){
  luce_palmoMano = analogRead(palmoMano);
  luce_indice = analogRead(indice);
  luce_medio = analogRead(medio);
  luce_anulare = analogRead(anulare);
  Serial.print("palmoMano: ");
  Serial.println(luce_palmoMano);
  Serial.print("indice: ");
  Serial.println(luce_indice);
  Serial.print("medio: ");
  Serial.println(luce_medio);
  Serial.print("anulare: ");
  Serial.println(luce_anulare);

  Serial.println();
  delay(1000);
}

void showMaxLight(){
  Serial.print("Max Luce palmo mano: ");
    Serial.println(luceMax_palmoMano);
  Serial.print("Max Luce indice: ");
    Serial.println(luceMax_indice);
  Serial.print("Max Luce medio: ");
    Serial.println(luceMax_medio);
  Serial.print("Max Luce anulare: ");
    Serial.println(luceMax_anulare);
} 
