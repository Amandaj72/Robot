#include <Arduino.h>
#include <Servo.h>
#include <PS2X_lib.h>

// Manette PS2
PS2X ps2x;
#define PS2_DAT 8
#define PS2_CMD 9
#define PS2_SEL 10
#define PS2_CLK 11

// Servomoteur
Servo myServo;
const int pinServo = 3;

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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);

  // Servomoteur
  myServo.attach(pinServo);

  // Moteurs
  pinMode(E1, OUTPUT);
  pinMode(E2, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);

  // Manette PS2
  // Paramètres : clock, command, attention, data, pressures, rumble
  int error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);
  
  if (error == 0) {
    Serial.println("Connexion manette OK");
  } else {
    Serial.println("Problèmes connexion manette");
    // pour avoir plus de détails par erreur, regarder code à la fin de cet article :
    // www.hackster.io/igorF2/arduino-robot-with-ps2-controller-playstation-2-joystick-85bddc
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  // Blocs : "Si entrée manette PS2" -> "Modifier la vitesse courante des moteurs"
  gererMoteurs();

  // Blocs : "Changement de direction" et "Joystick > Servo_direction et Joystick < zone_morte"
  gererDirection();

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
  // Choix : axe x du joystick droit pour la direction
  int joyX = ps2x.Analog(PSS_RX);

  if (abs(joyX - 128) > 15) { // zone morte
    int servoAngle = map(joyX, 0, 255, 0, 180); // mappage comme avant
    myServo.write(servoAngle);
  } else {
    myServo.write(90); // position du milieu par défaut
  }
}

void tirerNerf() {
  if (ps2x.Button(PSB_SQUARE)) { // changer par le bouton souhaité
    Serial.println("Nerf tiré");
    // Mettre l'implémentation ici
  }
}