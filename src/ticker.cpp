#include "ticker.h"

Ticker::Ticker()
{

}

void Ticker::setInterval(unsigned int i){
    interval = i;
}

bool Ticker::tick(){
    if (frameCounter >= interval){
        frameCounter = 0;
        return true;
    }
    else {
        frameCounter++;
        return false;
    }
}
