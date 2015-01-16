//
//  Palette.hpp
//  SandPoints
//
//  Created by Nikita Rokotyan on 16.01.15.
//
//

#ifndef SandPoints_Palette_hpp
#define SandPoints_Palette_hpp

#include "cinder/app/AppBasic.h"
#include "cinder/ImageIo.h"

using namespace ci;
using namespace ci::app;

class Palette {
public:
    Palette() {};
    Palette( string filename ){
        Surface image = loadImage( loadResource( filename ) );
        Surface::Iter iter = image.getIter();
        while( iter.line() )
            while( iter.pixel() )
                addToVector( colors, Color( iter.r()/255.f, iter.g()/255.f, iter.b()/255.f ) );
        
    }
    
    
    Color   getRandomColor() {
        return colors[ rand() % colors.size() ]; }
    
    vector< Color > colors;
    
};

#endif
