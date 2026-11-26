#include <Arduino.h>
#include <Servo.h>
#include <PS2X_lib.h>

// Manette PS2
PS2X ps2x;
#define PS2_DATA A1 //bleu
#define PS2_CMD A2 //vert
#define PS2_ATT A3 //rouge
#define PS2_CLK A4 //marron
//jaune gnd
//orange 3v

// Servomoteur
Servo Servo_dicretion;
const int pinServo_direction = 13;

Servo Servo_Z;
const int pinServo_Z = 12;

Servo Servo_angle;
const int pinServo_angle = 11;

Servo Servo_tire;
const int pinServo_tire = 10;

// Moteurs
const int M1 = 4;
const int E1   = 5;
const int E2   = 6;
const int M2 = 7;

int currentSpeed = 0;

// put function declarations here:
void gererMoteurs();
void gererDirection();
void tirerNerf();
void gererZ();
void gererangle();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);

  // Servomoteur
  Servo_dicretion.attach(pinServo_direction);
  Servo_dicretion.write(90); // position du milieu par défaut

  Servo_Z.attach(pinServo_Z);
  Servo_Z.write(90);

  Servo_angle.attach(pinServo_angle);
  Servo_angle.write(90); // position du milieu par défaut

  // Moteurs
  pinMode(E1, OUTPUT);
  pinMode(E2, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);

  // Manette PS2
  // Paramètres : clock, command, attention, data, pressures, rumble
  int error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_ATT, PS2_DATA, false, false);
  
  if (error == 0) {
    Serial.println("Connexion manette OK");
  } else {
    Serial.println("Problèmes connexion manette");
    // pour avoir plus de détails par erreur, regarder code à la fin de cet article :
    // www.hackster.io/igorF2/arduino-robot-with-ps2-controller-playstation-2-joystick-85bddc
  }
}

void loop() {
  // Blocs : "Si entrée manette PS2" -> "Modifier la vitesse courante des moteurs"
  gererMoteurs();

  // Blocs : "Changement de direction" et "Joystick > Servo_direction et Joystick < zone_morte"
  gererDirection();

  gererZ();

  gererangle();

  // Bloc : "Tir manuel"
  tirerNerf();

  delay(20);
}

// put function definitions here:
void gererMoteurs() {
  // Lecture de la manette PS2
  ps2x.read_gamepad(false, false);

  // Choix : axe y du joystick gauche pour avancer/reculer
  int joyY = ps2x.Analog(PSS_LY);
  
  // 128 environ -> position de repos (car 0 (haut) = avancer, et 255 (bas) = reculer donc milieu du joystick ~ 128)
  if (abs(joyY - 128) > 10) { // si on est pas en position de repos du coup
    if (joyY < 128) { // si on est vers le haut
      currentSpeed = map(joyY, 128, 0, 0, 255); // on change d'échelle, [128, 0] devient [0, 255] et on calcule joyY dans la nouvelle base
      digitalWrite(M1, HIGH);
      digitalWrite(M2, HIGH);
    } else { // si on est vers le bas du coup
      currentSpeed = map(joyY, 128, 255, 0, 255); // même principe
      digitalWrite(M1, LOW);
      digitalWrite(M2, LOW);
    }

    // On applique la vitesse sur les moteurs
    analogWrite(E1, currentSpeed);
    analogWrite(E2, currentSpeed);

  } else { // si on est en position de repos
    analogWrite(E1, 0);
    analogWrite(E2, 0);
  }
}

void gererDirection() {
  // Choix : axe x du joystick gauche pour la direction
  int joyX = ps2x.Analog(PSS_LX);

  if (abs(joyX - 128) > 15) { // zone morte
    int servoAngle = map(joyX, 0, 255, 45, 135); 
    Servo_dicretion.write(servoAngle);
  } else {
    Servo_dicretion.write(90); // position du milieu par défaut
  }
}

void gererZ() {
  // Choix : axe x du joystick gauche pour la direction
  int joyX = ps2x.Analog(PSS_LX);

  if (abs(joyX - 128) > 15) { // zone morte
    int servoAngle = map(joyX, 0, 255, 45, 135); 
    Servo_dicretion.write(servoAngle);
  } else {
    Servo_dicretion.write(90); // position du milieu par défaut
  }
}

void gererangle() {
  // L'axe vertical commande progressivement l'angle; le relâchement garde la position.
  static int positionAngle = 90;
  static unsigned long dernierPas = 0;

  int joyY = ps2x.Analog(PSS_RY);
  int deviation = abs(joyY - 128);

  if (deviation <= 15) {
    return;
  }

  unsigned long intervalle = map(constrain(deviation, 16, 127), 16, 127, 120, 20);
  unsigned long maintenant = millis();
  if (maintenant - dernierPas < intervalle) {
    return;
  }

  dernierPas = maintenant;
  int direction = joyY < 128 ? 1 : -1;
  int nouvellePosition = constrain(positionAngle + direction, 50, 150);
  if (nouvellePosition != positionAngle) {
    positionAngle = nouvellePosition;
    Servo_angle.write(positionAngle);
  }
}

void tirerNerf() {
  if (ps2x.Button(PSB_SQUARE)) { // changer par le bouton souhaité
    Serial.println("Nerf tiré");
    // Mettre l'implémentation ici
  }
}