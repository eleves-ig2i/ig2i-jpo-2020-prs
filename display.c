/******************************************************
 * Projet PRS - Sujet 4 - Carrefour (pas le magasin)
 * Sacha Lesueur, Romain Rousseaux, Guillaume Carlier
 * Fichier display.c
 * Gère l'affichage de la grille et les déplacements des voitures
 ******************************************************/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/shm.h>
#include <pthread.h>

// Mutex de sécurité lors de l'écriture dans la grille
pthread_mutex_t crossroadEdition = PTHREAD_MUTEX_INITIALIZER;

// Initialisation des variables globales
int TAILLE_ROUTE = 10;
char crossroad[10][10];
char * state = "";

#define CHECK(sts,msg) if ((sts) == -1 ) { perror(msg);exit(-1);}

/**
 * Struct de la voiture
 * \struct car_t
 * \brief Contient une id, une route d'origine, une route de destination et la position sur la grille
 */
typedef struct content {
    int id;
    int route;
    int direction;
    int x;
    int y;
} car_t ;

struct msgbuf {
    long mtype;
    car_t mcontent;
} message;

/* 
 * Initialisation de la grille selon le modèle suivant :
 * ...#  #...
 * ...#  #...
 * ...#  #...
 * ####  ####
 *
 *
 * ####  ####
 * ...#  #...
 * ...#  #...
 * ...#  #...
 * Chaque route possède 2 lignes / colonnes pour permettre aux voitures de se croiser
 */
void init() {
    pthread_mutex_lock(&crossroadEdition);
    // RAZ du tableau
    for (int i = 0; i < TAILLE_ROUTE; i++) {
        for (int j = 0; j < TAILLE_ROUTE; j++){
            crossroad[i][j] = 'a';
        }
    }

    // Init des murs
    for (int j = 0; j < TAILLE_ROUTE; j++) {
        for (int i = 0; i < TAILLE_ROUTE; i++){
            // Pour boucher les coins
            if ((i < 3 && j < 3) || (i < 3 && j > 6) || (i > 6 && j < 3) || (i > 6 && j > 6)) {
                crossroad[i][j] = '.';
            }
            // Pour créer les murs
            else if ((3 == i && j < 4) || (3 == i && j > 5) && (3 == j && i < 4) || (3 == j && i > 5) || (6 == i && j < 3) || (3 == j && i < 3) || (6 == j && i < 3) || (3 == i && j >= 6) || (6 == i && j >= 6) || (6 == j && i >= 6))
                crossroad[i][j] = '#';
	    // Pour les espaces vides pour la route	
            else
                crossroad[i][j] = ' ';
        }
    }
    pthread_mutex_unlock(&crossroadEdition);
}

/**
 * Affichage de la grille
 * \param state Chaine ne contenant qu'un caractère avec l'état du feu
 * Pour éviter les conflits, on vient lock l'édition de la route à chaque fois 
 */
void afficher(char * state) {
    switch (state[0]) {
        case 'V':
	    	pthread_mutex_lock(&crossroadEdition);
            crossroad[3][3] = 'R';
            crossroad[6][6] = 'R';
            crossroad[3][6] = 'V';
            crossroad[6][3] = 'V';
	    	pthread_mutex_unlock(&crossroadEdition);
            break;
        case 'R':
	    	pthread_mutex_lock(&crossroadEdition);
            crossroad[3][3] = 'V';
            crossroad[6][6] = 'V';
            crossroad[3][6] = 'R';
            crossroad[6][3] = 'R';
	    	pthread_mutex_unlock(&crossroadEdition);
            break;
        case 'O':
	    	pthread_mutex_lock(&crossroadEdition);
            crossroad[3][3] = 'R';
            crossroad[6][6] = 'R';
            crossroad[3][6] = 'R';
            crossroad[6][3] = 'R';
	    	pthread_mutex_unlock(&crossroadEdition);
            break;
        default:
            break;
    }	

    // Affichage réel de la route	
    for (int i = 0; i < TAILLE_ROUTE; i++) {
        for (int j = 0; j < TAILLE_ROUTE; j++){
            printf("%c", crossroad[i][j]);
        }
        puts("");
    }
}

/* 
 * Fonction de gestion des déplacements des voitures
 * Init la position de la voiture
 * Déplacement dans la direction voulu selon la route
 * \param voitureParam Pointeur vers la structure car_t reçue depuis la BAL
 */
void * thCarsMovement(car_t * voitureParam) {
    // Nettoyage de la première ligne de la grille
    crossroad[0][0] = '.';crossroad[0][1] = '.';crossroad[0][2] = '.'; crossroad[0][3] = '#';crossroad[0][4] = ' ';crossroad[0][5] = ' ';crossroad[0][6] = '#';crossroad[0][7] = '.';crossroad[0][8] = '.';crossroad[0][9] = '.';
    
    // On copie la structure pour que d'autres threads puissent fonctionner en parallèle	
    car_t voiture = *voitureParam;
    char val;
    sprintf(&val, "%d", voiture.id);
    short aTourne = 0;

    // Placement initial de la voiture sur la grille	
    switch (voiture.route) {
    case 1:
        crossroad[0][4] = val;
        voiture.x = 0; voiture.y = 4;
        break;
    case 2:
        crossroad[4][9] = val;
        voiture.x = 4; voiture.y = 9;        
        break;
    case 3:
        crossroad[9][5] = val;
        voiture.x = 9; voiture.y = 5;
        break;
    case 4:
        crossroad[5][0] = val;
        voiture.x = 5; voiture.y = 0;
        break;
    default:
        break;
    }

    // Partie déplacements + changements de direction	
    while (1) {
	// Lock de l'édition pour ne pas lire / écrire en même temps que d'autres threads    
	pthread_mutex_lock(&crossroadEdition);
	
	// Déplacement de la voiture selon la route sur laquelle elle se situe    
        switch (voiture.route) {
			case 1:
				// Si le feu est pour nous ou qu'on l'on a tourné ou que l'on a pas atteint le feu on avance
				// Et si il n'y a pas de voiture devant, on avance
				if ((strcmp(state, "R") == 0 || aTourne || voiture.x < 3) && crossroad[voiture.x+1][voiture.y] == ' ') {
					// On efface l'ancienne position de la voiture pour lui donner sa nouvelle
					crossroad[voiture.x][voiture.y] = ' ';
					voiture.x++;
					crossroad[voiture.x][voiture.y] = val;
				// Si la voiture a atteint le bout de cette route, on l'efface et on termlne le thread après avoir libéré la ressource	
				} else if(voiture.x == 9) {
					crossroad[voiture.x][voiture.y] = ' ';
					pthread_mutex_unlock(&crossroadEdition);
					pthread_exit(EXIT_SUCCESS);
				}
				break;
			case 2:
				if ((strcmp(state, "V") == 0 || aTourne || voiture.y > 6) && crossroad[voiture.x][voiture.y-1] == ' ') {
					crossroad[voiture.x][voiture.y] = ' ';
					voiture.y--;
					crossroad[voiture.x][voiture.y] = val;
				} else if (voiture.y == 0) {
					crossroad[voiture.x][voiture.y] = ' ';
					pthread_mutex_unlock(&crossroadEdition);
					pthread_exit(EXIT_SUCCESS);
				}
				break;
			case 3:
				if ((strcmp(state, "R") == 0 || aTourne || voiture.x > 6) && crossroad[voiture.x-1][voiture.y] == ' ') {
					crossroad[voiture.x][voiture.y] = ' ';
					voiture.x--;
					crossroad[voiture.x][voiture.y] = val;
				} else if (voiture.x == 0) {
					crossroad[voiture.x][voiture.y] = ' ';
					pthread_mutex_unlock(&crossroadEdition);
					pthread_exit(EXIT_SUCCESS);
				}
				break;
			case 4:
				if ((strcmp(state, "V") == 0 || aTourne || voiture.y < 3) && crossroad[voiture.x][voiture.y+1] == ' ') {
					crossroad[voiture.x][voiture.y] = ' ';
					voiture.y++;
					crossroad[voiture.x][voiture.y] = val;
				} else if (voiture.y == 9) {
				crossroad[voiture.x][voiture.y] = ' ';
					pthread_mutex_unlock(&crossroadEdition);
					pthread_exit(EXIT_SUCCESS);
				}
				break;
			default:
				puts("Une voiture fantôme ?");
				break;
        }

		// On libère la ressource car on ne modifiera / regardera plus la grille    
		pthread_mutex_unlock(&crossroadEdition);

		// Partie changements de direction
		// Pour changer de direction, on regarde la route de destination et on vient placer la voiture sur celle ci
		// On modifie voiture.route pour la route de destination
		// Cette modification ne se fait que lorsque la voiture se trouve au croisement de ses routes d'origine et de destination
		// On initialise un boolean à 1 si la voiture a tourné pour qu'elle ignore les feux (elle n'est plus affectée)
		switch (voiture.direction) {
			case 1:
                switch (voiture.route) {
                    case 1:
                        if (voiture.x == 4 && voiture.y == 4) {
                            aTourne = 1;
                            voiture.route = 1;
                        }
                        break;
                    case 2:
                        if (voiture.x == 4 && voiture.y == 4) {
                            aTourne = 1;
                            voiture.route = 1;
                        }
                        break;
                    case 4:
                        if (voiture.x == 5 && voiture.y == 4) {
                            aTourne = 1;
                            voiture.route = 1;
                        }
                        break;
                    default:
                        puts("Erreur");
                        break;
                }
                break;
            case 2:
                switch (voiture.route) {
                    case 1:
                        if (voiture.x == 4 && voiture.y == 4){
                            aTourne = 1;
                            voiture.route = 2;
						}
                        break;
                    case 2:
                        if (voiture.x == 4 && voiture.y == 5) {
                            aTourne = 1;
                            voiture.route = 2;
                        }
                        break;
                    case 3:
                        if (voiture.x == 4 && voiture.y == 5) {
                            aTourne = 1;
                            voiture.route = 2;
                        }
                        break;
                    default:
                        puts("Erreur");
                        break;
                }
                break;
            case 3:
                switch (voiture.route) {
                    case 2:
                        if (voiture.x == 4 && voiture.y == 5) {
                            aTourne = 1;
                            voiture.route = 3;
                        }
                        break;
                    case 3:
                        if (voiture.x == 5 && voiture.y == 5) {
                            aTourne = 1;
                            voiture.route = 3;
                        }
                        break;
                    case 4:
                        if (voiture.x == 5 && voiture.y == 5) {
                            aTourne = 1;
                            voiture.route = 3;
                        }
                        break;
                    default:
                        puts("Erreur");
                        break;
                }
                break;
            case 4:
                switch (voiture.route) {
                    case 1:
                        if (voiture.x == 5 && voiture.y == 4) {
                            aTourne = 1;
                            voiture.route = 4;
                        }
                        break;
                    case 3:
                        if (voiture.x == 5 && voiture.y == 5) {
                            aTourne = 1;
                            voiture.route = 4;
                        }
                        break;
                    case 4:
                        if (voiture.x == 5 && voiture.y == 4) {
                            aTourne = 1;
                            voiture.route = 4;
                        }
                        break;
                    default:
                        puts("Erreur");
                        break;
                }
                break;
            default:
                break;
        }
        sleep(1);
    }
    pthread_exit(EXIT_SUCCESS);
}

/**
 * Thread de lecture de la BAL
 * Chaque fois qu'un message est envoyé, une nouvelle voiture est créée et donc on crée le thread correspondant
 */
void * thLectureBal() {
    int idBal = msgget(ftok("/tmp/",5),IPC_CREAT | 0666), cptCars = 0;

    pthread_t thCars[200];

    while(1) {
        msgrcv(idBal, &message, sizeof(message), 1, 0);
        pthread_create(&thCars[cptCars++], NULL, (void *)thCarsMovement, &message.mcontent);
    }
}

int main(int argc, char *argv[]) {
    struct timespec tempo;
    tempo.tv_sec = 0;
    tempo.tv_nsec = 300000000;
    key_t key = ftok("/etc", 40);
    pthread_t lectureBal;
    int shmid = shmget(key, sizeof(char), SHM_R);
    init();
    pthread_create(&lectureBal, NULL, &thLectureBal, NULL);

    while(1) {
		// Lecture de l'état du feu dans la mémoire partagée
        state = (char *) shmat(shmid, NULL, 0);

        afficher(state);

	    nanosleep(&tempo, NULL);

	// Nettoyage de l'affichage
	system("clear");
        puts("\n\n\n\n");
    }

    CHECK(shmctl(shmid, IPC_RMID, NULL), "Erreur lors de la suppression");

    return 0;
}
