#include "type.h"
int file_mess,sem,id_segm, ordre;
chef *planning;

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

void remonte(){
    /*---Lorsque client recupere sa voiture, on passe au client suivant et on change la file d'attente---*/
    int i=0,val;
    while(planning[ordre].id_clients[i]!=0){
        val=planning[ordre].id_clients[i+1];
        planning[ordre].id_clients[i]=val;
        i++;
    }
}

void exit_erreur(){
    /*---Se lance si erreur lors des verifications---*/
    couleur(VERT);
    fprintf(stdout,"\tChef %d s'arrete car erreur \n",getpid());
    couleur(REINIT);
    exit(EXIT_FAILURE);
}

void exit_fin(){
    /*---Se lance lorsque on recoit SIGUSR1---*/
    couleur(VERT);
    fprintf(stdout,"\tChef %d s'arrete (sigusr1 recu)\n",getpid());
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

int main(int argc, char* argv[]){
    FILE* fich_cle;
    key_t cle;
    int i;
    travail_t travail_courant;
    reponse_t travail_fini;
    sigset_t E;

    /*---Création du masque de tout les signaux---*/
    sigfillset(&E);
    sigdelset(&E,SIGUSR1);
    sigprocmask(SIG_BLOCK, &E, NULL);

    /*---Création du sigaction , verification du nombre d'argument, et recuperation du numero de chef---*/
    mon_sigaction(SIGUSR1,exit_fin);
    planning=malloc(50*sizeof(chef));
    if(argc!=6){
        couleur(VERT);
        fprintf(stdout,"\tAttention probleme dans la creation du chef nbre arguments = %d\n",argc);
        couleur(REINIT);
        kill(getppid(),SIGUSR1);
    }
    ordre=atoi(argv[1]);


    /*---Création de la clé---*/
    fich_cle = fopen(FICHIER_CLE,"r");
    srand(time(NULL));
    if (fich_cle==NULL){
        couleur(VERT);
        fprintf(stdout,"\tLancement client impossible\n");
        couleur(REINIT);
        exit(-1);
    }
    cle = ftok(FICHIER_CLE,'a');
    if (cle==-1){
        couleur(VERT);
        fprintf(stdout,"\tPb creation cle\n");
        couleur(REINIT);
        exit(-1);
    }
    

    /* Recuperation file de message et ensemble de semaphore et segment de memoire*/
    file_mess=msgget(cle,0);
    if (errno==EACCES){
        couleur(VERT);
        fprintf(stdout,"\tProcessus n'as pas les droits\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==ENOMEM){
        couleur(VERT);
        fprintf(stdout,"\tPas assez le mémoire\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==ENOSPC){
        couleur(VERT);
        fprintf(stdout,"\tTrop de file\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }


    sem=semget(cle,5, 0);
    if (errno==EACCES){
        couleur(VERT);
        fprintf(stdout,"\tProcessus n'as pas les droits\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==ENOMEM){
        couleur(VERT);
        fprintf(stdout,"\tPas assez le mémoire\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==ENOSPC){
        couleur(VERT);
        fprintf(stdout,"\tTrop de file\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EINVAL){
        couleur(VERT);
        fprintf(stdout,"\tNombre d'outils invalide\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }

    
    id_segm=shmget(cle,planning[0].nb_total*sizeof(chef),0);
    if (errno==EACCES){
        couleur(VERT);
        fprintf(stdout,"\tProcessus n'as pas les droits (Segment de memoire partagé)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EIDRM){
        couleur(VERT);
        fprintf(stdout,"\tIDRM (Segment de memoire partagé)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==ERANGE){
        couleur(VERT);
        fprintf(stdout,"\tERANGE (Segment de memoire partagé)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EFAULT){
        couleur(VERT);
        fprintf(stdout,"\tEFAULT (Segment de memoire partagé)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EINVAL){
        couleur(VERT);
        fprintf(stdout,"\tEINVAL (Segment de memoire partagé)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    P(4,1);
    planning=shmat(id_segm,NULL,0);
    V(4,1);


    /*---Gestion des clients infini ---*/
    for(;;){
        P(4,1);
        if(planning[ordre].nb_clients>0){
            V(4,1);


            /*---Création des données pour la file de message---*/
            travail_courant.num_chef=ordre;
            travail_courant.type=ASK;
            travail_courant.duree=rand()%(1 - 10) + 1;
            for(i=0;i<4;i++){
                travail_courant.outils[i]=rand()%(1 - (atoi(argv[i+2]))/3)+1;
            }


            /*---Envoi d'un message aux mecano, avec les infos ci dessus---*/
            couleur(VERT);
            fprintf(stdout,"\tEnvoi d'une requete de travail par le chef n°%d !!!!\n",ordre);
            couleur(REINIT);
            msgsnd(file_mess,&travail_courant,sizeof(travail_t)-sizeof(travail_courant.type), 0);


            /*---Attente de la reception de la commande fini par le mecano qui s'en occupe---*/
            msgrcv(file_mess,&travail_fini,sizeof(reponse_t),ordre+2,0);
            P(4,1);
            planning[ordre].nb_clients-=1;
            kill(planning[ordre].id_clients[0],SIGUSR2);
            remonte();
        }
        V(4,1);
    }   
}