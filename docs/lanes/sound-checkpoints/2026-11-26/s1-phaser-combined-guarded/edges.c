#include "phaser.c"
#include <float.h>
#include <stdio.h>
#include <assert.h>
int main(void){
 float pole[]={-nextafterf(.375f,INFINITY),-.9f,-1.f,-100.f,0.f,-0.f,0x1p-149f,-0x1p-149f,0x1p-126f,.375f,-INFINITY,INFINITY,NAN};
 for(unsigned i=0;i<sizeof pole/sizeof pole[0];i++) assert(ph_word(ph_pole(pole[i]))==ph_word((float)exp((double)pole[i])));
 float rot[]={nextafterf(.015625f,INFINITY),-nextafterf(.015625f,INFINITY),.16f,-.16f,0.f,-0.f,0x1p-149f,-0x1p-149f,INFINITY,-INFINITY,NAN};
 for(unsigned i=0;i<sizeof rot/sizeof rot[0];i++){float s,c;ph_rotation(rot[i],&s,&c);assert(ph_word(s)==ph_word((float)sin((double)rot[i]))&&ph_word(c)==ph_word((float)cos((double)rot[i])));}
 printf("13 pole and 11 two-output rotation fallback edge values bit-identical\n");
}
