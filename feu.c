/******************************************************
 * Projet PRS - Sujet 4 - Carrefour (pas le magasin)
 * Sacha Lesueur, Romain Rousseaux, Guillaume Carlier
 * Fichier feu.c
 * Change la valeur du feu et l'écrit dans une mémoire partagée 
 * pour la persistence de la donnée
 ******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>

// Définition des tempos
#define TPS_FEU_ROUGE 6
#define TPS_FEU_VERT 12
#define TPS_ALL_ROUGE 2

//Création de la mémoire partagée en global
int SH_KEY, shmid;
char * boolean = "R";
struct shmid_ds shmid_struct;

void * thFeu() {
    while (1) {
	// Le feu passe au vert	
        shmctl(shmid, SHM_LOCK, &shmid_struct);
        strcpy(boolean, "V");
        shmctl(shmid, SHM_UNLOCK, &shmid_struct);
        sleep(TPS_FEU_VERT);

	// Tous les feux sont rouges
	shmctl(shmid, SHM_LOCK, &shmid_struct);
        strcpy(boolean, "O");
        shmctl(shmid, SHM_UNLOCK, &shmid_struct);
	sleep(TPS_ALL_ROUGE);
        
	// La première route passe au rouge, l'autre au vert
	shmctl(shmid, SHM_LOCK, &shmid_struct);
        strcpy(boolean, "R");
        shmctl(shmid, SHM_UNLOCK, &shmid_struct);
        sleep(TPS_FEU_ROUGE);

	// Tous les feux sont rouges
	shmctl(shmid, SHM_LOCK, &shmid_struct);
        strcpy(boolean, "O");
        shmctl(shmid, SHM_UNLOCK, &shmid_struct);
	sleep(TPS_ALL_ROUGE);
    }
}


int main() {
    SH_KEY = ftok("/etc", 40);
    shmid = shmget(SH_KEY, sizeof(char), IPC_CREAT | 0666);

    boolean = (char *)shmat(shmid, NULL, 0);

    pthread_t feu;

    // Création du thread pour les feux
    pthread_create(&feu, NULL, &thFeu, NULL);

    pthread_join(feu, NULL);

    shmctl(shmid, IPC_RMID, NULL);
}
