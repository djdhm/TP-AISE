#include <stdlib.h>
#include <pthread.h>
void  test1(void * args){
    int * a = malloc(200);
    int * b =malloc(6000);
    free(a);
    free(b);
}
void  test2(void * args){
    int * a = malloc(200);
    int * b =malloc(6000);
    free(a);
    free(b);
}
int main(){

    int * test = malloc(4*10);
    int * b = malloc(20);

    pthread_t thread1,thread2,thread3,thread4,thread5,thread6,thread7;
    pthread_create(&thread1,NULL,test1,NULL);
    pthread_create(&thread2,NULL,test1,NULL);
    pthread_create(&thread3,NULL,test1,NULL);
    pthread_create(&thread4,NULL,test1,NULL);
    pthread_create(&thread5,NULL,test1,NULL);
    pthread_create(&thread6,NULL,test1,NULL);
    pthread_create(&thread7,NULL,test1,NULL);

    pthread_create(&thread2,NULL,test2,NULL);
    free(b);
    free(test);

return 0;
}
