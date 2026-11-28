#include "drum909.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
void ref_drum909_init(drum909_t*);
void ref_drum909_trigger(drum909_t*,int,float);
void ref_drum909_render(drum909_t*,float*,float*,float*,int);
static void bench(int ref)
{
    void (*render)(drum909_t*,float*,float*,float*,int)=ref?ref_drum909_render:drum909_render;
    void (*trigger)(drum909_t*,int,float)=ref?ref_drum909_trigger:drum909_trigger;
    drum909_t d; memset(&d,0,sizeof d); (ref?ref_drum909_init:drum909_init)(&d);
    float check=0.f;
    clock_t start=clock();
    for(int i=0;i<70000;i++) {
        if(i%22==0) for(int t=DR_LT;t<=DR_HT;t++) trigger(&d,t,.7f);
        float y[256]={0};
        render(&d,y,0,0,256);
        check+=y[i&255];
    }
    printf("%s %.6fs %.8g\n",ref?"stock":"stage",(double)(clock()-start)/CLOCKS_PER_SEC,check);
}
int main(void) { for(int i=0;i<2;i++) {bench(1); bench(0);} }
