#include "type.h"
int file_mess, sem;

int P(int sembis, int n)
{
    /*---prise de n valeur dans le semaphore a la position sembis---*/
    struct sembuf op = {sembis, -n, SEM_UNDO};
    return semop(sem, &op, 1);
}

int V(int sembis, int n)
{
    /*---rednu de n valeur dans le semaphore a la position sembis---*/
    struct sembuf op = {sembis, n, SEM_UNDO};
    return semop(sem, &op, 1);
}

void exit_erreur(){
    /*---Se lance si erreur lors des verifications---*/
    couleur(JAUNE);
    fprintf(stdout,"\t\tMecano %d s'arrete car erreur \n",getpid());
    couleur(REINIT);
    exit(EXIT_FAILURE);
}

void exit_fin(){
    /*---Se lance lorsque on recoit SIGUSR1---*/
    couleur(JAUNE);
    fprintf(stdout,"\t\tmecano %d s'arrete (sigusr1 recu)\n",getpid());
    couleur(REINIT);
    exit(EXIT_SUCCESS);
}

void mon_sigaction(int signal, void (*f)(int)){
    struct sigaction action;
    action.sa_handler = f;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(signal,&action,NULL);
}


int main(int argc, char *argv[])
{
    FILE *fich_cle;
    key_t cle;
    travail_t travail_courant;
    reponse_t fini;
    int i,ordre;
    sigset_t E;

    /*---Création du masque de tout les signaux---*/
    sigfillset(&E);
    sigdelset(&E,SIGUSR1);
    sigprocmask(SIG_BLOCK, &E, NULL);

    /*---Creation du sigaction et verification du nombre d'arguments et recuperation de ceux ci---*/
    mon_sigaction(SIGUSR1,exit_fin);
    if(argc!=2){
        couleur(JAUNE);
        fprintf(stdout,"\t\tAttention probleme dans la creation du mecano\n");
        couleur(REINIT);
        kill(getppid(),SIGUSR1);
    }
    ordre=atoi(argv[1]);


    /*---Création de la clé---*/
    fich_cle = fopen(FICHIER_CLE, "r");
    if (fich_cle == NULL)
    {
        couleur(JAUNE);
        fprintf(stdout, "\t\tLancement client impossible\n");
        couleur(REINIT);
        exit(-1);
    }
    cle = ftok(FICHIER_CLE, 'a');
    if (cle == -1)
    {
        couleur(JAUNE);
        fprintf(stdout, "\t\tPb creation cle\n");
        couleur(REINIT);
        exit(-1);
    }


    
    /* Recuperation file de message et ensemble de semaphore et segment de memoire*/
    file_mess = msgget(cle, 0);
    if (errno==EACCES){
        couleur(JAUNE);
        fprintf(stdout,"\t\tProcessus n'as pas les droits\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==ENOMEM){
        couleur(JAUNE);
        fprintf(stdout,"\t\tPas assez le mémoire\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==ENOSPC){
        couleur(JAUNE);
        fprintf(stdout,"\t\tTrop de file\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    sem = semget(cle, 5, 0);
    if (errno==EACCES){
        couleur(JAUNE);
        fprintf(stdout,"\t\tProcessus n'as pas les droits (semaphores)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EIDRM){
        couleur(JAUNE);
        fprintf(stdout,"\t\tIDRM (semaphores)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==ERANGE){
        couleur(JAUNE);
        fprintf(stdout,"\t\tERANGE (sempahores)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EFAULT){
        couleur(JAUNE);
        fprintf(stdout,"\t\tEFAULT (sempahores)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EINVAL){
        couleur(JAUNE);
        fprintf(stdout,"\t\tEINVAL (sempahores)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    for (;;)
    {



        /*---Attente d'un message des chefs pour travailler---*/
        msgrcv(file_mess, &travail_courant, sizeof(travail_t), ASK, 0);
        fini.type = travail_courant.num_chef + 2;
        fini.num_mecano = ordre;


        /*---Verification des outils pour pouvoir travailler---*/
        for (i = 0; i < 4; i++)
        {
            P(i, travail_courant.outils[i]);
        }
        


        /*---On a tout les outils, on vas travailler pendant travail_courant.duree secondes ---*/
        couleur(JAUNE);
        fprintf(stdout,"\t\tMecano n°%d commence a travailler pendant %d secondes \n",ordre,travail_courant.duree);
        couleur(REINIT);
        sleep(travail_courant.duree);
        couleur(JAUNE);
        fprintf(stdout,"\t\tMecano n°%d a fini de travailler \n",ordre);
        couleur(REINIT);

        for (i = 0; i < 4; i++)
        {
            V(i, travail_courant.outils[i]);
        }


        /*---Envoi que le travail est fini--*/
        msgsnd(file_mess, &fini, sizeof(reponse_t)-sizeof(fini.type), IPC_NOWAIT);
    }
}