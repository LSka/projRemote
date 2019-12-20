#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxPJControl.h"
#include "fadetimer.h"
#include "ticker.h"


class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    void setupProjector();
    void drawProjector(ofEventArgs & args);
    void keyPressed(int key);
    void playButtonPressed();
    void stopButtonPressed();
    void playVideo();
    string timeFormat(float time);
    
    
    
    ofPath soundsPath;
    vector<ofImage> images;
    ofImage bgImage;

    
    ofVideoPlayer video;
    ofTexture videoTexture;
    
    ofxXmlSettings settings;
    
    
    float mainWindowWidth;
    float mainWindowHeight;
    float projectorWindowWidth;
    float projectorWindowHeight;
    float duration;
    float position;
    float elapsedVideoTime;
    float globalElapsed;
    float globalDuration;
    float oldElapsed;
    float leftMargin;
    float rightMargin;
    float topMargin;
    float bottomMargin;
    
    unsigned int fadeFrames;
    unsigned int mainAlpha;
    unsigned int xFadeTime;
    unsigned int imageTime;
    unsigned int fadeInTime;
    
    ofTrueTypeFont  verdana;
    ofTrueTypeFont  verdanaSmall;
    string elapsedTime;
    string remainingTime;
    
    ofParameterGroup parameters;

        
    ofxPJControl projector;
    
    
    bool videoStarted;
    bool isProjectorOn;
    
    ofImage* prevImage;
    ofImage* currentImage;
    ofFbo imagesFbo;
    unsigned int imagesPosition;
    
    ofSoundPlayer sound;
    ofDirectory soundsDir;
    unsigned int playhead;
    int bellState;

    ofDirectory videoDir;
    unsigned int playlistPosition;

    FadeTimer mainFade;
    FadeTimer xFadeTimer;
    FadeTimer videoVolumeTimer;
    Ticker carouselTicker;
    
};
