#ifndef _item_h_
#define _item_h_

#define PLANT 0x01
#define TRANSPARENT 0x02
#define OBSTACLE 0x04

#define IS(t, mask) (t & mask)
#define SET(orig, mask) (orig | mask)
#define UNSET(orig, mask) (orig & ~mask)

#define STATE_OFF 3
#define STATE_MASK 0x18

#define GET_V(t, off, mask) ((t & mask) >> off)
#define SET_V(orig, v, off, mask) (((v << off) & mask) | orig)

#define GET_STATE(t) GET_V(t, STATE_OFF, STATE_MASK)
#define SET_STATE(orig, v) SET_V(orig, v, STATE_OFF, STATE_MASK)
#define IS_STATE(t, v) (GET_STATE(t) == v)

#define SOLID 0
#define FLUID 1
#define GAS 2

extern int blocks[256][6];
extern char block_type[256];

#endif
