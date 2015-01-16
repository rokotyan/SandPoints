//
//  Auxilary.h
//  SandPoints
//
//  Created by Nikita Rokotyan
//
//

#ifndef SandPoints_Auxilary_h
#define SandPoints_Auxilary_h


float gMultFactor = 1;
float gMultFactorSquared = 1;

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

template <typename T>
bool addToVector( vector< T > &container, T element, bool ereaseIfFound = false)
{
    for (typename vector< T >::iterator e = container.begin(); e != container.end(); ++e )
        if ( element == *e ) {
            if ( ereaseIfFound ) container.erase( e );
            return false;
        }
    
    container.push_back( element );
    return true;
}

//template <typename Iter>
//Iter next(Iter iter) { return ++iter; }


#endif
