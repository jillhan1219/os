#include <stdio.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_IMMIGRANT 10
#define MAX_SPECTOR 10
bool judge_in = false, confirm = false;
int immigrant_enter_cnt = 0, immigrant_ckn_cnt = 0, judge_enter_times = 0, spector_cnt = 0, last = 1;
// number of immigrant and spector threads 
int imt_pid_cnt=0,spt_pid_cnt=0;
/* mutex: avoid starvation
*  immigrant_enter: the number of immigrants entering the building
*  immigrant_ckn: the number of immigrants checkin
*  confirmed: judge wait for immigrants to checkin
*  spector_num: the number of spectors entering the building
*/
sem_t mutex, immigrant_enter, immigrant_ckn, confirmed, spector_num;

void *immigrants(void *args){
    int id = *(int *)args;
    // immigrant wait to enter if judge is in the building
    while (judge_in);
    sem_wait(&mutex);
    // enter
    sem_wait(&immigrant_enter);
    immigrant_enter_cnt++;
    while (judge_in);
    printf("Immigrant #%d enters.\n", id);
    sleep(rand()%3);
    sem_post(&immigrant_enter);
    sem_post(&mutex);
    // checkin and sitdown
    sem_wait(&immigrant_ckn);
    immigrant_ckn_cnt++;
    printf("Immigrant #%d checkIn.\n", id);
    printf("Immigrant #%d sitDown.\n", id);
    sleep(rand()%3);
    sem_post(&immigrant_ckn);
    while (!judge_in);
    sem_wait(&confirmed);
    sem_post(&confirmed);
    // immigrants wait to leave if judge is in the building
    while (judge_in);
    // get certificate and leave
    printf("Immigrant #%d getCertificate.\n", id);
    printf("Immigrant #%d leaves.\n", id);
    sleep(rand()%3);
    pthread_exit(NULL);
}

void *judge(){
    do {
        sleep(rand()%3+3);
        // judge enter
        sem_wait(&mutex);
        judge_enter_times++;
        printf("Judge #%d enters.\n", judge_enter_times);
        judge_in = true;
        sleep(rand()%3);
        sem_post(&mutex);
        // judge wait to confirm if not all entering immigrants have checked in
        while (immigrant_enter_cnt != immigrant_ckn_cnt);
        sem_wait(&confirmed);
        // judge confirm
        for (int i = last; i <= immigrant_ckn_cnt ; ++i) {
            printf("Judge confrim the immigrant #%d.\n", i);
            sleep(rand()%3);
        }
        last = immigrant_ckn_cnt + 1;
        sem_post(&confirmed);
        // judge leave
        printf("Judge #%d leaves.\n", judge_enter_times);
        sleep(rand()%3);
        judge_in = false;
    } while ((last-1)<MAX_IMMIGRANT);// while there are still immigrants to be confirmed
    pthread_exit(NULL);
}

void *spector(void* args){
    int id = *(int *)args;
    // spector wait to enter if judge is in the building
    while (judge_in);
    sem_wait(&mutex);
    sem_wait(&spector_num);
    spector_cnt++;
    while (judge_in);
    // spector enter
    printf("Spector #%d enters.\n",id);
    sem_post(&spector_num);
    sem_post(&mutex);
    // spector spectate
    printf("Spector #%d spectates.\n", id);
    sleep(rand()%3);
    printf("Spector #%d leaves.\n", id);
    sleep(rand()%3);
    pthread_exit(NULL);
}

int main(){
    pthread_t jd,spt,imt;
    sem_init(&mutex, 0, 1);
    sem_init(&immigrant_enter, 0, 1);
    sem_init(&immigrant_ckn, 0, 1);
    sem_init(&confirmed, 0, 1);
    sem_init(&spector_num, 0, 1);


    pthread_create(&jd, NULL, judge, NULL);

    while (imt_pid_cnt<MAX_IMMIGRANT)
    {
        imt_pid_cnt++;
        if(pthread_create(&imt,NULL,immigrants,&imt_pid_cnt)!=0){
            fprintf(stderr,"pthread_create error\n");
            exit(1);
        }
        sleep(rand()%3);
        spt_pid_cnt++;
        if(pthread_create(&spt,NULL,spector,&spt_pid_cnt)!=0){
            fprintf(stderr,"pthread_create error\n");
            exit(1);
        }
        sleep(rand()%3);
    }
    

    pthread_join(jd, NULL);
    pthread_join(spt, NULL);
    pthread_join(imt, NULL);

    sem_destroy(&mutex);
    sem_destroy(&immigrant_enter);
    sem_destroy(&immigrant_ckn);
    sem_destroy(&confirmed);
    sem_destroy(&spector_num);

    return 0;
}
