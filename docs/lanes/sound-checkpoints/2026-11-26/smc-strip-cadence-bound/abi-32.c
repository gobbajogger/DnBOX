struct button {const char *caption;unsigned short ink;unsigned char symbol, led;};
struct strip {struct button b[4];char title[24],knob_label[24],knob_value[32],fader_value[32];
unsigned short inks[5]; unsigned char on[8];};
struct snapshot {struct strip row[8];unsigned short colors[4];unsigned char flags[4];};
_Static_assert(sizeof(void *) == 4 && sizeof(struct snapshot) == 1324, "witness");
