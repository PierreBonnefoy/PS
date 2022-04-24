#ifndef TYPE_H
#define TYPE_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define FICHIER_CLE "cle.serv"

typedef struct{
    int nb_clients;
    int id_clients[50];
    int nb_total;
}chef;

typedef struct
{
    long type;
    int num_chef;
    int duree;
    int outils[4];
}travail_t;

typedef struct
{
    long type;
    int num_mecano;
}reponse_t;

#define ASK 1

/* Couleurs dans xterm                                     */
#define couleur(param) printf("\033[%sm",param)

#define NOIR  "30"
#define ROUGE "31"
#define VERT  "32"
#define JAUNE "33"
#define BLEU  "34"
#define CYAN  "36"
#define BLANC "37"
#define REINIT "0"

#endif