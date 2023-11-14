//Bryce Henderson
//Hend0188


#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/types.h>

#define SHMKEY ((key_t) 1497)
#define SEMKEY ((key_t) 400L)

#define NSEMS 1

typedef struct {
    int value;
}

shared_mem;
shared_mem *total;

int sem_id;
static struct sembuf OP = {0,-1,0};
static struct sembuf OV = {0,1,0};
struct sembuf *P = &OP;
struct sembuf *V = &OV;



//Semaphore wait function
int POP() {
    int status;
    status = semop(sem_id,P,1);
    return status;
}

//Sempahore signal function
int VOP() {
    int status;
    status = semop(sem_id,V,1);
    return status;
}

void process(int process, int goal);

int main(){
    int shmid, ID, status, value, value1;
    union semun
    {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } semctl_arg;
    semctl_arg.val = 1;
    char *shmadd;
    shmadd = (char *)0;

    

    //Creating sempahore
    sem_id = semget(SEMKEY, NSEMS, IPC_CREAT | 0666);
    if (sem_id < 0){
         printf("Error in creating the semaphore.\n");
    }
        
    //Initialize semaphores
    value1 = semctl(sem_id, 0, SETVAL, semctl_arg);
    value = semctl(sem_id, 0, GETVAL, semctl_arg);
    if (value < 0){
        printf("Error detected in SETVAL.\n");
    }

    if ((shmid = shmget(SHMKEY, sizeof(int), IPC_CREAT | 0666)) < 0){ //declaring the shmid
        perror("shmget");
        exit(1);
    }
    if ((total = (shared_mem *)shmat(shmid, shmadd, 0)) ==         //assigning total
        (shared_mem *)-1){
            perror ("shmat");
            exit(0);
    }

   total->value = 0;

   int increments[] = {100000, 200000, 300000, 500000};
    for (int i = 0; i < 4; ++i){       //parent function forking into four processes
        pid_t pid = fork();
        if (pid == -1){
            perror("fork");
        }
        if (pid == 0){
            process(i, increments[i]);
        }

    }


        for (int i = 0; i < 4; ++i){            //parent waiting for the children to exit, then printing their pid
            pid_t child_pid = wait(&status);
            printf("Child with ID: %d has just exited. \n", child_pid);
        }

        if (shmdt(total) == -1){
            perror("shmdt");
            exit(-1);
        }

        if (shmctl(shmid, IPC_RMID, NULL) == -1)
        {
            perror("shmctl");
        }

        shmdt(total);
        shmctl(shmid, IPC_RMID, NULL);

    
    printf("End of program. \n");
    
 
    semctl_arg.val = 0;
    status = semctl(sem_id, 0, IPC_RMID, semctl_arg);
    if(status < 0) {
        printf("Error in removing the semaphore.\n");
    }
    return 0;
}


void process(int process, int goal){      //process to be run by each of the child to increment

    for (int i = 0; i < goal; ++i) {
        POP();
        total->value++;
        VOP();
        
    }
    printf("From Process %d: total = %d \n", process+1, total->value);
    exit(1);
}
