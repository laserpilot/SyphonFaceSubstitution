#include "ofApp.h"

//Add syphon
//Add blackmagic
//add grab frames from webcam when face is found

using namespace ofxCv;

void testApp::setup() {
#ifdef TARGET_OSX
	//ofSetDataPathRoot("../data/");
#endif
    
    
	ofSetVerticalSync(true);
	cloneReady = false;
    camSize.x = 1280;
    camSize.y = 720;
    
	cam.initGrabber(1280, 720);
	clone.setup(cam.getWidth(), cam.getHeight());
	ofFbo::Settings settings;
	settings.width = cam.getWidth();
	settings.height = cam.getHeight();
	maskFbo.allocate(settings);
	srcFbo.allocate(settings);
	camTracker.setup();
	srcTracker.setup();
	srcTracker.setIterations(25);
	srcTracker.setAttempts(4);

	faces.allowExt("jpg");
	faces.allowExt("png");
	faces.listDir("faces");
	currentFace = 0;
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
    
    syphonCapture.allocate(camSize.x, camSize.y, GL_RGB);
    pix.allocate(camSize.x, camSize.y, 3);
    
    syphonIn.setApplicationName("VDMX5");
    syphonIn.setServerName("Layer 1");

}

void testApp::update() {
    
    syphonCapture.begin();
    syphonIn.draw(0,0);
    syphonCapture.end();
    
//	cam.update();
//	if(cam.isFrameNew()) {
		//camTracker.update(toCv(cam));
        
        ofImage imageCopy;
        imageCopy.setFromPixels(pix);
        
        camTracker.update(toCv(imageCopy));
		
		cloneReady = camTracker.getFound();
		if(cloneReady) {
			ofMesh camMesh = camTracker.getImageMesh();
			camMesh.clearTexCoords();
			camMesh.addTexCoords(srcPoints);
			
			maskFbo.begin();
			ofClear(0, 255);
			camMesh.draw();
			maskFbo.end();
			
			srcFbo.begin();
			ofClear(0, 255);
			src.bind();
			camMesh.draw();
			src.unbind();
			srcFbo.end();
			
			clone.setStrength(16);
			clone.update(srcFbo.getTextureReference(), cam.getTextureReference(), maskFbo.getTextureReference());
		}
	}
}

void testApp::draw() {
	ofSetColor(255);
	
	if(src.getWidth() > 0 && cloneReady) {
		clone.draw(0, 0);
	} else {
		cam.draw(0, 0);
	}
	
	if(!camTracker.getFound()) {
		drawHighlightString("camera face not found", 10, 10);
	}
	if(src.getWidth() == 0) {
		drawHighlightString("drag an image here", 10, 30);
	} else if(!srcTracker.getFound()) {
		drawHighlightString("image face not found", 10, 30);
	}
}

void testApp::loadFace(string face){
	src.loadImage(face);
	if(src.getWidth() > 0) {
		srcTracker.update(toCv(src));
        srcPoints = srcTracker.getImagePoints();
        
	}
}

void testApp::dragEvent(ofDragInfo dragInfo) {
	loadFace(dragInfo.files[0]);
}

void testApp::keyPressed(int key){
	switch(key){
	case OF_KEY_UP:
		currentFace++;
		break;
	case OF_KEY_DOWN:
		currentFace--;
		break;
	}
	currentFace = ofClamp(currentFace,0,faces.size());
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
}

void testApp::exit(){

}
