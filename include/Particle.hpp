//
//  Particle.h
//  SandPoints
//
//  Created by Nikita Rokotyan
//
//

#ifndef SandPoints_Particle_h
#define SandPoints_Particle_h

#include "cinder/app/AppBasic.h"
using namespace ci;
using namespace ci::app;

class Particle {
public:
    Particle() {};
    
    Particle( Vec2f pos, Vec2f vel, float radius, Color color) {
        this->pos = pos;
        this->vel = vel;
        this->radius = radius;
        this->color = color;
    };
    void update()    {
        
        if ( vel.x > 6 ) vel.x = sgn( vel.x ) * 6; // Everything like in the old Sand Points App on Processing
        if ( vel.y > 6 ) vel.y = sgn( vel.y ) * 6; // :)
        if ( vel.lengthSquared() < 0.09 * gMultFactorSquared / 9 )
            vel = vel*12 + Rand::randVec2f();
        
        pos += vel;
        vel *= randFloat( 0.9f, 0.99f); // Orig value = 0.90f
    }
    void draw() {
        gl::color( color );
        gl::drawStrokedCircle( pos, radius);
    }
    
    float distanceApproximate( vector< Particle >::const_iterator p ) const { return  abs(pos.x - p->pos.x) + abs(pos.y - p->pos.y); }
    float distanceApproximate( Particle* p ) const { return  abs(pos.x - p->pos.x) + abs(pos.y - p->pos.y); }
    //return pos.distance( p->pos );
    
    
    Vec2f   pos, vel;
    float   radius;
    Color   color;
};



#endif
