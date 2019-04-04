#ifndef FADETIMER_H
#define FADETIMER_H
#include "ofMain.h"

class FadeTimer
{
public:

    void setDuration(unsigned int frames);
    void setBeginning(float value);
    void setTarget(float value);
    void start();
    void update();
    bool hasEnded();

    int getValue();

    FadeTimer();

private:
    unsigned int duration;
    unsigned int frameCount;
    int beginningValue;
    int targetValue;
    int currentValue;
    bool fadeEnded;
    bool fadeStarted;
};

#endif // FADETIMER_H
