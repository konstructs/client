#ifndef _item_h_
#define _item_h_

#define OSCILLATION_TYPE_NONE 0.0f
#define OSCILLATION_TYPE_OCEAN 1.0f
#define OSCILLATION_TYPE_PLANT 2.0f

extern int blocks[256][6];
extern char is_plant[256];
extern char is_obstacle[256];
extern char is_transparent[256];
extern float oscillation_type[256];

#endif
