#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    //initialise window dimensions variables
    mainWindowWidth = ofGetWidth();
    mainWindowHeight = ofGetHeight();
    
    leftMargin = mainWindowWidth * 0.1f;
    topMargin = mainWindowHeight * 0.1f;
    
    ofBackground(0);
    
    ofLog() << "loading settings.xml" <<endl;
    //we load our settings file
    if( settings.loadFile("settings.xml") ){
        ofLog() << "settings.xml loaded!" << endl;
    }
    else{
        ofLog()<< "unable to load settings.xml check data/ folder" << endl;
    }
    
    //Setup the projector control
    projector.setup();
    string myProjectorIP = settings.getValue("PROJECTOR:ADDRESS","192.168.1.185");
    projector.setProjectorIP(myProjectorIP);
    projector.setProjectorType(PJLINK_MODE);
    //projector.sendPJLinkCommand("%1POWR ?");
    
    
    //load the images in the "images folder"
    string imgPath = settings.getValue("IMAGES:PATH","images");
    ofDirectory imgDir(imgPath);
    imgDir.allowExt("png");
    imgDir.allowExt("jpg");
    imgDir.listDir();
    imgDir.sort();
    for (unsigned int i = 0; i < imgDir.size(); i++){
        ofImage img;
        img.load(imgDir.getPath(i));
        images.push_back(img);
    }
    imagesPosition = 0;
    currentImage = &images[imagesPosition];

    //load the logo
    logo.load("logo.png");
    displayLogo = false;
    
    
    //load the audio files
    string soundsPath = settings.getValue("SOUNDS:PATH","audio");
    soundsDir.open(soundsPath);
    soundsDir.allowExt("wav");
    soundsDir.listDir();
    soundsDir.sort();
    playhead = 0;
    bellState = 0;
    

    //load the video files
    string videoPath = settings.getValue("VIDEOS:PATH","video");
    videoDir.open(videoPath);
    videoDir.allowExt("mp4");
    videoDir.listDir();
    videoDir.sort();

    //compute global video duration
    globalDuration = 0;
    oldElapsed = 0;
    globalElapsed = 0;
    for (unsigned int v = 0; v < videoDir.size(); v++){
        string vP = ofToDataPath(videoDir.getPath(v),true);
        ofVideoPlayer vid;
        vid.load(vP);
        float d = vid.getDuration();
        globalDuration += d;
    }
    
    elapsedTime = timeFormat(globalElapsed);
    remainingTime = timeFormat(globalDuration);
    
    position = 0;
    playlistPosition = 0;
    
    videoTexture.allocate(1920,1080,GL_RGBA);
    fadeFrames = 0;
    
    //Allocate the Fbo where the images will be mixed
    imagesFbo.allocate(1920, 1080,GL_RGBA);
    imagesFbo.begin();
    ofClear(255,255,255,255);
    imagesFbo.end();
    
    videoStarted = false;
    

    ofSetFullscreen(true);
    
    //Setup the GUI elements and the OSD
    verdana.load("verdana.ttf", 30, true, true);
    verdanaSmall.load("verdana.ttf", 12, true, true);
    
    imageTime = settings.getValue("IMAGES:CHANGETIME",60);
    xFadeTime = settings.getValue("IMAGES:XFADETIME",240);
    fadeInTime = settings.getValue("FADEINTIME",120);
    
    //set the global opacity to 255

    mainFade.setDuration(fadeInTime);
    mainFade.setBeginning(0);
    mainFade.setTarget(255);
    mainFade.start();

    videoVolumeTimer.setDuration(60);
    
    carouselTicker.setInterval(imageTime);

}

//THIS IS THE PROJECTION WINDOW SETUP
void ofApp::setupProjector(){
    ofSetFullscreen(true);
    projectorWindowWidth = ofGetWidth();
    projectorWindowHeight = ofGetHeight();
    ofSetBackgroundColor(0);
}

//--------------------------------------------------------------
void ofApp::update(){

    mainFade.update();
    videoVolumeTimer.update();
    float v = float(videoVolumeTimer.getValue()) * 0.01;
    video.setVolume(v);
    
    //update the sound system
    ofSoundUpdate();
    if (bellState == 1){
        if(sound.getPosition() >= 0.99f){
            sound.stop();
            sound.unload();
            playhead++;
            if (playhead >= soundsDir.size()){
                playhead = 0;
                bellState = 0;
            }
            else if (playhead < soundsDir.size()){
                sound.load(soundsDir.getPath(playhead));
                system("amixer sset Master 0%,100%");
                sound.play();
            }
        }
    }
    
    
    if (video.isLoaded() && videoStarted){
        video.update();
        
        //compute and prepare the elapsed and remaining video time (using the timeFormat method)
        duration = video.getDuration();
        position = video.getPosition();
        elapsedVideoTime = position*duration;
        globalElapsed = oldElapsed + elapsedVideoTime;
        float remainingVideoTime = globalDuration - globalElapsed;
        if(remainingVideoTime >= INFINITY || remainingVideoTime < 0){
            remainingVideoTime = 0;
        }
        elapsedTime = timeFormat(globalElapsed);
        remainingTime = timeFormat(remainingVideoTime);
        
        
        if (video.getIsMovieDone()){
            //load and play next video
            oldElapsed = globalElapsed;
            playlistPosition++;
            if (playlistPosition < videoDir.size()){
                string vP = ofToDataPath(videoDir.getPath(playlistPosition),true);
                video.load(vP);
                video.setLoopState(OF_LOOP_NONE);
                video.play();
            }
	//LOOP
            else if(settings.getValue("LOOP:VALUE",0) == 1){
		//reset the counters
                imagesPosition = 0;
                position = 0;
                playlistPosition = 0;
                logoTime = 0;
                oldElapsed = 0;
                globalElapsed = 0;
            }


        //if videos are finished and the loop is deactivated, go to black and restart the carousel
            else{
                videoStarted = false;
                
                //Fade Video Volume to zero
                videoVolumeTimer.setDuration(1);
                videoVolumeTimer.setBeginning(videoVolumeTimer.getValue());
                videoVolumeTimer.setTarget(0);
                videoVolumeTimer.start();
                
                //show the logo
                displayLogo = true;
                
                //reset the counters
                imagesPosition = 0;
                position = 0;
                playlistPosition = 0;
                logoTime = 0;
                oldElapsed = 0;
                globalElapsed = 0;
            }


        }
    }
    else{
        if (displayLogo){
            if (logoTime <= 1800){ //display the logo for 30 seconds, then restart the carousel
                currentImage = &logo;
                logoTime++;
            }
            else {
                displayLogo = false;
                imagesPosition = 0;
                currentImage = &images.at(imagesPosition);
            }

        }
        
        //draw the fbo
        imagesFbo.begin();
        currentImage->draw(0,0,imagesFbo.getWidth(),imagesFbo.getHeight());
        imagesFbo.end();
    }

    //advance the carousel
    if (carouselTicker.tick()){
        imagesPosition++;
        if (imagesPosition >= images.size()){
                imagesPosition = 0;
        }
        currentImage = &images.at(imagesPosition);

    }
    

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofHideCursor();
    if (videoStarted && !video.isPaused()){

        videoTexture = video.getTexture();
    }
    else {
        videoTexture = imagesFbo.getTexture();
    }
    

    ofSetColor(255,255,255,255); //this is to fade the master texture
    videoTexture.draw(0,0,mainWindowWidth,mainWindowHeight);
    
    //Draw the GUI and the OSD
    ofSetColor(255,255,255,255);
    
    
    ofSetColor(0, 0, 0,128);
    ofDrawRectRounded(mainWindowWidth * 0.8f, mainWindowHeight * 0.05f, mainWindowWidth * 0.145f, mainWindowHeight * 0.07f, 10);
    ofDrawRectRounded(mainWindowWidth * 0.08f, mainWindowHeight * 0.75f, mainWindowWidth * 0.84f, mainWindowHeight * 0.16f, 10);
    ofDrawRectRounded(mainWindowWidth * 0.1f, mainWindowHeight * 0.65f, mainWindowWidth * 0.4f, mainWindowHeight *0.05f, 10);
    
    ofSetColor(255,255,255,255);
    string now = ofToString(ofGetHours(),0,2,'0')+":"+ofToString(ofGetMinutes(),0,2,'0')+":"+ofToString(ofGetSeconds(),0,2,'0');
    verdana.drawString(now, mainWindowWidth * 0.8f, topMargin);
    ofSetColor(255,255,255,200);
    verdanaSmall.drawString("FPS: "+ofToString(ofGetFrameRate(),0),mainWindowWidth * 0.8f,mainWindowHeight * 0.15f);
    verdana.drawString(elapsedTime,mainWindowWidth * 0.1f,mainWindowHeight * 0.79f);
    verdana.drawString("-"+remainingTime,mainWindowWidth * 0.71f,mainWindowHeight * 0.79f);
    
    string bellIsPlaying;
    switch (bellState){
        case 0:
            bellIsPlaying = "Stop";
            break;
        case 1:
            bellIsPlaying = "Play";
            break;
        case 2:
            bellIsPlaying = "Pausa";
            break;
    }
    verdana.drawString("Campanella: "+ofToString(playhead+1)+" - "+bellIsPlaying,mainWindowWidth*0.1f,mainWindowHeight * 0.69f);
    
    ofRectangle soundProgress;
    soundProgress.x = mainWindowWidth*0.1f;
    soundProgress.y = mainWindowHeight * 0.65f;
    soundProgress.width = mainWindowWidth * (sound.getPosition()) * 0.4f;
    soundProgress.height = mainWindowHeight * 0.06f;
    
    ofSetColor(255,255,255,255);
    ofNoFill();
    ofDrawRectangle(soundProgress.x,soundProgress.y,mainWindowWidth * 0.4f,soundProgress.height);
    ofSetColor(255,255,255,128);
    ofFill();
    ofDrawRectangle(soundProgress);
    
    
    ofRectangle progressBar;
    
    progressBar.x = mainWindowWidth * 0.1f;
    progressBar.y = mainWindowHeight * 0.8f;
    progressBar.width = mainWindowWidth * ( ( playlistPosition + position ) / videoDir.size() )* 0.8f;
    progressBar.height = mainWindowHeight * 0.1f;
    
    ofSetColor(255,255,255,255);
    ofNoFill();
    ofDrawRectangle(progressBar.x,progressBar.y,mainWindowWidth * 0.8f,progressBar.height);
    ofSetColor(255,255,255,128);
    ofFill();
    ofDrawRectangle(progressBar);
    ofSetColor(255,255,255,255);

}

//DRAW ON THE PROJECTOR WINDOW
void ofApp::drawProjector(ofEventArgs & args){
    ofSetColor(255,255,255,255);
    videoTexture.draw(0,0,projectorWindowWidth,projectorWindowHeight);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key){
        case ' ':
            system("amixer sset Master 100%,0%");
            playVideo();
            displayLogo = false;
    break;
        case 's':
            video.stop();
            videoStarted = false;
            fadeFrames = 0;
            //reset the counters
            imagesPosition = 0;
            position = 0;
            playlistPosition = 0;
            logoTime = 0;
            oldElapsed = 0;
            globalElapsed = 0;

    break;
        case 'w':
            projector.On();
            break;
        case 'q':
            projector.Off();
            break;
        case 'z':
            if (bellState == 0){

                videoVolumeTimer.setDuration(1);
                videoVolumeTimer.setBeginning(videoVolumeTimer.getValue());
                videoVolumeTimer.setTarget(0);
                videoVolumeTimer.start();
                sound.load(soundsDir.getPath(playhead));
                system("amixer sset Master 0%,100%");
                sound.play();
                bellState = 1;
            }
            else {
                if (bellState == 1){
                    sound.setPaused(true);
                    bellState = 2;
                }
                else if (bellState == 2){
                    videoVolumeTimer.setDuration(1);
                    videoVolumeTimer.setBeginning(videoVolumeTimer.getValue());
                    videoVolumeTimer.setTarget(0);
                    videoVolumeTimer.start();
                    system("amixer sset Master 0%,100%");
                    sound.setPaused(false);
                    bellState = 1;
                }
            }
            break;
        case 'x':
            sound.stop();
            sound.unload();
            playhead--;
            playhead = ofClamp(playhead, 0, soundsDir.size()-1);
            bellState = 0;
            

            break;
        case 'c':
            sound.stop();
            sound.unload();
            playhead++;
            bellState = 0;
            if (playhead >= soundsDir.size()){
                playhead = 0;
            }
            break;
    case 'a':
        mainFade.start();
        break;

   case 'p':
	sound.stop();
	bellState = 0;
	playhead = 0;
            videoVolumeTimer.setDuration(30);
            videoVolumeTimer.setBeginning(0);
            videoVolumeTimer.setTarget(100);
            videoVolumeTimer.start();
	break;
            
        case 'r':
            system("sudo reboot");
        break;
            
            
            

    }
    

}

//--------------------------------------------------------------



void ofApp::playVideo(){
    displayLogo = false;
    if (!videoStarted){
        //load the video
        playlistPosition = 0;
        string vP = ofToDataPath(videoDir.getPath(playlistPosition),true);//NOTE: only the first video in the folder gets loaded
        video.load(vP);
        video.setLoopState(OF_LOOP_NONE);
        
        videoVolumeTimer.setDuration(30);
        videoVolumeTimer.setBeginning(0);
        videoVolumeTimer.setTarget(100);
        videoVolumeTimer.start();
        
        system("amixer sset Master 100%,0%");
        video.play();
        videoStarted = true;
    }
    else if(!video.isPaused()){
        
        videoVolumeTimer.setDuration(30);
        videoVolumeTimer.setBeginning(0);
        videoVolumeTimer.setTarget(100);
        videoVolumeTimer.start();

        video.setPaused(true);
    }
    else if(video.isPaused()){
        
        videoVolumeTimer.setDuration(30);
        videoVolumeTimer.setBeginning(0);
        videoVolumeTimer.setTarget(100);
        videoVolumeTimer.start();
        
        system("amixer sset Master 100%,0%");
        video.setPaused(false);
    }
}

string ofApp::timeFormat(float time){
    if (time <= 0){
        time = 0;
    }
    int ms = (int(time) - (int(time)))*100;
    int ss = int(time) % 60;
    int mm = (int(time) / 60) % 60;
    int hh = int(time) / 3600;
    string tS = ofToString(hh,0,2,'0')+":"+ofToString(mm,0,2,'0')+":"+ofToString(ss,2,2,'0')+":"+ofToString(ms,0,2,'0');
    return tS;
}
