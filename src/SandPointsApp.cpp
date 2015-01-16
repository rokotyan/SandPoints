#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/Rand.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Capture.h"
#include "cinder/MayaCamUI.h"

#include "ciUI.h"
#include "CinderOpenCV.h"

#include "Auxilary.hpp"
#include "Geometry.hpp"
#include "Palette.hpp"
#include "Particle.hpp"
#include "ParticleController.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;


class SandPointsApp : public AppBasic {
  public:
    void prepareSettings( Settings* settings );
	void setup();
	void mouseDown( MouseEvent event );
    void mouseDrag( MouseEvent event );
    void keyDown( KeyEvent event );
	void update();
	void draw();
    void drawConnections();
    void drawPointsOfMovement();
    void getDifferenceImage( Surface* frame1, Surface* frame2, Surface* difference );
    void guiEvent(ciUIEvent *event);
    //void detectBlobs( Surface* image );
    
    int                     mWindowWidth;
    int                     mWindowHeight;
    Vec2i                   mWorldSize;
    int                     mMode; // 0 = 2D, 1 = 3D
    
    Palette                 mPalette;
    
    vector< Particle >      mParticles;
    ParticleController      mParticleController;
        
    Capture                 mCapture;
    Vec2i                   mCaptureSize;
    Vec2f                   mCaptureToWorldScalingFactor;
    Surface                 mCaptureSurface, mCaptureSurfacePrev, mDifferenceSurface;
    gl::Texture             mCaptureTexture, mCaptureTexturePrev, mDifferenceTexture;
    cv::Mat					mCvDiffImage;
    bool                    mCaptureJustStarted;
    float                   mDiffImageTransparency;
    
    gl::Fbo                 mFbo, mFboFinal;
    gl::Texture             mTextureToMap;
    
    vector< Vec2f >         mBlobCenters;
    std::vector< Vec2i >    mPointsOfMovement;
    
    Geometry                mGeometry;
    
    // Camera
    MayaCamUI               mMayaCam;
    CameraPersp             mCamera;
    Vec2f                   mMayaCam2dPos;
    float                   mMayaCamDistance;
    float                   mMayaCamFov;
    
    
    // UI
    ciUICanvas *mGui;
    Vec2i mGuiDims;
};


void SandPointsApp::prepareSettings( Settings *settings )
{
//    NSArray *screenArray = [NSScreen screens];
//    NSScreen *screen = [screenArray objectAtIndex: 0];
//    NSRect screenRect = [screen visibleFrame];
//    
    
    settings->setWindowSize( 2560, 1440 ); // LED CINEMA DISPLAY
    settings->setResizable(false);
    mWorldSize = Vec2i( 2560, 1440 );
    mCaptureSize = Vec2i( 320, 240 );
    mCaptureToWorldScalingFactor = Vec2f( (float)mWorldSize.x/mCaptureSize.x, (float)mWorldSize.y/mCaptureSize.y );
    printf("Capture to World Scaling Factor: %4.1f  %4.1f\n", mCaptureToWorldScalingFactor.x, mCaptureToWorldScalingFactor.y );
    try {
		mCapture = Capture( mCaptureSize.x, mCaptureSize.y );
		mCapture.start();
	}
	catch( ... ) {
		console() << "Failed to initialize capture" << std::endl;
	}
}

void SandPointsApp::setup()
{
    
    gMultFactorSquared = mWorldSize.x * mWorldSize.y / (640*480);
    gMultFactor = sqrt( gMultFactorSquared );
    mWindowWidth = app::getWindowWidth();
    mWindowHeight = app::getWindowHeight();
    mMode = 0;
    //cout << "Global Scaling Factor: " << gMultFactor << endl;
    
    mPalette = Palette( "pal-01.png");
    mParticleController = ParticleController( &mParticles , mWorldSize );
    mParticleController.addRandomParticles(250, &mPalette);
    
    mCaptureSurface = Surface(320, 240, false);
    mCaptureSurfacePrev = Surface(320, 240, false);
    mDifferenceSurface = Surface(320, 240, false);
    mCaptureJustStarted = true;
    mDiffImageTransparency = 0.15f;
    
    gl::Fbo::Format format;
	mFbo = gl::Fbo( mWorldSize.x, mWorldSize.y, format );
    mFbo.bindFramebuffer();
    gl::clear( ColorA( 0, 0, 0 ,0 ) );
    gl::enableAlphaBlending();
    mFbo.unbindFramebuffer();
    mFboFinal = gl::Fbo( mWorldSize.x, mWorldSize.y, format );
    mFboFinal.bindFramebuffer();
    gl::clear( ColorA( 0, 0, 0 ,0 ) );
    gl::enableAlphaBlending();
    mFboFinal.unbindFramebuffer();
    
    // Geometry
    float mainPartPercentage = 0.75f;
    float verticalGapPercemtage = 0.3f;
    mGeometry = Geometry( mWorldSize, mainPartPercentage, verticalGapPercemtage );
    mGeometry.setTexture( &mTextureToMap );
    
    // Camera
    mMayaCamDistance = 650.f;
    mMayaCam2dPos = Vec2f( mainPartPercentage*mWindowWidth/2, mWindowHeight/2);
    mMayaCamFov   = 90;
    mCamera.setEyePoint( Vec3f( mMayaCam2dPos.x, mWindowHeight - mMayaCam2dPos.y, mMayaCamDistance ));
	mCamera.setCenterOfInterestPoint( Vec3f( mMayaCam2dPos.x, mWindowHeight - mMayaCam2dPos.y, 0 ) );
	mCamera.setPerspective( mMayaCamFov, getWindowAspectRatio(), 1.0f, 6000.0f );
    mCamera.setFarClip(6000.f);
    
//    mCamera.setEyePoint( Vec3f( (Vec2f)getWindowSize()/2.f,-650.f ));
//	mCamera.setCenterOfInterestPoint( Vec3f( (Vec2f)getWindowSize()/2 ) );
//    cam.setWorldUp( Vec3f(0,1,0) );
	//mMayaCam.setCurrentCam( mCamera );
    
    
    // GUI
    float guiElemLength = 320-CI_UI_GLOBAL_WIDGET_SPACING*2;
    mGui = new ciUICanvas(0,0,320,mWindowHeight);
    mGui->addWidgetDown(new ciUILabel("SAND POINTS", CI_UI_FONT_LARGE));
    mGui->addWidgetDown(new ciUISpacer(guiElemLength, 2), CI_UI_ALIGN_LEFT);
    mGui->addWidgetDown(new ciUISlider(guiElemLength, 20.f, 0.f, 1.f, &mDiffImageTransparency, "CAMERA BRIGHTNESS"), CI_UI_ALIGN_LEFT );
    vector<string> modes; modes.push_back("2D"); modes.push_back("3D");
    ciUIRadio* modeSelectorWidget =  new ciUIRadio(30, 30, "MODE SELECTOR", modes, CI_UI_ORIENTATION_HORIZONTAL);
    mGui->addWidgetDown( modeSelectorWidget );
    modeSelectorWidget->activateToggle("2D");
    
    
    mGui->addWidgetDown(new ciUI2DPad( guiElemLength, guiElemLength*.75, Vec2f( 0, mWindowWidth*mainPartPercentage ), Vec2f(0, mWindowHeight) , &mMayaCam2dPos, "CAMERA 2D POS"), CI_UI_ALIGN_LEFT);
    mGui->addWidgetDown(new ciUISlider(guiElemLength, 20.f, 250.f, 1000.f, &mMayaCamDistance, "CAMERA DISTANCE"), CI_UI_ALIGN_LEFT );
    mGui->addWidgetDown(new ciUISlider(guiElemLength, 20.f, 45.f, 120.f, &mMayaCamFov, "CAMERA FOV"), CI_UI_ALIGN_LEFT );
    
    mGui->getWidget("CAMERA 2D POS")->setVisible( false );
    mGui->getWidget("CAMERA DISTANCE")->setVisible( false );
    mGui->getWidget("CAMERA FOV")->setVisible( false );
    //mGui->setTheme(CI_UI_THEME_MINBLACK);
    
    
    
    mGui->registerUIEvents(this, &SandPointsApp::guiEvent);
    
    gl::clear( Color( 0,0,0 ) );

}

void SandPointsApp::mouseDrag( MouseEvent event )
{
    if ( event.getPos().x < 320 )
        return;
    
    //mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void SandPointsApp::mouseDown( MouseEvent event )
{
    if ( event.getPos().x < 320 )
        return;
    
    //mMayaCam.mouseDown( event.getPos() );
}

void SandPointsApp::keyDown( KeyEvent event )
{
    if ( event.getChar() == 'V' || event.getChar() == 'v' )
        mGui->toggleVisible();
    else if ( event.getChar() == 'F' || event.getChar() == 'f' ) {
        app::setFullScreen( !app::isFullScreen() );
        mWindowHeight = app::getWindowHeight();
        mWindowWidth  = app::getWindowWidth();
    }
}
void SandPointsApp::update()
{
    // Update GUI
    mGui->update();
    
    // Capture Update
    if( mCapture && mCapture.checkNewFrame() ) {
        if ( mCaptureJustStarted ) {
            mCaptureSurface = mCapture.getSurface();
            mCaptureJustStarted = false;
        }
        else {
            mCaptureSurfacePrev = mCaptureSurface;
            mCaptureSurface = mCapture.getSurface();
            
            
            mCaptureTexture = gl::Texture( mCaptureSurface );
            mCaptureTexturePrev = gl::Texture( mCaptureSurfacePrev );
            getDifferenceImage( &mCaptureSurface, &mCaptureSurfacePrev, &mDifferenceSurface);
            mDifferenceTexture = gl::Texture( mDifferenceSurface );
            
            
            mCvDiffImage = toOcv( Channel( mDifferenceSurface ) );
            
            cv::Mat binDiffImage;
            cv::threshold( mCvDiffImage, binDiffImage, 40, 255, CV_THRESH_BINARY );

            // Detect Blobs
            mBlobCenters.clear();
            for ( int y = 0; y < binDiffImage.rows; ++y )
                for ( int x = 0; x < binDiffImage.cols; ++x )
                {
                    int value = binDiffImage.at<uchar>(y ,x);
                    if ( value == 255 ) {
                        cv::Rect rect;
                        cv::floodFill( binDiffImage, cv::Point( x, y ), cv::Scalar( 200 ), &rect );
                        if ( rect.width > 5 && rect.width < 400 && rect.height > 5 && rect.height < 400 ) {
                            mBlobCenters.push_back( Vec2f( mCaptureToWorldScalingFactor.x * (float)(rect.x+rect.width/2), mCaptureToWorldScalingFactor.y * (float)(rect.y+rect.height/2) ) );
                        }
                    }
                }
            
        }
	}
    
    // Particles Update
    mParticleController.update();
    
    // Interactions with the world
    for ( std::vector< Particle >::iterator p = mParticles.begin(); p != mParticles.end(); ++p )
    {
        if  ( p->pos.x < 0 ) { p->vel.x = abs(p->vel.x); p->pos.x = 0; }
        if  ( p->pos.x > mWorldSize.x ) { p->vel.x = -abs(p->vel.x); p->pos.x = mWorldSize.x; }
        if  ( p->pos.y < 0 ) { p->vel.y = abs(p->vel.y); p->pos.y = 0; }
        if  ( p->pos.y > mWorldSize.y ) { p->vel.y = -abs(p->vel.y); p->pos.y = mWorldSize.y; }
    }
    
    // Interaction with movement
    for (std::vector< Vec2f>::iterator blob = mBlobCenters.begin(); blob != mBlobCenters.end(); ++blob )
        for ( std::vector< Particle >::iterator p = mParticles.begin(); p != mParticles.end(); ++p ) {
            float distMotion = blob->distance( p->pos );
            if ( distMotion > 0 && distMotion < 250*gMultFactor ) {
                float globAngle = atan2( blob->y - p->pos.y, blob->x - p->pos.x );
                p->vel.x += 2 * ( 4.f / (distMotion) ) * cos( globAngle ) * Rand::randFloat( 0.f, 20.f );
                p->vel.y += 2 * ( 4.f / (distMotion) ) * sin( globAngle ) * Rand::randFloat( 0.f, 20.f );
            }

        }
    
    // Remove random particle and add random one
    if ( app::getElapsedFrames()%3 == 0 )
        mParticleController.changeRandomParticle( &mPalette );
}

void SandPointsApp::draw()
{
    gl::clear();
    
    // FBO DRAWING STARTED --------------------------------------------------------------------------
    mFbo.bindFramebuffer();
    gl::setMatricesWindow( mWorldSize );
    gl::color( ColorA(0,0,0,0.14)) ;
    gl::drawSolidRect( Rectf( 0, 0, mWorldSize.x, mWorldSize.y) );

    gl::enableAdditiveBlending();
    gl::color( ColorA(1,1,1,mDiffImageTransparency)) ;
    if ( mDifferenceTexture )
        gl::draw( mDifferenceTexture, Rectf( 0, 0, mWorldSize.x, mWorldSize.y ) );
    gl::enableAlphaBlending();
    
    glLineWidth( 3 );
    glPointSize( 5 );
    mParticleController.draw();
    this->drawConnections();

    mFbo.unbindFramebuffer();
    // FBO DRAWING FINISHED -------------------------------------------------------------------------
    
    mFboFinal.bindFramebuffer();
    Vec2i mFboSize = mFbo.getSize();
    gl::setMatricesWindow( mWorldSize );
    gl::color( 1,1,1 );
    gl::Texture texture = mFbo.getTexture();
//    gl::draw( texture, Rectf( 0, 0, mWorldSize.x/2, mWorldSize.y/2 ) );
    gl::draw( texture, Area( Vec2i::zero(), 0.8*mFboSize ), Rectf( mWorldSize.x, 0, mWorldSize.x/2, mWorldSize.y/2  ) );
    gl::draw( texture, Area( Vec2i::zero(), 0.8*mFboSize ), Rectf( 0, mWorldSize.y, mWorldSize.x/2, mWorldSize.y/2  ) );
    gl::draw( texture, Area( Vec2i::zero(), 0.8*mFboSize ), Rectf( mWorldSize.x, mWorldSize.y, mWorldSize.x/2, mWorldSize.y/2  ) );
    gl::draw( texture, Area( Vec2i::zero(), 0.8*mFboSize ), Rectf( 0, 0, mWorldSize.x/2, mWorldSize.y/2 ) );
    mFboFinal.unbindFramebuffer();
    

    
    if (mMode == 0) {
        gl::setMatricesWindow( app::getWindowSize() );
        texture = mFboFinal.getTexture();
        gl::draw( texture, Rectf( 0, 0, mWindowWidth, mWindowHeight ) );
    }
    else if (mMode == 1) {
        gl::pushMatrices();
        gl::setMatrices( mCamera );
        mTextureToMap = mFboFinal.getTexture();
        mGeometry.draw();
        gl::popMatrices();
    }
    
    
    // Draw GUI
    gl::setMatricesWindow( app::getWindowSize() );
    mGui->draw();
    
}

void SandPointsApp::drawConnections()
{
    // Draw connections
    // Prepare GL Arrays
    vector< Vec2f > linesToDraw;
    vector< Color > linesToDrawColors;
    size_t numParticles = mParticles.size();
    for ( size_t i = 0; i < numParticles; ++i ) {
        for ( size_t j = i + 1; j < numParticles; ++j ) {
            float distance = abs( mParticles[i].pos.x - mParticles[j].pos.x ) + abs( mParticles[i].pos.y - mParticles[j].pos.y );
            float minDistance = 7*( mParticles[i].radius + mParticles[j].radius )/gMultFactor;
            if ( distance < minDistance ) {
                linesToDraw.push_back( mParticles[i].pos );
                linesToDraw.push_back( mParticles[j].pos );
                linesToDrawColors.push_back( mParticles[i].color );
                linesToDrawColors.push_back( mParticles[j].color );
                
            }
        }
    }
    
    // Draw GL Arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glColorPointer( 3, GL_FLOAT, 0, &linesToDrawColors.front().r );
    glVertexPointer( 2, GL_FLOAT, 0, &linesToDraw.front().x );
    
    glDrawArrays( GL_LINES, 0, linesToDraw.size() );
    
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void SandPointsApp::drawPointsOfMovement()
{
    // Draw Points of Movemens
    glPointSize(5.f);
    glColor3f( 1.f, 1.f, 0.f );
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer( 2, GL_FLOAT, 0, &mBlobCenters.front().x );
    glDrawArrays( GL_POINTS, 0, mBlobCenters.size() );
    glDisableClientState(GL_VERTEX_ARRAY);
    
}

void SandPointsApp::getDifferenceImage( Surface* frame1, Surface* frame2, Surface* difference )
{
    Surface::Iter iter1 = frame1->getIter();
    Surface::Iter iter2 = frame2->getIter();
    Surface::Iter diffIter = difference->getIter();
    while( iter1.line() && iter2.line() && diffIter.line() ) {
        while( iter1.pixel() && iter2.pixel() && diffIter.pixel() ) {
            diffIter.r() = abs( iter1.r() - iter2.r() );
            diffIter.g() = abs( iter1.g() - iter2.g() );
            diffIter.b() = abs( iter1.b() - iter2.b() );
        }
    }
}

void SandPointsApp::guiEvent(ciUIEvent *event)
{
    string name = event->widget->getName();

    if ( name == "CAMERA DISTANCE" ) {
        mCamera.setEyePoint( Vec3f( mMayaCam2dPos.x, mWindowHeight - mMayaCam2dPos.y, mMayaCamDistance ) );
    }
    else if ( name == "CAMERA FOV" ) {
        mCamera.setFov( mMayaCamFov );
    }
    else if ( name == "CAMERA 2D POS" ) {
        mCamera.setEyePoint( Vec3f( mMayaCam2dPos.x, mWindowHeight - mMayaCam2dPos.y, mMayaCamDistance ));
        mCamera.setCenterOfInterestPoint( Vec3f( mMayaCam2dPos.x, mWindowHeight - mMayaCam2dPos.y, 0 ) );
    }
    else if ( name == "3D" ) {
        mGui->getWidget("CAMERA 2D POS")->setVisible( true );
        mGui->getWidget("CAMERA DISTANCE")->setVisible( true );
        mGui->getWidget("CAMERA FOV")->setVisible( true );
        mMode = 1;
    }
    else if ( name == "2D" ) {
        mGui->getWidget("CAMERA 2D POS")->setVisible( false );
        mGui->getWidget("CAMERA DISTANCE")->setVisible( false );
        mGui->getWidget("CAMERA FOV")->setVisible( false );
        mMode = 0;
    }

}

CINDER_APP_BASIC( SandPointsApp, RendererGl )
