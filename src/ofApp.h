#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "Clone.h"
#include "ofxGui.h"
#include "ofxFaceTracker.h"
#include "ofxFaceTrackerThreaded.h"
#include "ofxSyphon.h"
#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void dragEvent(ofDragInfo dragInfo);
    void loadFace(string face);
    void crossFade();
    
    void keyPressed(int key);
    
    ofxSyphonClient syphonInput;
    ofxSyphonClient syphonInputCam;
    ofxSyphonServer syphonOutput;
    ofxFaceTrackerThreaded camTracker;
    ofVideoGrabber cam;
    
    ofxFaceTracker srcTracker;
    ofImage src;
    vector<ofVec2f> srcPoints;
    vector<ofVec2f> inputSrcPoints;
    ofMesh srcMesh;
    
    bool cloneReady;
    Clone clone;
    ofFbo srcFbo, maskFbo;
    ofFbo largeFbo;
    ofFbo crossfadeFbo;
    
    ofFbo  fboSyphonIn;
    ofPixels pix;
    
    
    ofDirectory faces;
    int currentFace;
    int xFade;
    
    ofxPanel gui;
    ofxIntSlider    faceTimer; //auto switching/fading masks
    ofxIntSlider    screenCapTimer;
    ofxIntSlider    texCoordX; //offset the mask on the mesh
    ofxIntSlider    texCoordY;
    //ofxIntSlider    xFade;
    ofxFloatSlider    texCoordXScale;
    ofxFloatSlider    texCoordYScale;
    ofxFloatSlider    faceNoise;
    ofxFloatSlider    faceNoiseScale;
    ofxIntSlider    cloneStrength;
    ofxToggle        saveImage; //automatically save an image when a new face is found - save every X seconds
    ofxToggle        autoAdvance; //auto crossfade between images in /faces folder
    ofxToggle        syphonMaskSource; //pick if syphon or the list of faces in the folder is a mask source

    
    bool drawGui;
    bool screenShot;
    bool fadeUp;
    bool fadeDown;
    int slot1, slot2;
    
    vector <ofImage> maskFile;
    
    ofImage screenshot;
    vector <ofImage> masks;
    
    float genericTime;
    float genericScreenCapTimer;
    ofxXmlSettings XML;
    int lastTagNumber;
    
    ofVideoPlayer movie; //if you want to play a movie on the face
};
