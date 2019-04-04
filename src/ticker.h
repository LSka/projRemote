#ifndef TICKER_H
#define TICKER_H
#include "ofMain.h"

class Ticker
{
public:
    Ticker();

    bool tick();
    void setInterval(unsigned int i);

private:
    unsigned int interval;
    unsigned int frameCounter;
};

#endif // TICKER_H
