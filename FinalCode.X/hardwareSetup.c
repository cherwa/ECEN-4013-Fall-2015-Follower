#include <p33ep256mu806.h>

void setupHardware(){

    //MIRP Stuff
    //MIRP Enable
    TRISDbits.TRISD9 = 0;
    //MIRP Carrier
    TRISDbits.TRISD10 = 0;
}
