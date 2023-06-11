// C++ port of Minecraft 4k JS (http://jsdo.it/notch/dB1E)
// By The8BitPimp
// See: the8bitpimp.wordpress.com

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <iostream.h>

//#define math_sin(x) sin(x)
//#define math_cos(x) cos(x)
//#define fxmul(a,b) (a*b)>>8

const int w = 320;
const int h = 200;

//VGA memory address
unsigned char *screen=(unsigned char *)0xA0000;
//SDL_Surface *screen = nullptr;

//const float math_pi = 3.14159265359f;
const int math_pi = 12868;

int sin_table [math_pi * 2];
int cos_table [math_pi * 2];

/*static inline float math_sin( float x ) {
    return sin( x );
}*/

static inline int math_sin( int x ) {
    if( x < 0) x += (math_pi * 2);
    return sin_table[x % (math_pi * 2)];
}

/*static inline float math_cos( float x ) {
    return cos( x );
}*/

static inline int math_cos( int x ) {
    if( x < 0) x += (math_pi * 2);
    return cos_table[x % (math_pi * 2)];
}

// the texture map
int texmap[ 16 * 16 * 16 * 3 ];

// the voxel map
char map[ 64 * 64 * 64 ];

static inline int random( int max ) {
    return (rand()^(rand()<<16)) % max;
}

static inline void plot( int x, int y, unsigned char c ) {
    screen[(y << 6) + (y << 8) + x] = c;
}

static void makeSinTable ( void ) {
    for(int i = 0; i < math_pi; i ++) {
        sin_table[i] = (int)(sin((float)i/4096.f) * 4096);
    }
}

static void makeCosTable ( void ) {
    for(int i = 0; i < math_pi; i ++) {
        cos_table[i] = (int)(cos((float)i/4096.f) * 4096);
    }
}

static void makePalette ( void ) {
    float rgMul = 9.142f;
    float bMul = 21.33f;
    outp(0x03c8, 0);
    for ( int i=0; i<256; i++) {
        unsigned char r = ((i>>5) & 7) * rgMul;
        unsigned char g = ((i>>2) & 7) * rgMul;
        unsigned char b = (i & 3) * bMul;
        outp(0x03c9, r);
        outp(0x03c9, g);
        outp(0x03c9, b);
        
    }
}

static void vgaTest ( void ) {
    for( int i=0; i<200; i++){
        for ( int j=0; j<256; j++) {
            plot(j, i, j);
        }
    }
}

static void makeTextures( void ) {
    
    // each texture
    for ( int j=0; j<16; j++ ) {

        int k = 255 - 96;

        // each pixel in the texture
        for ( int m=0; m<16 * 3; m++ )
            for ( int n = 0; n<16; n++ ) {

                int i1 = 0x966C4A;
                int i2 = 0;
                int i3 = 0;

                if (j == 4)
                    i1 = 0x7F7F7F;
                if ((j != 4) || (random(3) == 0))
                    k = 255 - random(96);
                if ( j == 1 )
                {
                    if (m < (((n * n * 3 + n * 81) >> 2) & 0x3) + 18)
                        i1 = 0x6AAA40;
                    else if (m < (((n * n * 3 + n * 81) >> 2) & 0x3) + 19)
                        k = k * 2 / 3;
                }
                if (j == 7)
                {
                    i1 = 0x675231;
                    if ((n > 0) && (n < 15) && (((m > 0) && (m < 15)) || ((m > 32) && (m < 47))))
                    {
                        i1 = 0xBC9862;
                        i2 = n - 7;
                        i3 = (m & 0xF) - 7;
                        if (i2 < 0)
                            i2 = 1 - i2;

                        if (i3 < 0)
                            i3 = 1 - i3;

                        if (i3 > i2)
                            i2 = i3;

                        k = 196 - random(32) + i2 % 3 * 32;
                    }
                    else if (random(2) == 0)
                        k = k * (150 - (n & 0x1) * 100) / 100;
                }
                if (j == 5)
                {
                    i1 = 0xB53A15;
                    if (((n + m / 4 * 4) % 8 == 0) || (m % 4 == 0))
                        i1 = 0xBCAFA5;
                }
                if (j == 9){
                    i1 = 0x4040FF;
                }
                i2 = k;
                if (m >= 32)
                    i2 /= 2;
                if (j == 8)
                {
                    i1 = 5298487;
                    if (random(2) == 0)
                    {
                        i1 = 0;
                        i2 = 255;
                    }
                }

                // fixed point colour multiply between i1 and i2
                i3 =
                    ((((i1 >> 16) & 0xFF) * i2 / 255) << 16) |
                    ((((i1 >>  8) & 0xFF) * i2 / 255) <<  8) |
                      ((i1        & 0xFF) * i2 / 255);
                // pack the colour away
                texmap[ n + m * 16 + j * 256 * 3 ] = i3;
            }
    }
}

static void makeMap( void ) {
    // add random blocks to the map
        for ( int x = 0; x < 64; x++) {
                for ( int y = 0; y < 12; y++) {
                        for ( int z = 0; z < 64; z++) {
                                int i = (z << 12) | (y << 6) | x;
                                float yd = (y - 32.5) * 0.4;
                                float zd = (z - 32.5) * 0.4;
                                map[i] = random( 16 );

                float th = random( 256 ) / 256.0f;

                                if (th > sqrt( sqrt( yd * yd + zd * zd ) ) - 0.8f)
                                        map[i] = 0;
                        }
                }
        }
}

static void init( void ) {
    makeTextures( );
    makeMap( );
    makePalette( );
    makeSinTable( );
    makeCosTable( );
    //vgaTest( );
}

// fixed point byte byte multiply
static inline int fxmul( int a, int b ) {
    return (a*b)>>8;
}

// fixed point 8bit packed colour multiply
static inline int rgbmul( int a, int b ) {
    int _r = (((a>>16) & 0xff) * b) >> 13;
    int _g = (((a>> 8) & 0xff) * b) >> 13;
    int _b = (((a    ) & 0xff) * b) >> 14;
    return (_r<<5) | (_g<<2) | _b;
}

int ox = (133120);
 int oy = 132120;
        int oz = 132120;
static void render( void ) {


    //float now = (float)(SDL_GetTicks( ) % 10000) / 10000.f;
        //Access to DOS' clock
        union REGS regs;
        regs.h.ah = 0x00;
        regs.h.al = 0x00;
        int386(0x1a, &regs, &regs);
        int now = (int)((float)(regs.x.edx % 10000) / 10000.f * 100.f) ;

        int xRot = (((math_sin((((now * math_pi) >> 12) * 8192) >> 12 )) * 1638) >> 12) + (math_pi >> 1);
        int yRot = (math_cos(((((now * math_pi) >> 12) * 8192) >> 12)) * 1638) >> 12;
 
        int   yCos = math_cos(yRot);
        int   ySin = math_sin(yRot);
        int   xCos = math_cos(xRot);
        int   xSin = math_sin(xRot);
        
        int ox = ox;
      int oy = 132120;
        int oz = 132120;
        // for each column
        for ( int x = 0; x < w; x++) {
                // get the x axis delta
                int ___xd = (((x - (w >> 1)) << 12) * 20) >> 12;  // 20 is 1/h
                // for each row
                for ( int y = 0; y < h; y++) {
                        // get the y axis delta
                        int    __yd = (((y - (h >> 1)) << 12) * 20) >> 12; //same
                        int    __zd = 4096;
                        int   ___zd =  ((__zd * yCos) >> 12) +  ((__yd * ySin) >> 12);
                        int   _yd =  (((__yd * yCos) >> 12) -  ((__zd * ySin) >> 12));
                        int   _xd =  (((___xd * xCos) >> 12) + ((___zd * xSin) >> 12));
                        int   _zd =  (((___zd * xCos) >> 12) - ((___xd * xSin) >> 12));

                        int col = 0;
                        int br = 255;
                        int ddist = 0;

                        int closest = 131072;
                        
                        // for each principle axis  x,y,z
                        for ( int d = 0; d < 3; d++) {
                                int dimLength = _xd;
                                if (d == 1)
                                        dimLength = _yd;
                                if (d == 2)
                                        dimLength = _zd;
                                if (dimLength == 0) dimLength = 1;

                                int ll = (16777216 / abs(dimLength));
                                //cout << "orig = " << (1.0f / (dimLength < 0.f ? -dimLength : dimLength)) << ", nouv = " << ll << ", dimLenght = " << dimLength << endl;
                                int xd = ((_xd * ll) >> 12);
                                int yd = ((_yd * ll) >> 12);
                                int zd = ((_zd * ll) >> 12);

                                int         initial = ox & 0xFFF;
                                if (d == 1) initial = oy & 0xFFF;
                                if (d == 2) initial = oz & 0xFFF;

                                if (dimLength > 0) initial = 1 - initial;

                                int dist = (ll * initial) >> 12;

                                int xp = ox + ((xd * initial) >> 12);
                                int yp = oy + ((yd * initial) >> 12);
                                int zp = oz + ((zd * initial) >> 12);

                                if (dimLength < 0) {
                                        if (d == 0)     xp-=4096;
                                        if (d == 1)     yp-=4096;
                                        if (d == 2)     zp-=4096;
                                }

                                // while we are concidering a ray that is still closer then the best so far
                                while (dist < closest) {
                                
                                        // quantize to the map grid
                                        int tex = map[ (((zp >> 12) & 63) << 12) | (((yp >> 12) & 63) << 6) | ((xp >> 12) & 63) ];
                                        
                                        // if this voxel has a texture applied
                                        if (tex > 0) {

                                                // find the uv coordinates of the intersection point
                                                int u = (((xp + zp) * 65536) >> 24) & 15;
                                                int v = (((yp * 65536) >> 24 )  & 15) + 16;
                                                
                                                // fix uvs for alternate directions?
                                                if (d == 1) {
                                                        u =  ((xp * 65536) >> 24) & 15;
                                                        v =  ((zp * 65536) >> 24) & 15;
                                                        if (yd < 0)
                                                                v += 32;
                                                }
                                                
                                                // find the colour at the intersection point
                                                int cc = texmap[ u + (v << 4) + (tex << 8) * 3 ];
                                                
                                                // if the colour is not transparent
                                                if (cc > 0) {
                                                        col = cc;

                                                        /* Only the one that uses float really works ... */
                                                        //ddist = 1044480 - ((((dist * 128 >> 12) * 1044480 >> 12))); // 128 -> 1/32
                                                        //ddist = 1044480 - (((dist >> 7) * 1044480) >> 12);
                                                        ddist = 1044480 - ((((dist << 12 ) / 131072 ) * 1044480) >> 12);
                                                        //ddist = (int)(255 - ((((float)dist/4096.f) / 32 * 255))) * 4096;
                                                        if(ddist == 0) ddist = 1;
                                                        br = 255 * (255 - ((d + 2) % 3) * 50) / 255;
                                                        
                                                        // we now have the closest hit point (also terminates this ray)
                                                        closest = dist;
                                                }
                                        }
                                        
                                        // advance the ray
                                        xp += xd;
                                        yp += yd;
                                        zp += zd;
                                        dist += ll;
                                }
                        }

            plot( x, y, rgbmul( col, fxmul( br, ddist >> 12 ) ) );
        }
    }
}static void right( void ) {


    //float now = (float)(SDL_GetTicks( ) % 10000) / 10000.f;
        //Access to DOS' clock
        union REGS regs;
        regs.h.ah = 0x00;
        regs.h.al = 0x00;
        int386(0x1a, &regs, &regs);
        int now = (int)((float)(regs.x.edx % 10000) / 10000.f * 100.f) ;

        int xRot = (((math_sin((((now * math_pi) >> 12) * 8192) >> 12 )) * 1638) >> 12) + (math_pi >> 1);
        int yRot = (math_cos(((((now * math_pi) >> 12) * 8192) >> 12)) * 1638) >> 12;
 
        int   yCos = math_cos(yRot);
        int   ySin = math_sin(yRot);
        int   xCos = math_cos(xRot);
        int   xSin = math_sin(xRot);
        
        int ox = ox; 
      int oy = oy ;
        int oz = oz - ((now * 262144) >> 12);
		
        // for each column
        for ( int x = 0; x < w; x++) {
                // get the x axis delta
                int ___xd = (((x - (w >> 1)) << 12) * 20) >> 12;  // 20 is 1/h
                // for each row
                for ( int y = 0; y < h; y++) {
                        // get the y axis delta
                        int    __yd = (((y - (h >> 1)) << 12) * 20) >> 12; //same
                        int    __zd = 4096;
                        int   ___zd =  ((__zd * yCos) >> 12) +  ((__yd * ySin) >> 12);
                        int   _yd =  (((__yd * yCos) >> 12) -  ((__zd * ySin) >> 12));
                        int   _xd =  (((___xd * xCos) >> 12) + ((___zd * xSin) >> 12));
                        int   _zd =  (((___zd * xCos) >> 12) - ((___xd * xSin) >> 12));

                        int col = 0;
                        int br = 255;
                        int ddist = 0;

                        int closest = 131072;
                        
                        // for each principle axis  x,y,z
                        for ( int d = 0; d < 3; d++) {
                                int dimLength = _xd;
                                if (d == 1)
                                        dimLength = _yd;
                                if (d == 2)
                                        dimLength = _zd;
                                if (dimLength == 0) dimLength = 1;

                                int ll = (16777216 / abs(dimLength));
                                //cout << "orig = " << (1.0f / (dimLength < 0.f ? -dimLength : dimLength)) << ", nouv = " << ll << ", dimLenght = " << dimLength << endl;
                                int xd = ((_xd * ll) >> 12);
                                int yd = ((_yd * ll) >> 12);
                                int zd = ((_zd * ll) >> 12);

                                int         initial = ox & 0xFFF;
                                if (d == 1) initial = oy & 0xFFF;
                                if (d == 2) initial = oz & 0xFFF;

                                if (dimLength > 0) initial = 1 - initial;

                                int dist = (ll * initial) >> 12;

                                int xp = ox + ((xd * initial) >> 12);
                                int yp = oy + ((yd * initial) >> 12);
                                int zp = oz + ((zd * initial) >> 12);

                                if (dimLength < 0) {
                                        if (d == 0)     xp-=4096;
                                        if (d == 1)     yp-=4096;
                                        if (d == 2)     zp-=4096;
                                }

                                // while we are concidering a ray that is still closer then the best so far
                                while (dist < closest) {
                                
                                        // quantize to the map grid
                                        int tex = map[ (((zp >> 12) & 63) << 12) | (((yp >> 12) & 63) << 6) | ((xp >> 12) & 63) ];
                                        
                                        // if this voxel has a texture applied
                                        if (tex > 0) {

                                                // find the uv coordinates of the intersection point
                                                int u = (((xp + zp) * 65536) >> 24) & 15;
                                                int v = (((yp * 65536) >> 24 )  & 15) + 16;
                                                
                                                // fix uvs for alternate directions?
                                                if (d == 1) {
                                                        u =  ((xp * 65536) >> 24) & 15;
                                                        v =  ((zp * 65536) >> 24) & 15;
                                                        if (yd < 0)
                                                                v += 32;
                                                }
                                                
                                                // find the colour at the intersection point
                                                int cc = texmap[ u + (v << 4) + (tex << 8) * 3 ];
                                                
                                                // if the colour is not transparent
                                                if (cc > 0) {
                                                        col = cc;

                                                        /* Only the one that uses float really works ... */
                                                        //ddist = 1044480 - ((((dist * 128 >> 12) * 1044480 >> 12))); // 128 -> 1/32
                                                        //ddist = 1044480 - (((dist >> 7) * 1044480) >> 12);
                                                        ddist = 1044480 - ((((dist << 12 ) / 131072 ) * 1044480) >> 12);
                                                        //ddist = (int)(255 - ((((float)dist/4096.f) / 32 * 255))) * 4096;
                                                        if(ddist == 0) ddist = 1;
                                                        br = 255 * (255 - ((d + 2) % 3) * 50) / 255;
                                                        
                                                        // we now have the closest hit point (also terminates this ray)
                                                        closest = dist;
                                                }
                                        }
                                        
                                        // advance the ray
                                        xp += xd;
                                        yp += yd;
                                        zp += zd;
                                        dist += ll;
                                }
                        }

            plot( x, y, rgbmul( col, fxmul( br, ddist >> 12 ) ) );
        }
    }
}
static void left( void ) {


    //float now = (float)(SDL_GetTicks( ) % 10000) / 10000.f;
        //Access to DOS' clock
        union REGS regs;
        regs.h.ah = 0x00;
        regs.h.al = 0x00;
        int386(0x1a, &regs, &regs);
        int now = (int)((float)(regs.x.edx % 10000) / 10000.f * 100.f) ;

        int xRot = (((math_sin((((now * math_pi) >> 12) * 8192) >> 12 )) * 1638) >> 12) + (math_pi >> 1);
        int yRot = (math_cos(((((now * math_pi) >> 12) * 8192) >> 12)) * 1638) >> 12;
 
        int   yCos = math_cos(yRot);
        int   ySin = math_sin(yRot);
        int   xCos = math_cos(xRot);
        int   xSin = math_sin(xRot);
        
        int ox = ox; 
      int oy = oy ;
        int oz = oz + ((now * 262144) >> 12);
		
        // for each column
        for ( int x = 0; x < w; x++) {
                // get the x axis delta
                int ___xd = (((x - (w >> 1)) << 12) * 20) >> 12;  // 20 is 1/h
                // for each row
                for ( int y = 0; y < h; y++) {
                        // get the y axis delta
                        int    __yd = (((y - (h >> 1)) << 12) * 20) >> 12; //same
                        int    __zd = 4096;
                        int   ___zd =  ((__zd * yCos) >> 12) +  ((__yd * ySin) >> 12);
                        int   _yd =  (((__yd * yCos) >> 12) -  ((__zd * ySin) >> 12));
                        int   _xd =  (((___xd * xCos) >> 12) + ((___zd * xSin) >> 12));
                        int   _zd =  (((___zd * xCos) >> 12) - ((___xd * xSin) >> 12));

                        int col = 0;
                        int br = 255;
                        int ddist = 0;

                        int closest = 131072;
                        
                        // for each principle axis  x,y,z
                        for ( int d = 0; d < 3; d++) {
                                int dimLength = _xd;
                                if (d == 1)
                                        dimLength = _yd;
                                if (d == 2)
                                        dimLength = _zd;
                                if (dimLength == 0) dimLength = 1;

                                int ll = (16777216 / abs(dimLength));
                                //cout << "orig = " << (1.0f / (dimLength < 0.f ? -dimLength : dimLength)) << ", nouv = " << ll << ", dimLenght = " << dimLength << endl;
                                int xd = ((_xd * ll) >> 12);
                                int yd = ((_yd * ll) >> 12);
                                int zd = ((_zd * ll) >> 12);

                                int         initial = ox & 0xFFF;
                                if (d == 1) initial = oy & 0xFFF;
                                if (d == 2) initial = oz & 0xFFF;

                                if (dimLength > 0) initial = 1 - initial;

                                int dist = (ll * initial) >> 12;

                                int xp = ox + ((xd * initial) >> 12);
                                int yp = oy + ((yd * initial) >> 12);
                                int zp = oz + ((zd * initial) >> 12);

                                if (dimLength < 0) {
                                        if (d == 0)     xp-=4096;
                                        if (d == 1)     yp-=4096;
                                        if (d == 2)     zp-=4096;
                                }

                                // while we are concidering a ray that is still closer then the best so far
                                while (dist < closest) {
                                
                                        // quantize to the map grid
                                        int tex = map[ (((zp >> 12) & 63) << 12) | (((yp >> 12) & 63) << 6) | ((xp >> 12) & 63) ];
                                        
                                        // if this voxel has a texture applied
                                        if (tex > 0) {

                                                // find the uv coordinates of the intersection point
                                                int u = (((xp + zp) * 65536) >> 24) & 15;
                                                int v = (((yp * 65536) >> 24 )  & 15) + 16;
                                                
                                                // fix uvs for alternate directions?
                                                if (d == 1) {
                                                        u =  ((xp * 65536) >> 24) & 15;
                                                        v =  ((zp * 65536) >> 24) & 15;
                                                        if (yd < 0)
                                                                v += 32;
                                                }
                                                
                                                // find the colour at the intersection point
                                                int cc = texmap[ u + (v << 4) + (tex << 8) * 3 ];
                                                
                                                // if the colour is not transparent
                                                if (cc > 0) {
                                                        col = cc;

                                                        /* Only the one that uses float really works ... */
                                                        //ddist = 1044480 - ((((dist * 128 >> 12) * 1044480 >> 12))); // 128 -> 1/32
                                                        //ddist = 1044480 - (((dist >> 7) * 1044480) >> 12);
                                                        ddist = 1044480 - ((((dist << 12 ) / 131072 ) * 1044480) >> 12);
                                                        //ddist = (int)(255 - ((((float)dist/4096.f) / 32 * 255))) * 4096;
                                                        if(ddist == 0) ddist = 1;
                                                        br = 255 * (255 - ((d + 2) % 3) * 50) / 255;
                                                        
                                                        // we now have the closest hit point (also terminates this ray)
                                                        closest = dist;
                                                }
                                        }
                                        
                                        // advance the ray
                                        xp += xd;
                                        yp += yd;
                                        zp += zd;
                                        dist += ll;
                                }
                        }

            plot( x, y, rgbmul( col, fxmul( br, ddist >> 12 ) ) );
        }
    }
}
static void up( void ) {


    //float now = (float)(SDL_GetTicks( ) % 10000) / 10000.f;
        //Access to DOS' clock
        union REGS regs;
        regs.h.ah = 0x00;
        regs.h.al = 0x00;
        int386(0x1a, &regs, &regs);
        int now = (10) ;

        int xRot = (((math_sin((((now * math_pi) >> 12) * 8192) >> 12 )) * 1638) >> 12) + (math_pi >> 1);
        int yRot = (math_cos(((((now * math_pi) >> 12) * 8192) >> 12)) * 1638) >> 12;
 
        int   yCos = math_cos(yRot);
        int   ySin = math_sin(yRot);
        int   xCos = math_cos(xRot);
        int   xSin = math_sin(xRot);
        
        int ox = ox;
      int oy = oy - ((now * 262144) >> 12);
        int oz = oz;
        // for each column
        for ( int x = 0; x < w; x++) {
                // get the x axis delta
                int ___xd = (((x - (w >> 1)) << 12) * 20) >> 12;  // 20 is 1/h
                // for each row
                for ( int y = 0; y < h; y++) {
                        // get the y axis delta
                        int    __yd = (((y - (h >> 1)) << 12) * 20) >> 12; //same
                        int    __zd = 4096;
                        int   ___zd =  ((__zd * yCos) >> 12) +  ((__yd * ySin) >> 12);
                        int   _yd =  (((__yd * yCos) >> 12) -  ((__zd * ySin) >> 12));
                        int   _xd =  (((___xd * xCos) >> 12) + ((___zd * xSin) >> 12));
                        int   _zd =  (((___zd * xCos) >> 12) - ((___xd * xSin) >> 12));

                        int col = 0;
                        int br = 255;
                        int ddist = 0;

                        int closest = 131072;
                        
                        // for each principle axis  x,y,z
                        for ( int d = 0; d < 3; d++) {
                                int dimLength = _xd;
                                if (d == 1)
                                        dimLength = _yd;
                                if (d == 2)
                                        dimLength = _zd;
                                if (dimLength == 0) dimLength = 1;

                                int ll = (16777216 / abs(dimLength));
                                //cout << "orig = " << (1.0f / (dimLength < 0.f ? -dimLength : dimLength)) << ", nouv = " << ll << ", dimLenght = " << dimLength << endl;
                                int xd = ((_xd * ll) >> 12);
                                int yd = ((_yd * ll) >> 12);
                                int zd = ((_zd * ll) >> 12);

                                int         initial = ox & 0xFFF;
                                if (d == 1) initial = oy & 0xFFF;
                                if (d == 2) initial = oz & 0xFFF;

                                if (dimLength > 0) initial = 1 - initial;

                                int dist = (ll * initial) >> 12;

                                int xp = ox + ((xd * initial) >> 12);
                                int yp = oy + ((yd * initial) >> 12);
                                int zp = oz + ((zd * initial) >> 12);

                                if (dimLength < 0) {
                                        if (d == 0)     xp-=4096;
                                        if (d == 1)     yp-=4096;
                                        if (d == 2)     zp-=4096;
                                }

                                // while we are concidering a ray that is still closer then the best so far
                                while (dist < closest) {
                                
                                        // quantize to the map grid
                                        int tex = map[ (((zp >> 12) & 63) << 12) | (((yp >> 12) & 63) << 6) | ((xp >> 12) & 63) ];
                                        
                                        // if this voxel has a texture applied
                                        if (tex > 0) {

                                                // find the uv coordinates of the intersection point
                                                int u = (((xp + zp) * 65536) >> 24) & 15;
                                                int v = (((yp * 65536) >> 24 )  & 15) + 16;
                                                
                                                // fix uvs for alternate directions?
                                                if (d == 1) {
                                                        u =  ((xp * 65536) >> 24) & 15;
                                                        v =  ((zp * 65536) >> 24) & 15;
                                                        if (yd < 0)
                                                                v += 32;
                                                }
                                                
                                                // find the colour at the intersection point
                                                int cc = texmap[ u + (v << 4) + (tex << 8) * 3 ];
                                                
                                                // if the colour is not transparent
                                                if (cc > 0) {
                                                        col = cc;

                                                        /* Only the one that uses float really works ... */
                                                        //ddist = 1044480 - ((((dist * 128 >> 12) * 1044480 >> 12))); // 128 -> 1/32
                                                        //ddist = 1044480 - (((dist >> 7) * 1044480) >> 12);
                                                        ddist = 1044480 - ((((dist << 12 ) / 131072 ) * 1044480) >> 12);
                                                        //ddist = (int)(255 - ((((float)dist/4096.f) / 32 * 255))) * 4096;
                                                        if(ddist == 0) ddist = 1;
                                                        br = 255 * (255 - ((d + 2) % 3) * 50) / 255;
                                                        
                                                        // we now have the closest hit point (also terminates this ray)
                                                        closest = dist;
                                                }
                                        }
                                        
                                        // advance the ray
                                        xp += xd;
                                        yp += yd;
                                        zp += zd;
                                        dist += ll;
                                }
                        }

            plot( x, y, rgbmul( col, fxmul( br, ddist >> 12 ) ) );
        }
    }
}
static void down( void ) {


    //float now = (float)(SDL_GetTicks( ) % 10000) / 10000.f;
        //Access to DOS' clock
        union REGS regs;
        regs.h.ah = 0x00;
        regs.h.al = 0x00;
        int386(0x1a, &regs, &regs);
        int now = (int)((float)(regs.x.edx % 10000) / 10000.f * 100.f) ;

        int xRot = (((math_sin((((now * math_pi) >> 12) * 8192) >> 12 )) * 1638) >> 12) + (math_pi >> 1);
        int yRot = (math_cos(((((now * math_pi) >> 12) * 8192) >> 12)) * 1638) >> 12;
 
        int   yCos = math_cos(yRot);
        int   ySin = math_sin(yRot);
        int   xCos = math_cos(xRot);
        int   xSin = math_sin(xRot);
        
        int ox = ox;
      int oy = oy + ((now * 262144) >> 12);
        int oz = oz;
        // for each column
        for ( int x = 0; x < w; x++) {
                // get the x axis delta
                int ___xd = (((x - (w >> 1)) << 12) * 20) >> 12;  // 20 is 1/h
                // for each row
                for ( int y = 0; y < h; y++) {
                        // get the y axis delta
                        int    __yd = (((y - (h >> 1)) << 12) * 20) >> 12; //same
                        int    __zd = 4096;
                        int   ___zd =  ((__zd * yCos) >> 12) +  ((__yd * ySin) >> 12);
                        int   _yd =  (((__yd * yCos) >> 12) -  ((__zd * ySin) >> 12));
                        int   _xd =  (((___xd * xCos) >> 12) + ((___zd * xSin) >> 12));
                        int   _zd =  (((___zd * xCos) >> 12) - ((___xd * xSin) >> 12));

                        int col = 0;
                        int br = 255;
                        int ddist = 0;

                        int closest = 131072;
                        
                        // for each principle axis  x,y,z
                        for ( int d = 0; d < 3; d++) {
                                int dimLength = _xd;
                                if (d == 1)
                                        dimLength = _yd;
                                if (d == 2)
                                        dimLength = _zd;
                                if (dimLength == 0) dimLength = 1;

                                int ll = (16777216 / abs(dimLength));
                                //cout << "orig = " << (1.0f / (dimLength < 0.f ? -dimLength : dimLength)) << ", nouv = " << ll << ", dimLenght = " << dimLength << endl;
                                int xd = ((_xd * ll) >> 12);
                                int yd = ((_yd * ll) >> 12);
                                int zd = ((_zd * ll) >> 12);

                                int         initial = ox & 0xFFF;
                                if (d == 1) initial = oy & 0xFFF;
                                if (d == 2) initial = oz & 0xFFF;

                                if (dimLength > 0) initial = 1 - initial;

                                int dist = (ll * initial) >> 12;

                                int xp = ox + ((xd * initial) >> 12);
                                int yp = oy + ((yd * initial) >> 12);
                                int zp = oz + ((zd * initial) >> 12);

                                if (dimLength < 0) {
                                        if (d == 0)     xp-=4096;
                                        if (d == 1)     yp-=4096;
                                        if (d == 2)     zp-=4096;
                                }

                                // while we are concidering a ray that is still closer then the best so far
                                while (dist < closest) {
                                
                                        // quantize to the map grid
                                        int tex = map[ (((zp >> 12) & 63) << 12) | (((yp >> 12) & 63) << 6) | ((xp >> 12) & 63) ];
                                        
                                        // if this voxel has a texture applied
                                        if (tex > 0) {

                                                // find the uv coordinates of the intersection point
                                                int u = (((xp + zp) * 65536) >> 24) & 15;
                                                int v = (((yp * 65536) >> 24 )  & 15) + 16;
                                                
                                                // fix uvs for alternate directions?
                                                if (d == 1) {
                                                        u =  ((xp * 65536) >> 24) & 15;
                                                        v =  ((zp * 65536) >> 24) & 15;
                                                        if (yd < 0)
                                                                v += 32;
                                                }
                                                
                                                // find the colour at the intersection point
                                                int cc = texmap[ u + (v << 4) + (tex << 8) * 3 ];
                                                
                                                // if the colour is not transparent
                                                if (cc > 0) {
                                                        col = cc;

                                                        /* Only the one that uses float really works ... */
                                                        //ddist = 1044480 - ((((dist * 128 >> 12) * 1044480 >> 12))); // 128 -> 1/32
                                                        //ddist = 1044480 - (((dist >> 7) * 1044480) >> 12);
                                                        ddist = 1044480 - ((((dist << 12 ) / 131072 ) * 1044480) >> 12);
                                                        //ddist = (int)(255 - ((((float)dist/4096.f) / 32 * 255))) * 4096;
                                                        if(ddist == 0) ddist = 1;
                                                        br = 255 * (255 - ((d + 2) % 3) * 50) / 255;
                                                        
                                                        // we now have the closest hit point (also terminates this ray)
                                                        closest = dist;
                                                }
                                        }
                                        
                                        // advance the ray
                                        xp += xd;
                                        yp += yd;
                                        zp += zd;
                                        dist += ll;
                                }
                        }

            plot( x, y, rgbmul( col, fxmul( br, ddist >> 12 ) ) );
        }
    }
}

static void foward( void ) {


    //float now = (float)(SDL_GetTicks( ) % 10000) / 10000.f;
        //Access to DOS' clock
        union REGS regs;
        regs.h.ah = 0x00;
        regs.h.al = 0x00;
        int386(0x1a, &regs, &regs);
        int now = (int)((float)(regs.x.edx % 10000) / 10000.f * 100.f) ;

        int xRot = (((math_sin((((now * math_pi) >> 12) * 8192) >> 12 )) * 1638) >> 12) + (math_pi >> 1);
        int yRot = (math_cos(((((now * math_pi) >> 12) * 8192) >> 12)) * 1638) >> 12;
 
        int   yCos = math_cos(yRot);
        int   ySin = math_sin(yRot);
        int   xCos = math_cos(xRot);
        int   xSin = math_sin(xRot);
        
        int ox = ox + ((now * 262144) >> 12);
      int oy = oy;
        int oz = oz;
        // for each column
        for ( int x = 0; x < w; x++) {
                // get the x axis delta
                int ___xd = (((x - (w >> 1)) << 12) * 20) >> 12;  // 20 is 1/h
                // for each row
                for ( int y = 0; y < h; y++) {
                        // get the y axis delta
                        int    __yd = (((y - (h >> 1)) << 12) * 20) >> 12; //same
                        int    __zd = 4096;
                        int   ___zd =  ((__zd * yCos) >> 12) +  ((__yd * ySin) >> 12);
                        int   _yd =  (((__yd * yCos) >> 12) -  ((__zd * ySin) >> 12));
                        int   _xd =  (((___xd * xCos) >> 12) + ((___zd * xSin) >> 12));
                        int   _zd =  (((___zd * xCos) >> 12) - ((___xd * xSin) >> 12));

                        int col = 0;
                        int br = 255;
                        int ddist = 0;

                        int closest = 131072;
                        
                        // for each principle axis  x,y,z
                        for ( int d = 0; d < 3; d++) {
                                int dimLength = _xd;
                                if (d == 1)
                                        dimLength = _yd;
                                if (d == 2)
                                        dimLength = _zd;
                                if (dimLength == 0) dimLength = 1;

                                int ll = (16777216 / abs(dimLength));
                                //cout << "orig = " << (1.0f / (dimLength < 0.f ? -dimLength : dimLength)) << ", nouv = " << ll << ", dimLenght = " << dimLength << endl;
                                int xd = ((_xd * ll) >> 12);
                                int yd = ((_yd * ll) >> 12);
                                int zd = ((_zd * ll) >> 12);

                                int         initial = ox & 0xFFF;
                                if (d == 1) initial = oy & 0xFFF;
                                if (d == 2) initial = oz & 0xFFF;

                                if (dimLength > 0) initial = 1 - initial;

                                int dist = (ll * initial) >> 12;

                                int xp = ox + ((xd * initial) >> 12);
                                int yp = oy + ((yd * initial) >> 12);
                                int zp = oz + ((zd * initial) >> 12);

                                if (dimLength < 0) {
                                        if (d == 0)     xp-=4096;
                                        if (d == 1)     yp-=4096;
                                        if (d == 2)     zp-=4096;
                                }

                                // while we are concidering a ray that is still closer then the best so far
                                while (dist < closest) {
                                
                                        // quantize to the map grid
                                        int tex = map[ (((zp >> 12) & 63) << 12) | (((yp >> 12) & 63) << 6) | ((xp >> 12) & 63) ];
                                        
                                        // if this voxel has a texture applied
                                        if (tex > 0) {

                                                // find the uv coordinates of the intersection point
                                                int u = (((xp + zp) * 65536) >> 24) & 15;
                                                int v = (((yp * 65536) >> 24 )  & 15) + 16;
                                                
                                                // fix uvs for alternate directions?
                                                if (d == 1) {
                                                        u =  ((xp * 65536) >> 24) & 15;
                                                        v =  ((zp * 65536) >> 24) & 15;
                                                        if (yd < 0)
                                                                v += 32;
                                                }
                                                
                                                // find the colour at the intersection point
                                                int cc = texmap[ u + (v << 4) + (tex << 8) * 3 ];
                                                
                                                // if the colour is not transparent
                                                if (cc > 0) {
                                                        col = cc;

                                                        /* Only the one that uses float really works ... */
                                                        //ddist = 1044480 - ((((dist * 128 >> 12) * 1044480 >> 12))); // 128 -> 1/32
                                                        //ddist = 1044480 - (((dist >> 7) * 1044480) >> 12);
                                                        ddist = 1044480 - ((((dist << 12 ) / 131072 ) * 1044480) >> 12);
                                                        //ddist = (int)(255 - ((((float)dist/4096.f) / 32 * 255))) * 4096;
                                                        if(ddist == 0) ddist = 1;
                                                        br = 255 * (255 - ((d + 2) % 3) * 50) / 255;
                                                        
                                                        // we now have the closest hit point (also terminates this ray)
                                                        closest = dist;
                                                }
                                        }
                                        
                                        // advance the ray
                                        xp += xd;
                                        yp += yd;
                                        zp += zd;
                                        dist += ll;
                                }
                        }

            plot( x, y, rgbmul( col, fxmul( br, ddist >> 12 ) ) );
        }
    }
}
static void back( void ) {


    //float now = (float)(SDL_GetTicks( ) % 10000) / 10000.f;
        //Access to DOS' clock
        union REGS regs;
        regs.h.ah = 0x00;
        regs.h.al = 0x00;
        int386(0x1a, &regs, &regs);
        int now = (int)((float)(regs.x.edx % 10000) / 10000.f * 100.f) ;

        int xRot = (((math_sin((((now * math_pi) >> 12) * 8192) >> 12 )) * 1638) >> 12) + (math_pi >> 1);
        int yRot = (math_cos(((((now * math_pi) >> 12) * 8192) >> 12)) * 1638) >> 12;
 
        int   yCos = math_cos(yRot);
        int   ySin = math_sin(yRot);
        int   xCos = math_cos(xRot);
        int   xSin = math_sin(xRot);
        
        int ox = ox - ((now * 262144) >> 12);
        int oy = oy;
        int oz = oz;

        // for each column
        for ( int x = 0; x < w; x++) {
                // get the x axis delta
                int ___xd = (((x - (w >> 1)) << 12) * 20) >> 12;  // 20 is 1/h
                // for each row
                for ( int y = 0; y < h; y++) {
                        // get the y axis delta
                        int    __yd = (((y - (h >> 1)) << 12) * 20) >> 12; //same
                        int    __zd = 4096;
                        int   ___zd =  ((__zd * yCos) >> 12) +  ((__yd * ySin) >> 12);
                        int   _yd =  (((__yd * yCos) >> 12) -  ((__zd * ySin) >> 12));
                        int   _xd =  (((___xd * xCos) >> 12) + ((___zd * xSin) >> 12));
                        int   _zd =  (((___zd * xCos) >> 12) - ((___xd * xSin) >> 12));

                        int col = 0;
                        int br = 255;
                        int ddist = 0;

                        int closest = 131072;
                        
                        // for each principle axis  x,y,z
                        for ( int d = 0; d < 3; d++) {
                                int dimLength = _xd;
                                if (d == 1)
                                        dimLength = _yd;
                                if (d == 2)
                                        dimLength = _zd;
                                if (dimLength == 0) dimLength = 1;

                                int ll = (16777216 / abs(dimLength));
                                //cout << "orig = " << (1.0f / (dimLength < 0.f ? -dimLength : dimLength)) << ", nouv = " << ll << ", dimLenght = " << dimLength << endl;
                                int xd = ((_xd * ll) >> 12);
                                int yd = ((_yd * ll) >> 12);
                                int zd = ((_zd * ll) >> 12);

                                int         initial = ox & 0xFFF;
                                if (d == 1) initial = oy & 0xFFF;
                                if (d == 2) initial = oz & 0xFFF;

                                if (dimLength > 0) initial = 1 - initial;

                                int dist = (ll * initial) >> 12;

                                int xp = ox + ((xd * initial) >> 12);
                                int yp = oy + ((yd * initial) >> 12);
                                int zp = oz + ((zd * initial) >> 12);

                                if (dimLength < 0) {
                                        if (d == 0)     xp-=4096;
                                        if (d == 1)     yp-=4096;
                                        if (d == 2)     zp-=4096;
                                }

                                // while we are concidering a ray that is still closer then the best so far
                                while (dist < closest) {
                                
                                        // quantize to the map grid
                                        int tex = map[ (((zp >> 12) & 63) << 12) | (((yp >> 12) & 63) << 6) | ((xp >> 12) & 63) ];
                                        
                                        // if this voxel has a texture applied
                                        if (tex > 0) {

                                                // find the uv coordinates of the intersection point
                                                int u = (((xp + zp) * 65536) >> 24) & 15;
                                                int v = (((yp * 65536) >> 24 )  & 15) + 16;
                                                
                                                // fix uvs for alternate directions?
                                                if (d == 1) {
                                                        u =  ((xp * 65536) >> 24) & 15;
                                                        v =  ((zp * 65536) >> 24) & 15;
                                                        if (yd < 0)
                                                                v += 32;
                                                }
                                                
                                                // find the colour at the intersection point
                                                int cc = texmap[ u + (v << 4) + (tex << 8) * 3 ];
                                                
                                                // if the colour is not transparent
                                                if (cc > 0) {
                                                        col = cc;

                                                        /* Only the one that uses float really works ... */
                                                        //ddist = 1044480 - ((((dist * 128 >> 12) * 1044480 >> 12))); // 128 -> 1/32
                                                        //ddist = 1044480 - (((dist >> 7) * 1044480) >> 12);
                                                        ddist = 1044480 - ((((dist << 12 ) / 131072 ) * 1044480) >> 12);
                                                        //ddist = (int)(255 - ((((float)dist/4096.f) / 32 * 255))) * 4096;
                                                        if(ddist == 0) ddist = 1;
                                                        br = 255 * (255 - ((d + 2) % 3) * 50) / 255;
                                                        
                                                        // we now have the closest hit point (also terminates this ray)
                                                        closest = dist;
                                                }
                                        }
                                        
                                        // advance the ray
                                        xp += xd;
                                        yp += yd;
                                        zp += zd;
                                        dist += ll;
                                }
                        }

            plot( x, y, rgbmul( col, fxmul( br, ddist >> 12 ) ) );
        }
    }
}

int main( void ) {
	
	   char ch;
    //SDL_Init( SDL_INIT_VIDEO );
    //screen = SDL_SetVideoMode( w, h, 32, 0 );

    union REGS regs;

    //VGA 320x200 graphic mode 
    regs.h.ah = 0x00;
    regs.h.al = 0x13;
    int386(0x10, &regs, &regs);

    init( );
    bool running = true;

	
	
    float fps = 0.f;

    int cpt = 0;
	
	render( );
	right( );
	left( );
	up( );
	up( );up( );
	up( );
	up( );
	up( );
	up( );up( );
	up( );
	up( );
	up( );up( );up( );up( );
	up( );
    regs.h.ah = 0x00;
    regs.h.al = 0x00;
    int386(0x1a, &regs, &regs);
    int time = regs.x.edx;
    float dTime = 0;
    



	
	
	while ( running ) {
        /*SDL_Event event;
        while ( SDL_PollEvent( &event ) ) {
            running &= (event.type != SDL_QUIT);
        }*/
		



			


        //SDL_Flip( screen );
     
        cpt ++;
		
		  ch = getch();
        if (ch == 68) // 27 is the ASCII code for ESC key
        {
            right( );
			
        }
		 if (ch == 65) // 27 is the ASCII code for ESC key
        {
            left( );
			
        }
		  ch = getch();
        if (ch == 87) // 27 is the ASCII code for ESC key
        {
            foward( );
			
        }
		 if (ch == 83) // 27 is the ASCII code for ESC key
        {
            back( );
			
        }
		 if (ch == 32) // 27 is the ASCII code for ESC key
        {
            up( );
			
        }
		 if (ch == 90) // 27 is the ASCII code for ESC key
        {
            down( );
			
        }
		 if (ch == 27) // 27 is the ASCII code for ESC key
        {
             running = false;
			
        }
		
    }

  
    regs.h.ah = 0x00;
    regs.h.al = 0x00;
    int386(0x1a, &regs, &regs);
    dTime = (float)(regs.x.edx - time) /18.f;
    fps = (float)cpt / dTime;

    //Text mode
    regs.h.ah = 0x00;
    regs.h.al = 0x03;
    int386(0x10, &regs, &regs);

    cout << "FPS : " << fps << endl;
    
    return 0;
}

