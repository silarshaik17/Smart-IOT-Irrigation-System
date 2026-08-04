#include "types.h"
#define ROW0 16 //p1.16
#define ROW1 17
#define ROW2 18
#define ROW3 19

#define COL0 20
#define COL1 21
#define COL2 22
#define COL3 23 //p1.23
void InitKPM(void);
u32 KeyScan(void);
u32 ColScan(void);
int Get1DigitValue(char *msg);
int Get2DigitValue(char *msg);

