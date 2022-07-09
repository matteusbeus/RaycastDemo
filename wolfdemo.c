/*
 * SegaCD WolfDemo
 * By: Chilly Willy
 * Based on LCDWolf3D
 * By: Jeremy A Burns
 */

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "hw_md.h"
#include "files.h"
#include "wolfdemo.h"

extern volatile uint16_t switch_flag;
extern volatile uint16_t int3_flag;

uint8_t pos = 0;
uint8_t direction = 0;
uint8_t headAnimationFrame = 5;
uint8_t timer = 0;
uint8_t statusBar = 0;

fixed_t sintab[403], costab[403], oostab[403], ooctab[403], oosh[256];

uint16_t *textures[5];
/*
    0,
    tex_wood,
    tex_bricks,
    tex_bird,
    tex_bricks2
*/

const uint8_t level[] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,2,2,3,2,2,3,2,2,3,2,2,4,4,4,4,4,4,4,4,4,4,4,1,
  1,2,0,0,0,0,0,0,0,0,0,2,4,0,0,0,0,0,0,0,0,0,4,1,
  1,3,0,0,0,0,0,0,0,0,0,3,4,0,0,0,0,0,0,0,0,0,4,1,
  1,2,0,0,0,3,2,2,2,0,0,2,4,0,0,4,0,4,0,4,0,0,4,1,
  1,2,0,0,0,2,2,0,0,0,0,2,4,0,0,0,0,0,0,0,0,0,4,1,
  1,3,0,0,0,3,3,0,0,0,0,3,4,0,0,4,0,0,0,4,0,0,4,1,
  1,2,2,0,2,2,2,0,0,0,0,2,4,0,0,0,0,4,4,4,0,0,4,1,
  1,0,2,3,2,0,2,2,0,2,2,2,4,4,4,4,4,0,0,0,0,4,4,1,
  1,0,0,0,0,0,0,4,0,4,4,4,4,4,4,4,0,0,0,0,0,4,4,1,
  1,0,0,0,0,0,0,4,0,0,0,0,0,0,4,0,0,0,0,0,0,0,4,1,
  1,0,0,0,0,0,0,4,0,0,0,0,0,0,4,0,0,0,3,0,0,0,4,1,
  1,0,0,0,0,0,0,4,0,4,4,4,0,0,0,0,0,0,0,0,0,0,4,1,
  1,0,0,0,0,0,0,4,0,4,4,4,4,4,4,4,0,0,0,0,0,4,4,1,
  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,0,0,0,4,4,4,1,
  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,0,0,0,1,
  1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,
  1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,1,0,0,1,0,0,0,1,
  1,4,0,0,0,0,0,0,4,0,0,0,0,0,0,0,1,0,0,1,0,0,0,1,
  1,4,0,4,0,0,0,0,4,0,0,0,4,1,0,0,0,1,1,0,0,0,0,1,
  1,4,0,4,4,4,4,4,4,0,0,0,1,4,0,0,0,0,0,0,0,0,0,1,
  1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
  1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
  1,1,1,1,1,1,1,1,1,1,1,4,1,1,4,1,1,4,1,1,4,1,1,1
};


uint16_t *screenBuffer = (uint16_t *)0x0C0000; // word ram on CD side (in 1M mode)

fixed_t px = I2F(70);
fixed_t py = I2F(170);
fixed_t pa = -102944; // I2F(-3.14159265f / 2.0f)

void castRay( fixed_t x, fixed_t y, fixed_t angle, uint8_t screenX, SLICE *result )
{
        
        // origin coordinates
        fixed_t ox = x;        
        fixed_t oy = y;

        // calculate offset angle
        fixed_t offsetAngle = screenX * 536 - 34304;
        angle += offsetAngle;

        // set yAxisWall to false
        boolean_t yAxisWall = false;

        // The coordinate of the block we're sitting in
        uint8_t blockX = x >> (BLOCK_SHIFT + 16);    
        uint8_t blockY = y >> (BLOCK_SHIFT + 16);

        fixed_t cosine = FIX_COS( angle );
        fixed_t sine   = FIX_SIN( angle );

        fixed_t oneOverCosine = FIX_OOC( angle );
        fixed_t oneOverSine   = FIX_OOS( angle );

        fixed_t xNext_x, xNext_y, xNext_l;
        fixed_t yNext_x, yNext_y, yNext_l;

        do
        {

            xNext_x = blockX << (BLOCK_SHIFT + 16);
            if( cosine > 0 ) xNext_x += I2F(BLOCK_SIZE);
            xNext_x -= x;
            xNext_l = FIX_MUL( xNext_x, oneOverCosine );

            yNext_y = blockY << (BLOCK_SHIFT + 16);
            if( sine > 0 ) yNext_y += I2F(BLOCK_SIZE);
            yNext_y -= y;
            yNext_l = FIX_MUL( yNext_y, oneOverSine );

            if( xNext_l < yNext_l )
            {
                yAxisWall = true;
                xNext_y = FIX_MUL( xNext_l, sine );
                blockX += cosine > 0 ? 1 : -1 ;
                x += xNext_x;
                y += xNext_y;
            }
            else
            {
                yAxisWall = false;
                yNext_x = FIX_MUL( yNext_l, cosine );
                blockY += sine > 0 ? 1 : -1 ;
                x += yNext_x;
                y += yNext_y;
            }

            result->textureId = level[ blockX + MAP_WIDTH * blockY ];
        }
        while( result->textureId == 0 );

        result->targetBlockX = blockX;
        result->targetBlockY = blockY;

        if( yAxisWall ) {
            result->textureOffset = ( y >> (BLOCK_SHIFT + 16 - 6) );
        } else {
            result->textureOffset = ( x >> (BLOCK_SHIFT + 16 - 6) );
        }

        fixed_t dx = x - ox;
        fixed_t dy = y - oy;

        uint32_t dd = ( approxDist( F2I(dx), F2I(dy) ) * ( FIX_COS( offsetAngle ) >> 8 ) ) >> 8;

        result->sliceHeight = dd < 8 ? 255 : VISPLANEDIST_TIMES_WALLHEIGHT / dd;

}

void drawSlice( uint8_t screenX, SLICE *slice )
{
        if( slice->sliceHeight == 0 )
            return; // early out if no height

        uint8_t offsetY   = ( slice->sliceHeight >  64 ) ? ( 0 ) : ( 32 - ( slice->sliceHeight >> 1 ) );
        uint8_t overflowY = ( slice->sliceHeight <= 64 ) ? ( 0 ) : ( ( slice->sliceHeight - 64 ) >> 1 );

        register uint8_t heightY   = ( slice->sliceHeight >  64 ) ? ( 64 ) : ( slice->sliceHeight );
        register uint16_t textureInc = FIX_MUL( ( 1 << 12 ), oosh[slice->sliceHeight] );
        register uint16_t textureY =  FIX_MUL( ( overflowY << 12 ), oosh[slice->sliceHeight] );
        register uint16_t *texturePtr = &textures[ slice->textureId ][ slice->textureOffset & 63 ];
        register uint16_t *screenPtr = &screenBuffer[ offsetY * 322 + screenX + 8 ];

        while( heightY )
        {
            register uint16_t c = texturePtr[ textureY & ~63 ];
            *screenPtr = c;
            screenPtr += 161;
            *screenPtr = c;
            screenPtr += 161;
            textureY += textureInc;
            heightY--;
        }
}

boolean_t shouldInterpolate( SLICE *sliceA, SLICE *sliceB )
{
        if( sliceA->textureId != sliceB->textureId ) return false;

        uint8_t xDiff = UNSIGNED_DIFF( sliceA->targetBlockX, sliceB->targetBlockX );
        uint8_t yDiff = UNSIGNED_DIFF( sliceA->targetBlockY, sliceB->targetBlockY );

        if( xDiff == 0 && yDiff <= 1 ) return true;
        if( xDiff <= 1 && yDiff == 0 ) return true;

        return false;
}

void drawHead()
{

        register uint8_t x, y;

        if (timer == 10) {
            if (headAnimationFrame < 7)
            {
                headAnimationFrame++;
            } 
            else 
            {
                headAnimationFrame = 5;
            }
            timer = 0;
        } 

        if (timer < 2) 
        {

            register uint16_t *sprite_head = (uint16_t *)(filePtr[headAnimationFrame] + 4);
            register uint16_t *screenPtr = &screenBuffer[ 66 * 322 + 32 + 8 ];
            for( y = 17 ; y < 46; ++y, screenPtr+=322 )
            for( x = 18 ; x < 42; ++x )
            {
            
                register uint16_t c = sprite_head[ x + ( y << 6 ) ]; 

                if( c )
                {
                    screenPtr[ x ] = c;
                    screenPtr[ x + 161 ] = c;
                }

            }

        }

        timer++;

}

void drawPistol()
{

        register uint8_t x, y, temp;
        register uint16_t *sprite_pistol = (uint16_t *)(filePtr[4] + 4);
        register uint16_t *screenPtr = &screenBuffer[ 30 * 322 + 32 + 8 ];
        for( y = 29 ; y < 63 ; ++y, screenPtr+=322 )
        for( x = 18 ; x < 46 ; ++x )
        {
            
            temp = y - pos;

            // selecting pixel within image with single dimension array (y << 6 = y * 2^6 = y * 64)
            register uint16_t c = sprite_pistol[ x + ( temp << 6 ) ]; 

            if( c )
            {
                screenPtr[ x ] = c;
                screenPtr[ x + 161 ] = c;
            }

        }

}

void drawStatusBar()
{

        register uint8_t x, y;
        register uint16_t *status_bar_bitmap = (uint16_t *)(filePtr[8] + 4);
        register uint16_t *screenPtr = &screenBuffer[ 64 * 322 + 8 ];
        for( y = 0 ; y < 32 ; ++y, screenPtr+=322 )
        for( x = 0 ; x < 128 ; ++x )
        {

            register uint16_t c = status_bar_bitmap[ y * 128 + x ]; 

            if( c )
            {
                screenPtr[ x ] = c;
                screenPtr[ x + 161 ] = c;
            }

        }

}

void render()
{
        uint8_t x, y;

        for( y=0; y<128; y++ )
            memset( &screenBuffer[8 + y * 161], y < 64 ? 0x44 : 0x77, 256 );

        SLICE sliceA, sliceB;
        SLICE sliceX[ 3 ];

        // player x, y, angle, x?, slice?
        castRay( px, py, pa, 0, &sliceA );

        for( x=4; x<=128; x+=4 )
        {

            castRay( px, py, pa, x, &sliceB );

            if( shouldInterpolate( &sliceA , &sliceB ) )
            {
                sliceX[1].textureId     = sliceA.textureId;
                sliceX[1].textureOffset = ( sliceA.textureOffset + sliceB.textureOffset ) >> 1;
                sliceX[1].sliceHeight   = ( sliceA.sliceHeight   + sliceB.sliceHeight   ) >> 1;
            }
            else castRay( px, py, pa, x - 2, &sliceX[1] );

            sliceX[0].textureId     =   sliceA.textureId;
            sliceX[0].textureOffset = ( sliceA.textureOffset + sliceX[1].textureOffset ) >> 1;
            sliceX[0].sliceHeight   = ( sliceA.sliceHeight   + sliceX[1].sliceHeight   ) >> 1;

            sliceX[2].textureId     =   sliceX[1].textureId;
            sliceX[2].textureOffset = ( sliceX[1].textureOffset + sliceB.textureOffset ) >> 1;
            sliceX[2].sliceHeight   = ( sliceX[1].sliceHeight   + sliceB.sliceHeight   ) >> 1;

            drawSlice( x - 4, &sliceA );
            drawSlice( x - 3, &sliceX[0] );
            drawSlice( x - 2, &sliceX[1] );
            drawSlice( x - 1, &sliceX[2] );

            sliceA = sliceB;
        }

        drawPistol();

        if (statusBar != 2)
        {
            drawStatusBar();
            statusBar ++;
        }

        drawHead();

        // switch banks (in INT2 routine)
        switch_flag = 1;
}

void setup()
{
        uint16_t i;

        textures[0] = 0;
        textures[1] = (uint16_t *)(filePtr[0] + 4);
        textures[2] = (uint16_t *)(filePtr[1] + 4);
        textures[3] = (uint16_t *)(filePtr[2] + 4);
        textures[4] = (uint16_t *)(filePtr[3] + 4);

        for( i=1; i<256; i++ )
            oosh[i] = 65536 / i;

        for( i=0; i<403; i++ )
        {
            float cosine = cosf( (float)i / 64.0f );
            float sine   = sinf( (float)i / 64.0f );

            if( sine   <  0.002f && sine   >= 0.0f ) sine   =  0.002f;
            if( sine   > -0.002f && sine   <  0.0f ) sine   = -0.002f;
            if( cosine <  0.002f && cosine >= 0.0f ) cosine =  0.002f;
            if( cosine > -0.002f && cosine <  0.0f ) cosine = -0.002f;

            float oneOverCosine = 1.0f / cosine;
            float oneOverSine   = 1.0f / sine;

            sintab[i] = (fixed_t)(sine * 65536.0f);
            costab[i] = (fixed_t)(cosine * 65536.0f);
            oostab[i] = (fixed_t)(oneOverSine * 65536.0f);
            ooctab[i] = (fixed_t)(oneOverCosine * 65536.0f);
        }
}

void loop()
{
        while (switch_flag) ; // wait until word ram is switched
        render();

        uint16_t buttons = GET_PAD(0);

        fixed_t dx = 0;
        fixed_t dy = 0;

        if( buttons & SEGA_CTRL_UP )
        {
            dx =  FIX_MUL(I2F(5), FIX_COS( pa ) );
            dy =  FIX_MUL(I2F(5), FIX_SIN( pa ) );
            if (direction == 0 && pos != 5) {
                pos++;
                if (pos == 5) {
                    direction = 1;
                }
            }  if (direction == 1 && pos > 0) {
                pos--;
                if (pos == 0) {
                    direction = 0;
                }
            }
        }
        if( buttons & SEGA_CTRL_DOWN )
        {
            dx =  FIX_MUL(I2F(-5), FIX_COS( pa ) );
            dy =  FIX_MUL(I2F(-5), FIX_SIN( pa ) );
            if (direction == 0 && pos != 5) {
                pos++;
                if (pos == 5) {
                    direction = 1;
                }
            }  if (direction == 1 && pos > 0) {
                pos--;
                if (pos == 0) {
                    direction = 0;
                }
            }
        }
        if( buttons & SEGA_CTRL_A )
        {
            dx =  FIX_MUL(I2F(5), FIX_COS( pa - 102944 ) );
            dy =  FIX_MUL(I2F(5), FIX_SIN( pa - 102944 ) );
        }
        if( buttons & SEGA_CTRL_C )
        {
            dx =  FIX_MUL(I2F(-5), FIX_COS( pa - 102944 ) );
            dy =  FIX_MUL(I2F(-5), FIX_SIN( pa - 102944 ) );
        }
        if( buttons & SEGA_CTRL_LEFT )
        {
            pa -= 8235;
            if( pa < 0 )
                pa += 411775;
        }
        if( buttons & SEGA_CTRL_RIGHT )
        {
            pa += 8235;
            if( pa > 411774 )
                pa -= 411775;
        }

        // Collision detection
        uint8_t blockX = px >> (BLOCK_SHIFT + 16);    // The coordinate of the block we're sitting in
        uint8_t blockY = py >> (BLOCK_SHIFT + 16);
        if( dx < 0 )
        {
            if( level[ ( blockX - 1 ) + MAP_WIDTH * blockY ] )
            {
                uint8_t sx = ( F2I(px) & ( BLOCK_SIZE - 1 ) ) + 1;
                if( ( sx + F2I(dx) ) < HIT_WIDTH )
                    dx = I2F(HIT_WIDTH - sx);
            }
        }
        else if( dx > 0 )
        {
            if( level[ ( blockX + 1 ) + MAP_WIDTH * blockY ] )
            {
                uint8_t sx = BLOCK_SIZE - ( F2I(px) & ( BLOCK_SIZE - 1 ) );
                if( ( sx - F2I(dx) ) < HIT_WIDTH )
                    dx = I2F(sx - HIT_WIDTH);
            }
        }
        if( dy < 0 )
        {
            if( level[ blockX + MAP_WIDTH * ( blockY - 1 ) ] )
            {
                uint8_t sy = ( F2I(py) & ( BLOCK_SIZE - 1 ) ) + 1;
                if( ( sy + F2I(dy) ) < HIT_WIDTH )
                    dy = I2F(HIT_WIDTH - sy);
            }
        }
        else if( dy > 0 )
        {
            if( level[ blockX + MAP_WIDTH * ( blockY + 1 ) ] )
            {
                uint8_t sy = BLOCK_SIZE - ( F2I(py) & ( BLOCK_SIZE - 1 ) );
                if( ( sy - F2I(dy) ) < HIT_WIDTH )
                    dy = I2F(sy - HIT_WIDTH);
            }
        }

        // move player
        px += dx;
        py += dy;

        // game logic goes here

}
