#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"

//========================================================================
int main( ){
    
    //Create two windows with shared render context and put them in their respective monitor
    ofGLFWWindowSettings ctlWinSettings;
	ctlWinSettings.title = "Control Window";
	ctlWinSettings.monitor = 0;

    ctlWinSettings.windowMode = OF_WINDOW;
	ctlWinSettings.decorated = false;
	ctlWinSettings.setSize(640,400);
    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(ctlWinSettings);
    mainWindow->setWindowPosition(0, 0);
   // mainWindow->setFullscreen(true);

	ofGLFWWindowSettings projWinSettings;
	projWinSettings.title = "Projection Window";
	projWinSettings.shareContextWith = mainWindow;
	projWinSettings.decorated = false;
	projWinSettings.monitor = 1;
	projWinSettings.setSize(640, 360);
    shared_ptr<ofAppBaseWindow> projectorWindow = ofCreateWindow(projWinSettings);
    projectorWindow->setVerticalSync(true);
   // projectorWindow->setWindowPosition(1281,0);
   // projectorWindow->setFullscreen(true);
    
    
    shared_ptr<ofApp> mainApp(new ofApp);
    mainApp->setupProjector();
    ofAddListener(projectorWindow->events().draw,mainApp.get(),&ofApp::drawProjector);
    
    ofRunApp(mainWindow, mainApp);
    ofRunMainLoop();
    
}
