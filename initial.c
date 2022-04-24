#include "type.h"
int file_mess,sem,id_segm,*fils_pid;
chef *planning;

int P(int sembis, int n)
{
    /*---prise de n valeur dans le semaphore a la position sembis---*/
    struct sembuf op = {sembis, -n, SEM_UNDO};
    return semop(sem, &op, 1);
}

int V(int sembis, int n)
{
    /*---rendu de n valeur dans le semaphore a la position sembis---*/
    struct sembuf op = {sembis, n, SEM_UNDO};
    return semop(sem, &op, 1);
}

void usage(char* nom){
    /*---Fonction si mauvais nombre d'arguments---*/
    couleur(ROUGE); 
    fprintf(stdout," usage : <%s> <nb_chefs> <nb_mecano> <nb_1> <nb_2> <nb_3> <nb_4>\n",nom);
    fprintf(stdout," usage : <%s> <nb_chefs> <nb_mecano> <nb_1> <nb_2> <nb_3> <nb_4> <temps_intervale_clients>\n",nom);
    couleur(REINIT);
}

void ajout_pid(int n){
    /*--Execution a la création d'un processus---*/
    int i;
    for(i=0;fils_pid[i]!=0;i++){
    }
    fils_pid[i]=n;
}

void lancement_chefs(int n,int liste_outils[]){
    /*---Création et execution de n chefs---*/
    int i;
    pid_t pid;
    char *param[5];
    fils_pid=calloc(1024,sizeof(int));
    for(i=0;i<5;i++){
        param[i]=malloc(sizeof(int));
    }
    for(i=1;i<5;i++){
        sprintf(param[i],"%d",liste_outils[i-1]);
    }
    couleur(ROUGE); 
    fprintf(stdout,"(Garage) Creation des chefs :\n");
    couleur(REINIT);
    for (i = 0; i < n; i++)
    {
      pid = fork();
      if (pid == -1)
      {
        couleur(ROUGE); 
        fprintf(stdout, "Erreur des pid\n");
        couleur(REINIT);
      }
      if(pid==0){
        sprintf(param[0],"%d",i);
        execl("chef","chef",param[0],param[1],param[2],param[3],param[4],NULL);
      }
      ajout_pid(pid);
      couleur(ROUGE); 
      fprintf(stdout,"Lancement chefs\n");
      couleur(REINIT);
    }
}

void clean(){
    /*---Nettoyage des IPC et kill des processus fils---*/ 
    int i,j;
    msgctl(file_mess,IPC_RMID,NULL);
    semctl(sem,0, IPC_RMID,NULL);
    shmctl(id_segm,IPC_RMID,NULL);
    for(i=0;fils_pid[i]!=0;i++){
        kill(fils_pid[i],SIGUSR1);
    }
    P(4,1);
    for(i=0;i<planning[0].nb_total;i++){
        for(j=0;planning[i].id_clients[j]!=0;j++){
            kill(planning[i].id_clients[j],SIGUSR1);
        }
    }
    shmdt(planning);
}

void exit_erreur(){
    /*---Se lance si erreur lors des verifications---*/
    couleur(ROUGE); 
    fprintf(stdout,"Serveur s'arrete car erreur \n");
    couleur(REINIT);
    clean();
    exit(EXIT_FAILURE);
}

void exit_fin(){
    /*---Se lance lorsque reception de  SIGUSR1---*/
    couleur(ROUGE);
    fprintf(stdout,"Serveur s'arrete (sigusr1 recu)\n");
    couleur(REINIT);
    clean();
    exit(EXIT_SUCCESS);
}

void mon_sigaction(int signal, void (*f)(int)){
    struct sigaction action;
    action.sa_handler = f;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(signal,&action,NULL);
}

void lancement_mecano(int n){
    /*---Lancement et execution de n mecano---*/
    int i;
    char *param[1];
    param[0]=malloc(4*sizeof(int));
    pid_t pid;
    couleur(ROUGE);
    fprintf(stdout,"(Garage) Creation des mecano :\n");
    couleur(REINIT);
    for (i = 0; i < n; i++)
    {
      pid = fork();
      if (pid == -1)
      {
        couleur(ROUGE);
        fprintf(stdout, "Erreur des pid\n");
        couleur(REINIT);
      }
      if(pid==0){
        sprintf(param[0],"%d",i);
        execl("mecano","mecano",param[0],NULL);
      }
      couleur(ROUGE);
      fprintf(stdout,"Lancement mecano\n");
      couleur(REINIT);
      ajout_pid(pid);
    }
}

int main (int argc, char* argv[]){
    FILE* fich_cle;
    key_t cle;
    int i,nb_chefs, nb_mecano,outils[5],duree_clients=2;
    char *param[3];
    pid_t pid;
    sigset_t E;

    /*---Création du masque de tout les signaux---*/
    sigfillset(&E);
    sigdelset(&E,SIGUSR1);
    sigprocmask(SIG_BLOCK, &E, NULL);

    /*---Création du sigaction et verification du nombre d'arguments ---*/
    mon_sigaction(SIGUSR1,exit_fin);
    if(argc!=7 && argc!=8){
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    if(argc==8){
        duree_clients=atoi(argv[7]);
    }
    

    /*---Recuperation des parametres---*/
    nb_chefs = atoi(argv[1]);
    nb_mecano = atoi(argv[2]);
    planning=malloc(50*sizeof(chef));
    for (i=3;i<=6;i++){
        outils[i-3]=atoi(argv[i]);
    }
    outils[4]=1;


    /*---Creation de la cle---*/
    /*---Test si le fichier cle existe deja---*/
    fich_cle = fopen(FICHIER_CLE,"r");
    if (fich_cle==NULL){
        if (errno==ENOENT){
            fich_cle=fopen(FICHIER_CLE,"w");
            if (fich_cle==NULL){
                couleur(ROUGE);
                fprintf(stdout,"Lancement fils impossible\n");
                couleur(REINIT);
                exit(-1);
            }
        }
        else {
            couleur(ROUGE);
            fprintf(stdout,"Lancement fils impossible\n");
            couleur(REINIT);
            exit(-1);
        }
    }
    cle = ftok(FICHIER_CLE,'a');
    if (cle==-1){
        couleur(ROUGE);
        fprintf(stdout,"Pb creation cle\n");
        couleur(REINIT);
        exit(-1);
    }
    file_mess=msgget(cle,0);
    if (file_mess!=-1){
        sem = semget(cle, 1, 0);
        id_segm=shmget(cle,sizeof(planning),IPC_EXCL | 0660);
        clean();
    }


    /*---Creation file de message---*/
    file_mess=msgget(cle,IPC_CREAT | IPC_EXCL | 0660);


    /*---Vérif de la bonne création---*/
    if (errno==EACCES){
        couleur(ROUGE);
        fprintf(stdout,"Processus n'as pas les droits\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==ENOMEM){
        couleur(ROUGE);
        fprintf(stdout,"Pas assez le mémoire\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==ENOSPC){
        couleur(ROUGE);
        fprintf(stdout,"Trop de file\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }


    /* Création des sémaphors */
    sem=semget(cle,5,IPC_CREAT | IPC_EXCL | 0660);
    if (errno==EACCES){
        couleur(ROUGE);
        fprintf(stdout,"Processus n'as pas les droits (semaphores)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EIDRM){
        couleur(ROUGE);
        fprintf(stdout,"IDRM (semaphores)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==ERANGE){
        couleur(ROUGE);
        fprintf(stdout,"ERANGE (sempahores)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EFAULT){
        couleur(ROUGE);
        fprintf(stdout,"EFAULT (sempahores)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EINVAL){
        couleur(ROUGE);
        fprintf(stdout,"EINVAL (sempahores)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }


    /* Initialisation des sém */
    for (i = 0; i < 5; i++){

        semctl(sem,i,SETVAL,outils[i]);
    }
    
    

    /*---Initalisation du segment de memoire partagé---*/
    id_segm = shmget(cle,atoi(argv[1])*sizeof(chef), IPC_CREAT | IPC_EXCL | 0660);
    if (errno==EACCES){
        couleur(ROUGE);
        fprintf(stdout,"Processus n'as pas les droits (Segment de memoire partagé)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EIDRM){
        couleur(ROUGE);
        fprintf(stdout,"IDRM (Segment de memoire partagé)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==ERANGE){
        couleur(ROUGE);
        fprintf(stdout,"ERANGE (Segment de memoire partagé)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EFAULT){
        couleur(ROUGE);
        fprintf(stdout,"EFAULT (Segment de memoire partagé)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    if (errno==EINVAL){
        couleur(ROUGE);
        fprintf(stdout,"EINVAL (Segment de memoire partagé)\nbye\n");
        couleur(REINIT);
        exit_erreur();
    }
    P(4,1);
    planning=shmat(id_segm,NULL,0);
    for(i=0;i<nb_chefs;i++){
        planning[i].nb_clients=0;
        planning[i].nb_total=nb_chefs;
    }
    V(4,1);


    /*---Lancement des chefs et des mecaniciens ---*/
    lancement_chefs(nb_chefs,outils);
    lancement_mecano(nb_mecano);


    /*---Création des parametres pour les clients---*/
    for(i=0;i<3;i++){
        param[i]=malloc(sizeof(int));
    }
    sprintf(param[0], "%d", nb_chefs);
    sprintf(param[1], "%d", id_segm);
    sprintf(param[2], "%d", sem);
    

    /*---Boucle infini avec landement des fils---*/
    for (;;)
    {
        sleep(duree_clients);
        pid = fork();
        if (pid == -1)
        {
            couleur(ROUGE);
            fprintf(stdout, "Erreur des pid\n");
            couleur(REINIT);
        }
        if (pid == 0)
        {
            execl("client", "client", param[0],param[1],param[2], NULL);
        }
        couleur(ROUGE);
        fprintf(stdout,"Lancement clients %d\n",pid);
        couleur(REINIT);
    }
}