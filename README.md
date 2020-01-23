# Carrefour2i
Remerciements à Guillaume Carlier, Romain Rousseaux et Sacha Lesueur pour ce projet :) 
### Sujet 4 - PRS LA1

A l’aide des outils vus en cours de Programmation Système, nous pouvons modéliser un tel système de carrefour en croix et en reproduire le fonctionnement.

Une voiture est représentée par un processus et est composée de :
- Une position sur le carrefour
- Une direction (tout droit, gauche ou droite)
- Une provenance (haut, bas, gauche, droite)
Les feux rouges (couleurs rouge et vert seulement, orange si vous voulez), fonctionnant par paire, seront représentés par un processus qui :
- Enverra un signal quand il passe rouge/vert pour que les voitures puissent passer.
L’apparition de voiture pourra être soit aléatoire, soit provoquée par l’utilisateur via un signal.
Il faudra prendre en compte le code de la route et ne pas tourner si une voiture va tout droit pour ne pas lui foncer dedans.

Une modélisation graphique sommaire du carrefour peut être envisagée pour une compréhension plus aisée du fonctionnement.

### Solution

Utilisation de :
- Threads
- Mutex
- Boites aux lettres
- Mémoire partagée

### Fonctionnement

- Compiler le projet avec `make`
- Ouvrir 3 terminaux (2 seront en arrière plan) 
- Lancer le programme feu.c `./feu`
- Lancer le programme display.c `./display`. Ce sera notre affichage principal donc il faut garder ce terminal à portée de main.
- Lancer le programme car_generation.c `./car_generation`

Les voitures sont générées automatiquement avec un délai entre 3 et 9 secondes aléatoire.
On pourra ainsi voir les voitures, identifiées par un chiffre de 0 à 8 bouger sur le carrefour.

### Notations

Un 'V' représente un feu vert. Un 'R' représente un feu rouge.
Étant français, nous roulons à droite, les deux sont donc situés sur la droite de la route concernée.
```
...#  #...
...#  #||.
.->#  #\/.
###V  R###


###R  V###
..^#  #<-.
..|#  #...
...#  #...
```
Chaque chiffre représente une voiture et donc ses déplacements.
