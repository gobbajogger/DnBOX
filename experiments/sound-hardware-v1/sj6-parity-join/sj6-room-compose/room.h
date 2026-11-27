#include <stdint.h>
typedef struct RoomOwner RoomOwner;
typedef struct RoomBlock {
 const float *after_room_hp_l,*after_room_hp_r;
 unsigned frames,send_open,room_type,wet_limiter_bypassed,dry_rage_already_applied;
 float wet_mix, rage_tier_gain;
} RoomBlock;
RoomOwner *compose_room_create(void);
void compose_room_destroy(RoomOwner *);
int compose_room_admits(const RoomOwner*,const RoomBlock*,const float*,const float*,unsigned);
void compose_room_tick(RoomOwner*,const RoomBlock*,float*,float*,unsigned);
int compose_room_feedback(RoomOwner*,float);
