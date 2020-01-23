/******************************************************
 * Projet PRS - Sujet 4 - Carrefour (pas le magasin)
 * Sacha Lesueur, Romain Rousseaux, Guillaume Carlier
 * Fichier car_generation.c
 * Génère des voitures aléatoirement et les dépose dans une BAL
 ******************************************************/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

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

#define CHECK(sts,msg) if ((sts) == -1 ) { perror(msg);exit(-1);}

int main(int argc,char* argv[]) {
    srand(time(NULL));
    int id = 0;
    
    int idBal = msgget(ftok("/tmp/",5),IPC_CREAT | 0666);

    while(1) {
        // Initialisation des données du message et de la voiture
        message.mcontent.id = id++;
        message.mcontent.route = rand() % 4 + 1;
        message.mtype = 1;
        
        // Initialisation des coordonnées
        message.mcontent.x = 0;
        message.mcontent.y = 0;

        // La voiture ne peut pas faire un demi tour
        do {
            message.mcontent.direction = rand() % 4 + 1;
        } while (message.mcontent.route + 2 == message.mcontent.direction || message.mcontent.route - 2 == message.mcontent.direction);

        // Pour une simplicité d'affichage, on ne dépasse pas 9 comme id
        if (id == 9)
            id = 0;

        printf("----Origine : %d----\n", message.mcontent.route);
        printf("----Destination : %d----\n", message.mcontent.direction);
        
        CHECK(msgsnd(idBal,&message,sizeof(message),0), "Erreur lors de l'envoi du message");
        // Temporisation entre la génération de voitures avec un minimum de 3 secondes
        sleep(rand() % 6 + 3);
    }
    return 0;
}
