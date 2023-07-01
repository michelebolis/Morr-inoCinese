#include <Servo.h> // include la Libreria Servo.h

Servo servo_indice;// crea l'oggetto di tipo Servo, servo_indice sarà l'oggetto su cui opererai
Servo servo_medio;
Servo servo_anulare;
Servo servo_mignolo;
Servo servo_pollice;

int pos = 0;    // inizializza una variabile di tipo intero pos il cui valore sarà la posizione da impartire al servo

void setup() {
  servo_indice.attach(8); // lega l'oggetto servo_indice al pin a cui abbiamo collegato il nostro servo, in questo caso il pin 8
  servo_medio.attach(7);
  servo_anulare.attach(6);
  servo_mignolo.attach(5);
  servo_pollice.attach(4);
}

void mossa_forbice() {
  for (pos = 0; pos < 180; pos ++)
  {
    servo_indice.write(pos);
    servo_medio.write(pos);
    servo_pollice.write(pos);
    servo_anulare.write(pos);
    servo_mignolo.write(pos);
    delay(15);
  }

  for (pos = 180; pos >= 1; pos --)
  {
    servo_pollice.write(pos);
    servo_anulare.write(pos);
    servo_mignolo.write(pos);
    delay(15);
  }
}

void mossa_sasso() {
  for (pos = 0; pos < 180; pos ++) // imposta un ciclo con valori che vanno da 0 a 180, sarano i gradi di spostamento del nostro servo
  {
    servo_indice.write(pos);              // con il metodo write() passi all'oggetto myservo la posizione che deve raggiungere,
    // il servo si sposterà gradualmente dalla sua posizione 0° alla posizione 180°
    servo_medio.write(pos);
    servo_anulare.write(pos);
    servo_mignolo.write(pos);
    servo_pollice.write(pos);
    delay(15);                       // imposta un ritardo di 15 millesimi di secondo per ogni ciclo del for.
    // Più sarà alto il ritardo più il servo sarà lento.
  }

  for (pos = 180; pos >= 1; pos -= 1) // In questo caso imposta un ciclo con valori che vanno da 180 a 0
  {
    servo_indice.write(pos);
    servo_medio.write(pos);
    servo_anulare.write(pos);
    servo_mignolo.write(pos);
    servo_pollice.write(pos);
    delay(15);
  }
}
