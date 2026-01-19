#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    //initialise window dimensions variables
    mainWindowWidth = ofGetWidth();
    mainWindowHeight = ofGetHeight();
    leftMargin = mainWindowWidth * 0.1f;
    topMargin = mainWindowHeight * 0.1f;

	//Allocate the video texture
	videoTexture.allocate(1920,1080,GL_RGBA);

	//Allocate the Fbo where the images will be mixed
	imagesFbo.allocate(1920, 1080,GL_RGBA);
	imagesFbo.begin();
	ofClear(255,255,255,255);
	imagesFbo.end();

	//load the settings file

    ofLog() << "loading settings.xml" <<endl;
    if( settings.load("settings.xml") ){
        ofLog() << "settings.xml loaded!" << endl;
    }
    else{
        ofLogWarning()<< "unable to load settings.xml check data/ folder" << endl;
    }
    
    //Setup the projector control
    projector.setup();
    string myProjectorIP = settings.getValue("PROJECTOR:ADDRESS","192.168.1.185");
    projector.setProjectorIP(myProjectorIP);
    projector.setProjectorType(PJLINK_MODE);
    //projector.sendPJLinkCommand("%1POWR ?");
    
	//load the logo
	logo.load("logo.png");

    //load the images in the "images folder"
    string imgPath = settings.getValue("IMAGES:PATH","images");
    ofDirectory imgDir(imgPath);
	if (imgDir.isDirectory()){
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
		displayLogo = false;
	}

	else {
		displayLogo = true;
	}
 
    //load the audio files
    string soundsPath = settings.getValue("SOUNDS:PATH","audio");
	soundsDir.open(soundsPath);
	if (soundsDir.isDirectory() && !soundsDir.isDirectoryEmpty(soundsPath)){
		soundsDir.allowExt("wav");
		soundsDir.listDir();
		soundsDir.sort();
		ofLog()<<"loaded "<<soundsDir.size()<<" audio files."<<endl;
		bellState = 0;
		bellFileName = "";
	}
	else {
		ofLogError()<<"audio directory empty or not a directory"<<endl;
	}

    //load the video files
    string videoPath = settings.getValue("VIDEOS:PATH","video");
	videoDir.open(videoPath);
	if (videoDir.isDirectory() && !videoDir.isDirectoryEmpty(videoPath)){
		videoDir.allowExt("mp4");
		videoDir.listDir();
		videoDir.sort();
		ofLog()<< "loaded " + ofToString(videoDir.size()) + " videos" <<endl ;
		
		
		
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
	}
	else {
		ofLogError()<<"video directory empty or not available"<<endl;
	}

    elapsedTime = timeFormat(globalElapsed);
    remainingTime = timeFormat(globalDuration);
    
    position = 0;
    playlistPosition = 0;

    videoStarted = false;

    
    //Setup the GUI elements and the OSD
    verdana.load("verdana.ttf", 30, true, true);
    verdanaSmall.load("verdana.ttf", 12, true, true);
    
    imageTime = settings.getValue("IMAGES:CHANGETIME",600);
    fadeInTime = settings.getValue("FADEINTIME",120);
    
    //set the global opacity to 255
    mainFade.setDuration(fadeInTime);
    mainFade.setBeginning(0);
    mainFade.setTarget(255);
    mainFade.start();

	//set the image carousel speed
	carouselTicker.setInterval(imageTime);

	//setup the OSC receiver
	oscPort = settings.getValue("OSC:PORT", 7999);
	oscReceiver.setup(oscPort);

}

//THIS IS THE PROJECTION WINDOW SETUP
void ofApp::setupProjector(){
    //ofSetFullscreen(true);
    projectorWindowWidth = ofGetWidth();
    projectorWindowHeight = ofGetHeight();
    ofSetBackgroundColor(0);
}

//--------------------------------------------------------------
void ofApp::update(){

    mainFade.update();
	oscUpdate();

    //update the sound system
    ofSoundUpdate();
	if (bellState == 1){
		if (videoStarted){
			video.setVolume(0);
		}
		if(!sound.isPlaying()){
			sound.stop();
			sound.unload();
			bellState = 0;
			if (videoStarted){
				video.setVolume(1);
			}
		}
	}

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

	if (videoStarted){
		if (video.getIsMovieDone()){
			//load and play next video
			oldElapsed = globalElapsed;
			playlistPosition++;
			//ofLog()<< "Moving playlist cursor to position: " << playlistPosition <<endl;
			if (playlistPosition < videoDir.size()){
				string vP = ofToDataPath(videoDir.getPath(playlistPosition),true);
				video.load(vP);
				video.setLoopState(OF_LOOP_NONE);
				video.play();
				ofLog()<< "playing video number: " << playlistPosition <<endl;
			}

			//LOOP
			else if(videoLoop){
				//reset the counters
				imagesPosition = 0;
				position = 0;
				playlistPosition = 0;
				logoTime = 0;
				oldElapsed = 0;
				globalElapsed = 0;
				string vP = ofToDataPath(videoDir.getPath(playlistPosition),true);
				video.load(vP);
				video.setLoopState(OF_LOOP_NONE);
				video.play();
			}
			//if videos are finished and the loop is deactivated, go to black and restart the carousel
			else{
				ofLog()<< "no more videos. Stopping" <<endl;
				videoStarted = false;

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
	if (displayLogo){
		if (logoTime <= 1800){ //display the logo for 30 seconds, then restart the carousel
			currentImage = &logo;
			logoTime++;
		}
		else {
			if (images.size() != 0){ //if the image directory is empty, keep the logo on
				displayLogo = false;
				imagesPosition = 0;
				currentImage = &images.at(imagesPosition);
			}
		}

	}
        
	//draw the fbo
	imagesFbo.begin();
	currentImage->draw(0,0,imagesFbo.getWidth(),imagesFbo.getHeight());
	imagesFbo.end();


    //advance the carousel
    if (carouselTicker.tick()){
        imagesPosition++;
        if (imagesPosition >= images.size()){
                imagesPosition = 0;
        }
		if (images.size() != 0){ //don't start the carousel if the image directory is empty
			currentImage = &images.at(imagesPosition);
		}

    }
    

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofHideCursor();
    if (videoStarted && !video.isPaused()){
		if(video.getTexture().isAllocated()){
			videoTexture = video.getTexture();
		}
    }
    else {
		if(imagesFbo.isAllocated()){
			videoTexture = imagesFbo.getTexture();
		}
    }
	if (videoTexture.isAllocated()){
		videoTexture.draw(0,0,mainWindowWidth,mainWindowHeight);
	}

    //Draw the GUI and the OSD
    
    ofSetColor(0, 0, 0,128);
    ofDrawRectRounded(mainWindowWidth * 0.8f, mainWindowHeight * 0.05f, mainWindowWidth * 0.145f, mainWindowHeight * 0.07f, 10); //clock background
    ofDrawRectRounded(mainWindowWidth * 0.1f, mainWindowHeight * 0.75f, mainWindowWidth * 0.84f, mainWindowHeight * 0.15f, 10); //video background
    ofDrawRectRounded(mainWindowWidth * 0.1f, mainWindowHeight * 0.65f, mainWindowWidth * 0.4f, mainWindowHeight *0.06f, 10); //bells background

	//draw clock
	ofSetColor(255,255,255,255);
    now = ofToString(ofGetHours(),0,2,'0')+":"+ofToString(ofGetMinutes(),0,2,'0')+":"+ofToString(ofGetSeconds(),0,2,'0');
    verdana.drawString(now, mainWindowWidth * 0.8f, topMargin);

	//draw fps meter
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
    verdana.drawString(bellFileName+" - "+bellIsPlaying,mainWindowWidth*0.1f,mainWindowHeight * 0.69f);

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
	if (videoTexture.isAllocated()){
		videoTexture.draw(0,0,projectorWindowWidth,projectorWindowHeight);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key){
        case ' ':
            playVideo();
			break;
        case 's':
			stopVideo();
			break;
		case 'l':
			videoLoop = !videoLoop;
			break;
		case 'd':
			bellStop();
			break;
        case 'w':
            projector.On();
            break;
        case 'q':
            projector.Off();
            break;
		case 49:
			playBell(0);
			break;
		case 50:
			playBell(1);
			break;
		case 51:
			playBell(2);
			break;
		case 52:
			playBell(3);
            break;
		case 53:
			playBell(4);
			break;
		case 'a':
			mainFade.start();
			break;
		case 'r':
			system("sudo reboot");
			break;
		case 27:
			ofLog()<<"esc key pressed, quitting"<<endl;
			break;
		default:
			ofLog()<< "wrong key pressed: "<<key <<endl;
    }
}

//--------------------------------------------------------------



void ofApp::playVideo(){
	if (videoDir.size() != 0){ //Don't try to load videos if the video directory is empty
		displayLogo = false;
		//If the videos are not playing, load and start the first video,
		//if videos are playing, toggle pause state
		if (!videoStarted){
			//load the video
			playlistPosition = 0;
			string vP = ofToDataPath(videoDir.getPath(playlistPosition),true);//NOTE: only the first video in the folder gets loaded
			video.load(vP);
			video.setLoopState(OF_LOOP_NONE);
			video.play();
			ofLog()<< "first video started" <<endl;
			videoStarted = true;
		}
		else if(!video.isPaused()){
			ofLog()<< "video paused" <<endl;
			video.setPaused(true);
		}
		else if(video.isPaused()){
			ofLog()<< "playing resumed" <<endl;
			video.setPaused(false);
		}
	}

}

void ofApp::stopVideo(){
	video.stop();
	videoStarted = false;
	//reset the counters
	imagesPosition = 0;
	position = 0;
	playlistPosition = 0;
	logoTime = 0;
	oldElapsed = 0;
	globalElapsed = 0;
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

void ofApp::playBell(unsigned int b){
	if (bellState == 0){
		if (soundsDir.size() != 0){ //Don't play if audio directory is empty
			if (b < soundsDir.size()){
				sound.load(soundsDir.getPath(b));
				bellFileName = soundsDir.getPath(b);
				bellFileName = ofFilePath::getBaseName(bellFileName);
				ofLog()<<"playing: "<<bellFileName<<endl;
				sound.play();
				sound.setLoop(false);
				sound.setPan(1.0f);
				bellState = 1;
			}
			else {
				ofLogWarning()<<"out of range. No audio file present at index "<<b<<endl;
			}
		}
		else {
			ofLogWarning()<<"Sound directory is empty. Not Playing"<<endl;
		}
	}
	else {
		ofLogWarning()<<"A bell is already playing"<<endl;
	}
}

void ofApp::bellStop(){
	if (sound.isPlaying()){
		sound.stop();
		sound.unload();
		bellState = 0;
		ofLog()<<"Bell stopped"<<endl;
	}
}

void ofApp::oscUpdate(){
	while (oscReceiver.hasWaitingMessages()){
		ofxOscMessage m;
		oscReceiver.getNextMessage(m);

		if(m.getAddress() == "/bell/play"){
			if (m.getNumArgs() > 0){
				playBell(m.getArgAsInt(0));
			}
		}
		else if(m.getAddress() == "/bell/stop"){
			bellStop();
		}
		else if(m.getAddress() == "/video/play"){
			playVideo();
		}
		else if(m.getAddress() == "/video/stop"){
			stopVideo();
		}
		else if(m.getAddress() == "/video/loop"){
			if (m.getNumArgs() > 0){
				if(m.getArgAsInt(0) == 1){
					videoLoop = true;
					ofLog()<<"Video Looping on"<<endl;
				}
				else {
					videoLoop = false;
					ofLog()<<"Video Looping off"<<endl;
				}
			}
		}

		else{

			// unrecognized message: display on the bottom of the screen
			string msgString;
			msgString = m.getAddress();
			msgString += ":";
			for(size_t i = 0; i < m.getNumArgs(); i++){

				// get the argument type
				msgString += " ";
				msgString += m.getArgTypeName(i);
				msgString += ":";

				// display the argument - make sure we get the right type
				if(m.getArgType(i) == OFXOSC_TYPE_INT32){
					msgString += ofToString(m.getArgAsInt32(i));
				}
				else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
					msgString += ofToString(m.getArgAsFloat(i));
				}
				else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
					msgString += m.getArgAsString(i);
				}
				else{
					msgString += "unhandled argument type " + m.getArgTypeName(i);
				}
			}
			//print the warning
			ofLogWarning()<<msgString<<endl;
		}
	}//END check messages
}
