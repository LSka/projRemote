#include "fadetimer.h"

FadeTimer::FadeTimer()
{

}

void FadeTimer::setDuration(unsigned int frames){
    duration = frames;
}

void FadeTimer::setBeginning(float value){
    beginningValue = value;
}

void FadeTimer::setTarget(float value){
    targetValue = value;
}

void FadeTimer::start(){
    frameCount = 0;
    fadeStarted = true;
}

void FadeTimer::update(){
    if (fadeStarted){
        if (!hasEnded()){
            frameCount++;
        }
    }

}

bool FadeTimer::hasEnded(){
    if (frameCount == duration){
        fadeStarted = false;
        return true;
    }
    else{
        return false;
    }
}

int FadeTimer::getValue(){
    currentValue = ofMap(frameCount,0,duration,beginningValue,targetValue);
    return currentValue;
}

