#include "type.h"
int sem,id_segm;
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

void exit_erreur(){
    /*---Se lance si erreur lors des verifications---*/
    couleur(BLEU);
    fprintf(stdout,"\t\t\tClient %d s'arrete car erreur \n",getpid());
    couleur(REINIT);
    exit(EXIT_FAILURE);
}

void exit_fin(){
    /*---Se lance lorsque on recoit SIGUSR1---*/
    couleur(BLEU);
    fprintf(stdout,"\t\t\tClient %d s'arrete (sigusr1 recu)\n",
    getpid());couleur(REINIT);
    exit(EXIT_SUCCESS);
}

void mon_sigaction(int signal, void (*f)(int)){
    struct sigaction action;
    action.sa_handler = f;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(signal,&action,NULL);
}

void commande_fini(){
    /*---Lancement lorsque la client recupere sa voiture---*/
    couleur(BLEU);
    fprintf(stdout,"\t\t\tJe suis : %d ,ma commande est fini je part, Au revoir\n",getpid());
    couleur(REINIT);
    exit(EXIT_SUCCESS);
}

int main(int argc,char* argv[]){
    FILE* fich_cle;
    key_t cle;
    int i,min,indexmin=0;
    sigset_t E;

    /*---Création du masque de tout les signaux---*/
    sigfillset(&E);
    sigdelset(&E,SIGUSR2);
    sigdelset(&E,SIGUSR1);
    sigprocmask(SIG_BLOCK, &E, NULL);

    /*---Definition des sigaction---*/
    mon_sigaction(SIGUSR1,exit_fin);
    mon_sigaction(SIGUSR2,commande_fini);


    /*---Création de la clé---*/
    fich_cle = fopen(FICHIER_CLE,"r");
    srand(time(NULL));
    if (fich_cle==NULL){
        couleur(BLEU);
        fprintf(stdout,"\t\t\tLancement client impossible\n");
        couleur(REINIT);
        exit(-1);
    }
    cle = ftok(FICHIER_CLE,'a');
    if (cle==-1){
        fprintf(stdout,"\t\t\tPb creation cle\n");
        exit(-1);
    }


    /*---allocation de la taille du segment de memoire partagé---*/
    planning=malloc(50*sizeof(chef));

    
    /*---Verification du nombre d'argument---*/
    srand(time(NULL));
    if(argc!=4){
        couleur(BLEU);
        fprintf(stdout,"\t\t\tAttention probleme dans la creation du clients\n");
        couleur(REINIT);
        kill(getppid(),SIGUSR1);
    }

    /*---Creéation et initialisation du segment de memoire partagé et du semaphore---*/
    id_segm = shmget(cle,atoi(argv[1])*sizeof(chef),0);
    if (errno==EACCES){
        couleur(BLEU);
        fprintf(stdout,"\t\t\tProcessus n'as pas les droits (Segment de memoire partagé)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EIDRM){
        couleur(BLEU);
        fprintf(stdout,"\t\t\tIDRM (Segment de memoire partagé)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==ERANGE){
        couleur(BLEU);
        fprintf(stdout,"\t\t\tERANGE (Segment de memoire partagé)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EFAULT){
        couleur(BLEU);
        fprintf(stdout,"\t\t\tEFAULT (Segment de memoire partagé)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EINVAL){
        couleur(BLEU);
        fprintf(stdout,"\t\t\tEINVAL (Segment de memoire partagé)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }

    sem=semget(cle,5,0);
    if (errno==EACCES){
        couleur(BLEU);
        fprintf(stdout,"\t\t\tProcessus n'as pas les droits (semaphores)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EIDRM){
        couleur(BLEU);
        fprintf(stdout,"\t\t\tIDRM (semaphores)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==ERANGE){
        couleur(BLEU);
        fprintf(stdout,"\t\t\tERANGE (sempahores)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EFAULT){
        couleur(BLEU);
        fprintf(stdout,"\t\t\tEFAULT (sempahores)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EINVAL){
        couleur(BLEU);
        fprintf(stdout,"\t\t\tEINVAL (sempahores)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    P(4,1);
    planning=shmat(id_segm,NULL,0);


    /*---On cherche on la file d'attente est la plus faible, et on recupere le chef dans indexmin---*/
    min=planning[0].nb_clients;
    for(i=0;i<atoi(argv[1]);i++){
        if(min>planning[i].nb_clients){
            min=planning[i].nb_clients;
            indexmin=i;
        }
        
    }


    /*---On augmente de 1 le nombre de personne dans la file d'attente, et on y rajoute le client---*/
    planning[indexmin].nb_clients+=1;
    planning[indexmin].id_clients[planning[indexmin].nb_clients-1]=getpid();
    shmdt(planning);
    V(4,1);


    /*---On attend la fin de la reparation---*/
    pause();
    exit(0);
}