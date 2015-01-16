//
//  Geometry.hpp
//  SandPoints
//
//  Created by Nikita Rokotyan
//
//

#ifndef SandPoints_Geometry_hpp
#define SandPoints_Geometry_hpp

#include "cinder/app/AppBasic.h"
using namespace ci;
using namespace ci::app;

class Geometry {
public:
    Geometry() {};
    Geometry(Vec2i windowDimentions, float mainPartPercentage, float verticalGapPercentage )
    {
        this->mainPartPercentage = mainPartPercentage;
        this->verticalGapPercentage = verticalGapPercentage;
        mainPart[0] = Vec3f( 0, 0, 0 );
        mainPart[1] = Vec3f( + mainPartPercentage * windowDimentions.x, 0, 0 );
        mainPart[2] = Vec3f( + mainPartPercentage * windowDimentions.x, +windowDimentions.y, 0 );
        mainPart[3] = Vec3f( 0, +windowDimentions.y, 0 );
        
        rightPart[0] = Vec3f( + mainPartPercentage * windowDimentions.x, 0, 0);
        rightPart[1] = Vec3f( + mainPartPercentage * windowDimentions.x, 0, (1-mainPartPercentage)*windowDimentions.x/2  );
        rightPart[2] = Vec3f( + mainPartPercentage * windowDimentions.x, +windowDimentions.y, (1-mainPartPercentage)*windowDimentions.x/2  );
        rightPart[3] = Vec3f( + mainPartPercentage * windowDimentions.x, +windowDimentions.y, 0  );
        
        leftBottomPart[0] = Vec3f( 0, 0, 0 );
        leftBottomPart[1] = Vec3f( 0, 0, +(1-mainPartPercentage)*windowDimentions.x/2  );
        leftBottomPart[2] = Vec3f( 0, windowDimentions.y*( 0.5f -  verticalGapPercentage/2), +(1-mainPartPercentage)*windowDimentions.x/2  );
        leftBottomPart[3] = Vec3f( 0, windowDimentions.y*( 0.5f -  verticalGapPercentage/2), 0 );
        
        leftTopPart[0] = Vec3f( 0, windowDimentions.y*( 0.5f +  verticalGapPercentage/2), 0 );
        leftTopPart[1] = Vec3f( 0, windowDimentions.y*( 0.5f +  verticalGapPercentage/2), +(1-mainPartPercentage)*windowDimentions.x/2  );
        leftTopPart[2] = Vec3f( 0, windowDimentions.y, +(1-mainPartPercentage)*windowDimentions.x/2  );
        leftTopPart[3] = Vec3f( 0, windowDimentions.y, 0 );
    }
    void setTexture(gl::Texture *texture) {
        texturePtr = texture;
        
        mainPartTexCoords[0] = Vec2f( 0.5 - (mainPartPercentage)/2, 0.f );
        mainPartTexCoords[1] = Vec2f( 0.5 + (mainPartPercentage)/2, 0.f );
        mainPartTexCoords[2] = Vec2f( 0.5 + (mainPartPercentage)/2, 1.f );
        mainPartTexCoords[3] = Vec2f( 0.5 - (mainPartPercentage)/2, 1.f );
        
        rightPartTexCoords[0] = Vec2f( 0.5 + (mainPartPercentage)/2, 0.f );
        rightPartTexCoords[1] = Vec2f( 1, 0.f );
        rightPartTexCoords[2] = Vec2f( 1, 1.f );
        rightPartTexCoords[3] = Vec2f( 0.5 + (mainPartPercentage)/2, 1.f );
        
        leftTopPartTexCoords[0] = Vec2f( 0.5 - (mainPartPercentage)/2, 0.f );
        leftTopPartTexCoords[1] = Vec2f( 0, 0.f );
        leftTopPartTexCoords[2] = Vec2f( 0, (1-verticalGapPercentage)/2 );
        leftTopPartTexCoords[3] = Vec2f( 0.5 - (mainPartPercentage)/2, (1-verticalGapPercentage)/2  );
        
        leftBottomPartTexCoords[0] = Vec2f( 0.5 - (mainPartPercentage)/2, 1 - (1-verticalGapPercentage)/2);
        leftBottomPartTexCoords[1] = Vec2f( 0, 1 - (1-verticalGapPercentage)/2);
        leftBottomPartTexCoords[2] = Vec2f( 0, 1 );
        leftBottomPartTexCoords[3] = Vec2f( 0.5 - (mainPartPercentage)/2, 1  );
        
    }
    void draw() {
        
        drawFrame();
        
        if ( *texturePtr == NULL )
            return;
        
        gl::color(1,1,1);
        
        gl::SaveTextureBindState saveBindState( (*texturePtr).getTarget() );
        gl::BoolState saveEnabledState( (*texturePtr).getTarget() );
        gl::ClientBoolState vertexArrayState( GL_VERTEX_ARRAY );
        gl::ClientBoolState texCoordArrayState( GL_TEXTURE_COORD_ARRAY );
        (*texturePtr).enableAndBind();
        
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        
        glTexCoordPointer( 2, GL_FLOAT, 0, mainPartTexCoords );
        glVertexPointer( 3, GL_FLOAT, 0, mainPart );
        glDrawArrays( GL_QUADS, 0, 4 );
        
        glTexCoordPointer( 2, GL_FLOAT, 0, rightPartTexCoords );
        glVertexPointer( 3, GL_FLOAT, 0, rightPart );
        glDrawArrays( GL_QUADS, 0, 4 );
        
        glTexCoordPointer( 2, GL_FLOAT, 0, leftTopPartTexCoords );
        glVertexPointer( 3, GL_FLOAT, 0, leftTopPart );
        glDrawArrays( GL_QUADS, 0, 4 );
        
        glTexCoordPointer( 2, GL_FLOAT, 0, leftBottomPartTexCoords );
        glVertexPointer( 3, GL_FLOAT, 0, leftBottomPart );
        glDrawArrays( GL_QUADS, 0, 4 );
        
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        
        
        
    }
    void drawFrame()
    {
        glColor3f( 0.5f, 0.5f, 0.5f );
        glEnableClientState(GL_VERTEX_ARRAY);
        
        glVertexPointer( 3, GL_FLOAT, 0, mainPart );
        glDrawArrays( GL_LINE_LOOP, 0, 4 );
        
        glVertexPointer( 3, GL_FLOAT, 0, rightPart );
        glDrawArrays( GL_LINE_LOOP, 0, 4 );
        
        glVertexPointer( 3, GL_FLOAT, 0, leftTopPart );
        glDrawArrays( GL_LINE_LOOP, 0, 4 );
        
        glVertexPointer( 3, GL_FLOAT, 0, leftBottomPart );
        glDrawArrays( GL_LINE_LOOP, 0, 4 );
        
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    
    Vec3f   mainPart[4];
    Vec2f   mainPartTexCoords[4];
    Vec3f   leftTopPart[4], leftBottomPart[4];
    Vec2f   leftTopPartTexCoords[4], leftBottomPartTexCoords[4];
    Vec3f   rightPart[4];
    Vec2f   rightPartTexCoords[4];
    
    float mainPartPercentage;
    float verticalGapPercentage;
    gl::Texture* texturePtr;
};


#endif
