#pragma once
#include <stdint.h>
typedef int (*sub_pair)(void *ctx,unsigned channel,unsigned frame,float *a,float *b);
typedef struct {
  sub_pair pair; void *ctx;
  unsigned frames,channels,loop;
  float start,end,first,last;
} sub_media;
typedef struct {
  const sub_media *media;
  double cursor;
  unsigned gate,active;
} sub_owner;
/* Caller invokes commit at note/sample-ownership handoff, not a panel write. */
int sub_commit(sub_owner *o,const sub_media *m);
int sub_gate(sub_owner *o,unsigned gate);
/* -1: fail closed, outputs/state untouched. 0: finished. 1: emitted and advanced. */
int sub_frame(sub_owner *o,double sr_ratio,double note_ratio,float gain,float *audio);
typedef struct {const int16_t *data;unsigned frames,channels;} sub_pcm16;
int sub_pcm16_pair(void *ctx,unsigned channel,unsigned frame,float *a,float *b);
