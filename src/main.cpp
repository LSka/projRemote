#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"

//========================================================================
int main( ){
    
    //Create two windows with shared render context and put them in their respective monitor
    ofGLFWWindowSettings settings;
    settings.title = "Control Window";
    settings.monitor = 0;
 
   settings.windowMode = OF_GAME_MODE;
    settings.decorated = false;
    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
    mainWindow->setWindowPosition(0, 0);
   // mainWindow->setFullscreen(true);
    
    settings.title = "Projection Window";
    settings.shareContextWith = mainWindow;
    settings.monitor = 1;
   // settings.setSize(1280, 720);
    shared_ptr<ofAppBaseWindow> projectorWindow = ofCreateWindow(settings);
    projectorWindow->setVerticalSync(true);
   // projectorWindow->setWindowPosition(1281,0);
   // projectorWindow->setFullscreen(true);
    
    
    shared_ptr<ofApp> mainApp(new ofApp);
    mainApp->setupProjector();
    ofAddListener(projectorWindow->events().draw,mainApp.get(),&ofApp::drawProjector);
    
    ofRunApp(mainWindow, mainApp);
    ofRunMainLoop();
    
}
