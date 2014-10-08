#include "ofApp.h"

using namespace ofxCv;

void ofApp::setup() {
#ifdef TARGET_OSX
	//ofSetDataPathRoot(".../data/");
#endif
	ofSetVerticalSync(true);
    //ofSetFrameRate(30);
	cloneReady = false;
	cam.initGrabber(640, 360);
	clone.setup(cam.getWidth(), cam.getHeight());
	ofFbo::Settings settings;
	settings.width = cam.getWidth();
	settings.height = cam.getHeight();
	maskFbo.allocate(settings);
	srcFbo.allocate(settings);
    
    largeFbo.allocate(640,360);
    crossfadeFbo.allocate(500, 500, GL_RGBA);
	camTracker.setup();
	srcTracker.setup();
	srcTracker.setIterations(25);
	srcTracker.setAttempts(4);
    

	//faces.allowExt("jpg");
	//faces.allowExt("png");
    
    //alternate version where we don't use syphon input as a mask texture, but use images instead
	faces.listDir("faces");
    cout<< "Loaded face count: " << faces.size() <<endl;
    for (int i=0; i<faces.size(); i++){
        masks.push_back(faces.getPath(i));
    }
    
	currentFace = 0;
    
    crossfadeFbo.begin();
    ofClear(0, 0, 0, 0);
    crossfadeFbo.end();
    
    srcFbo.begin();
    ofClear(0, 0, 0, 0);
    srcFbo.end();
    
    //FBO 
    ofPoint camSize;
    camSize.x = 640;
    camSize.y = 480;
    fboSyphonIn.allocate(camSize.x, camSize.y, GL_RGB);
   // fboSyphonOut.allocate(camSize.x, camSize.y, GL_RGB);
    
    pix.allocate(camSize.x, camSize.y, 3);

    
    
    /*
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}*/
    XML.loadFile("facePoints.xml");
    /*
    //This stores the points on the first face so that it can be re-used with unusual images
    //currently stored with elijah wood's face
    
     
    for (int i = 0; i < srcPoints.size(); i++) {
        int tagNum = XML.addTag("PT");
        XML.setValue("PT:X", srcPoints[i].x, tagNum);
        XML.setValue("PT:Y", srcPoints[i].y, tagNum);
        cout << srcPoints[i].x << " " <<srcPoints[i].y <<endl;
    }
    
    XML.saveFile("facePoints.xml");
*/
    
    //this is so we can have a pre-loaded mask face from XML - makes it easier to texture map new textures on the face with non-face images
    int totalToRead = XML.getNumTags("PT");

    for(int i = 0; i < totalToRead; i++){
        int x = XML.getValue("PT:X", 0, i);
        int y = XML.getValue("PT:Y", 0, i);
        srcPoints.push_back(ofVec2f(x,y));
        inputSrcPoints.push_back(ofVec2f(x,y));
    }
    
    //GUI--------------------
    gui.setup ("Panel");
    gui.add ( screenCapTimer.setup("Save_Time", 15, 10, 60));
    gui.add ( faceTimer.setup("Face_Time", 10, 5, 60));
    gui.add ( texCoordX.setup("txCoordX offset", 0, -250, 250));
    gui.add ( texCoordY.setup("txCoordY offset", 0, -250, 250));
    //gui.add ( xFade.setup("xFade", 127, 0, 255));
    gui.add ( texCoordXScale.setup("txCoordX scale", 1, 0, 4));
    gui.add ( texCoordYScale.setup("txCoordY scale", 1, 0, 4));
    gui.add ( faceNoise.setup("Noise Speed", 0.00, 0, 0.2));
        gui.add ( faceNoiseScale.setup("Noise_scale", 40, 0, 300));
       gui.add ( cloneStrength.setup("Clone Strength", 16, 0, 30));
    gui.add ( saveImage.setup("SaveImage", false));
        gui.add ( syphonMaskSource.setup("Syphon Mask Source", true));
    genericTime = 0;
    
    xFade = 255;

    drawGui = true;
    
    
    syphonInput.setup();
    syphonInput.setApplicationName("VDMX5");
    syphonInput.setServerName("mask_layer");
    syphonInputCam.setup();
    syphonInputCam.setApplicationName("VDMX5");
    syphonInputCam.setServerName("camera_layer");
}
//------------------------------------------------------------
void ofApp::update() {
    
    //movie.idleMovie();
	//cam.update();
    
	//if(cam.isFrameNew()) {
        
        //ofPixels pix;
        // ofTexture texTest;
        //texTest.allocate(syphonInput.texReturn().getTextureData());
        // texTest = syphonInput.texReturn();
        // texTest.allocate(cam.getWidth(), cam.getHeight(), GL_RGB);
        //texTest.readToPixels(pix);
    
    
    
        ofImage imageCopy;
        imageCopy.setFromPixels(pix);
    
        //pass fbo of syphon feed into face tracker - ofSyphon doesn't currently have a way to frame sync (no syphon.getframenew), so you may end up with missing frames or other glitches with using the tracker
		camTracker.update(toCv(imageCopy));
		
		cloneReady = camTracker.getFound(); //is there a face that has been found in frame?
		if(cloneReady) {
			ofMesh camMesh = camTracker.getImageMesh();
			camMesh.clearTexCoords();
            for(int i=0; i< srcPoints.size(); i++){
                
                //add some offsets noise to the texture map
                camMesh.addTexCoord(ofVec2f(texCoordX + srcPoints[i].x*(texCoordXScale) + faceNoiseScale*ofSignedNoise((faceNoise*i)*ofGetElapsedTimef()),
                                            texCoordY + srcPoints[i].y*(texCoordYScale) + faceNoiseScale*ofSignedNoise((faceNoise*i)*ofGetElapsedTimef()))); //face points from droppedin image
            }

			maskFbo.begin();
			ofClear(0, 255);
			camMesh.draw();
			maskFbo.end();
			
			srcFbo.begin();
			ofClear(0, 255);
            if(syphonMaskSource){
                syphonInput.bind();
                
                camMesh.draw();
                syphonInput.unbind();
            }else{

                crossfadeFbo.getTextureReference().bind();
                //cam.getTextureReference().bind(); //uncomment these to use live video as texture
                camMesh.draw();
                crossfadeFbo.getTextureReference().unbind();
                //cam.getTextureReference().unbind();
     
            }

			srcFbo.end();
			
            
			clone.setStrength(cloneStrength);
			//clone.update(srcFbo.getTextureReference(), cam.getTextureReference(), maskFbo.getTextureReference());
            clone.update(srcFbo.getTextureReference(), fboSyphonIn.getTextureReference(), maskFbo.getTextureReference());
		}
	//}
    
    if(ofGetElapsedTimef()> genericTime+faceTimer){ //change to a new face every X seconds
        genericTime = ofGetElapsedTimef();
        currentFace++;
        cout << "New Face 4 U" <<endl;
        currentFace = currentFace%faces.size();
        src = masks[currentFace];
         xFade = 255;
        //if(faces.size()!=0){
          //  loadFace(faces.getPath(currentFace)); //grab new face
        //}
    }
    xFade--;
    xFade = ofClamp(xFade, 0, 255);
    
    //Save screenshots if there has been enough time in between and there is a face in frame
    if(ofGetElapsedTimef()>genericScreenCapTimer+screenCapTimer && camTracker.getFound() && saveImage){
        cout<<"Time to save another face!"<<endl;
        genericScreenCapTimer = ofGetElapsedTimef();
        screenShot = true;
    }  
}
//------------------------------------------------------------
void ofApp::draw() {
	ofSetColor(255);
    ofEnableAlphaBlending();
    
    //1. Draw the syphon output image to an FBO...this seems to work better if in draw instead of update
    ofSetColor(255);
    fboSyphonIn.begin();    //start capturing your FBO
    ofClear(0,0,0,0);       //clear it
    syphonInputCam.draw(0,0);  //draw the incompatible syphon texture to an FBO. Syphon texture cannot be used with read to pixels because it is a different texture type than ofTexture's internal type
    fboSyphonIn.end();                      
    
    //2. Take that FBO reference image (ie the image input) and copy it to some pixels
    //We have to use pixels here because CV operations cannot be performed on textures
    fboSyphonIn.getTextureReference().readToPixels(pix);//these will be read in later in update
    
    crossFade(); //crossfade between the different images
    
    ofSetColor(255);
    //Draw everything into FBO so you don't have to deal with scaling other pieces...ofScale would work as well
    largeFbo.begin();
	if(src.getWidth() > 0 && cloneReady) {
		clone.draw(0, 0);
	} else {
		syphonInputCam.draw(0, 0);
	}
    
    largeFbo.end();
    
    largeFbo.draw(0, 0, ofGetWidth(), ofGetHeight());
    
    //syphonInput.draw(0, 0, 640, 480);
    //syphonInputCam.draw(640, 0, 640, 480);
    
    //For saving screenshot
    if(screenShot){
        screenshot.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
        //screenshot.saveImage("saveFace/"+ofToString(ofGetSystemTime())+".jpg");
        ofRect(0,0,ofGetWidth(), ofGetHeight()); //cam flash
        screenShot = false;
        cout<<"FACE SAVED"<<endl;
    }
    
    //Show/hide gui
    if(drawGui){
        //screenshot.draw(200, ofGetHeight()-200, 260,200);
        float timeDiff;
        timeDiff = ofGetElapsedTimef() - genericScreenCapTimer;
        drawHighlightString("Time since last face save: " +  ofToString(timeDiff), 10, 350);
        
        //crossfadeFbo.draw(0,ofGetHeight()-200, 200, 200);
        //src.draw(0,ofGetHeight()-200, 200, 200);
        ofMesh faceMesh;
        
        int n = srcPoints.size();
        for(int i = 0; i < n; i++) {
            faceMesh.addVertex(ofVec3f(srcPoints[i].x, srcPoints[i].y,0));
        }
        
        ofSetColor(255);
        ofPushMatrix();
        ofTranslate(0,ofGetHeight()-400);
        //faceMesh.drawWireframe();
        ofPopMatrix();
        
        gui.draw();
        if(!camTracker.getFound()) {
            drawHighlightString("Live Face Not Found", 10, 300);
        }
        if(!srcTracker.getFound()) {
            drawHighlightString("image face not found", 10,325);
        }
        ofDrawBitmapString(ofToString(ofGetFrameRate()), 200, 20);
        
        ofDrawBitmapString("Send Camera from VDMX5 to layer named: \"camera_layer\"",20,380);
        ofDrawBitmapString("Send Mask from VDMX5 to layer named: \"mask_layer\"",20,400);
    }
}
void ofApp::crossFade(){
    
    crossfadeFbo.begin();
    ofClear(0,255); //clear previous contents
    ofSetColor(255, 255, 255, xFade);
    masks[currentFace].draw(0, 0, 500,500);
    ofSetColor(255, 255, 255, 255-xFade);
    masks[(currentFace+1)%masks.size()].draw(0, 0, 500,500);
    crossfadeFbo.end();
    
}
//------------------------------------------------------------
void ofApp::loadFace(string face){
    
    //NOT THREADED - SLOWS DOWN APP
	src.loadImage(face);
    crossfadeFbo.begin();
    crossfadeFbo.end();
    
	if(src.getWidth() > 0 && src.isAllocated()) {
		srcTracker.update(toCv(src)); //convert source image to opencv acceptable format
        if(srcTracker.getFound()){
            srcPoints = srcTracker.getImagePoints(); //load the vector of points that are returned from the tracker as the proper face points
            //srcMesh = srcTracker.getImageMesh();
        }
        else{
            srcPoints = inputSrcPoints;
        }
	}
}
//------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
	loadFace(dragInfo.files[0]); //grab the first file
}

void ofApp::keyPressed(int key){
	switch(key){
	case OF_KEY_UP:
		currentFace++;
		break;
	case OF_KEY_DOWN:
		currentFace--;
		break;
    case 'd':
        drawGui = !drawGui;
    }
    
	currentFace = currentFace%faces.size();
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
}
