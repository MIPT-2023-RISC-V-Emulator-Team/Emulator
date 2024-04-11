#define SIM_X_SIZE 800
#define SIM_Y_SIZE 800


void simInit();
void simExit();
void simFlush();
void simPutPixel(int x, int y, int argb);
int simRand();
