//
//  ParticleController.hpp
//  SandPoints
//
//  Created by Nikita Rokotyan
//
//

#ifndef SandPoints_ParticleController_hpp
#define SandPoints_ParticleController_hpp

#include "cinder/app/AppBasic.h"
using namespace ci;
using namespace ci::app;


class ParticleController {
public:
    ParticleController() {};
    ParticleController( vector< Particle > *particlesPtr, Vec2i worldSize ) { this->particlesPtr = particlesPtr; this->mWorldSize = worldSize; }
    void    addRandomParticles( int number, Palette *palette )
    {
        doCollide = true;
        for ( int i = 0; i < number; ++i )
        {
            Vec2f randomPos = Vec2f( randInt( 0, mWorldSize.x ), randInt( 0, mWorldSize.y ) );
            Vec2f randomVel = Vec2f( randFloat( -3.f, 3.f ), randFloat( -3.f, 3.f ) )/2;
            float randomRadius = randFloat( 5 * gMultFactor, 8 * gMultFactor );
            Color randomColor = palette->getRandomColor();
            particlesPtr->push_back( Particle(randomPos, randomVel, randomRadius, randomColor ) );
        }
    }
    void changeRandomParticle( Palette *palette )
    {
        Vec2f randomPos = Vec2f( randInt( 0, mWorldSize.x ), randInt( 0, mWorldSize.y ) );
        Vec2f randomVel = Vec2f( randFloat( -3.f, 3.f ), randFloat( -3.f, 3.f ) )/2;
        float randomRadius = randFloat( 4 * gMultFactor, 8 * gMultFactor );
        Color randomColor = palette->getRandomColor();
        
        particlesPtr->at( randInt(0, particlesPtr->size()) ) = Particle(randomPos, randomVel, randomRadius, randomColor );
    }
    void drawParticles() {
        glPointSize( 4.f );
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        
        glColorPointer( 3, GL_FLOAT, sizeof(Particle), &particlesPtr->front().color.r );
        glVertexPointer( 2, GL_FLOAT, sizeof(Particle), &particlesPtr->front().pos.x );
        
        glDrawArrays( GL_POINTS, 0, particlesPtr->size() );
        
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        
    }
    void update()
    {
        for ( std::vector< Particle >::iterator p = particlesPtr->begin(); p != particlesPtr->end(); ++p )
            p->update();
        
        if ( doCollide )
            for ( std::vector< Particle >::iterator p1 = particlesPtr->begin(); p1 != particlesPtr->end(); ++p1 )
                for ( std::vector< Particle >::iterator p2 = next( p1 ); p2 != particlesPtr->end(); ++p2 ) {
                    float dist = 1.01f*( p1->pos.distance( p2->pos ) );
                    if ( dist < 1.05f*( p1->radius + p2->radius) ) {
                        float alpha = atan2( p2->pos.y - p1->pos.y, p2->pos.x - p1->pos.x );
                        Vec2f target = Vec2f( p1->pos.x + cos( alpha )*dist, p1->pos.y + sin( alpha )*dist );
                        Vec2f a = 0.99f * (target - p2->pos);
                        p1->vel -= a;
                        p2->vel += a;
                        p1->vel *= 0.95;
                        p2->vel *= 0.95;
                    }
                }
        
    }
    void draw() {
        drawParticles();
        //        for ( std::vector< Particle >::iterator p = particlesPtr->begin(); p != particlesPtr->end(); ++p )
        //        {
        //            p->draw();
        //        }
    }
    
    vector< Particle >  *particlesPtr;
    bool                doCollide;
    Vec2i               mWorldSize;
    
    
};

#endif
