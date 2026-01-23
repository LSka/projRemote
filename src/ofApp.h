#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxPJControl.h"
#include "ofxOsc.h"
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
	void stopVideo();
	void playBell(unsigned int b);
	void bellStop();
	void oscUpdate();
    string timeFormat(float time);
	string bellFileName;
	string now;

    
    
    ofPath soundsPath;
    vector<ofImage> images;
    ofImage logo;

    
    ofVideoPlayer video;
    ofTexture videoTexture;
	ofImage* prevImage;
	ofImage* currentImage;
	ofFbo imagesFbo;

    ofxXmlSettings settings;

	ofxOscReceiver oscReceiver;

    
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
    
    unsigned int mainAlpha;
    unsigned int imageTime;
    unsigned int fadeInTime;
    unsigned int logoTime;
	unsigned int imagesPosition;
	unsigned int playhead;
	unsigned int playlistPosition;
	unsigned int oscPort;
	int bellState;

    ofTrueTypeFont  textFontLarge;
    ofTrueTypeFont  textFontMedium;
	ofTrueTypeFont	textFontSmall;
    string elapsedTime;
    string remainingTime;
	string soundFileNamesList;

    ofParameterGroup parameters;

    ofxPJControl projector;

    bool videoStarted;
    bool isProjectorOn;
    bool displayLogo;
	bool videoLoop;

    ofSoundPlayer sound;
    ofDirectory soundsDir;
    ofDirectory videoDir;

    FadeTimer mainFade;
    Ticker carouselTicker;
    
};
