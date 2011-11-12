/* -*- mode: C; c-basic-offset: 4; intent-tabs-mode: nil -*-
 *
 * message banners that show up to display score or game info
 * Copyright <c> 2011 Sifteo, Inc. All rights reserved.
 */

#include "TimeKeeper.h"
#include "string.h"
#include "assets.gen.h"

TimeKeeper::TimeKeeper()
{
	m_fTimer = TIME_INITIAL;
}


void TimeKeeper::Reset()
{
	m_fTimer = TIME_INITIAL;
}

void TimeKeeper::Draw( Cube &cube )
{
	//for now, just draw in the corner
	_SYS_vbuf_pokeb(&cube.vbuf.sys, offsetof(_SYSVideoRAM, mode), _SYS_VM_BG0_SPR_BG1);
	// Allocate tiles for the timer

	char aBuf[16];
	//how many digits
	int iTimer = (int)m_fTimer;

	sprintf( aBuf, "%d", iTimer );
	int iDigits = strlen( aBuf );

	switch( iDigits )
	{
		case 1:
			_SYS_vbuf_fill(&cube.vbuf.sys, offsetof(_SYSVideoRAM, bg1_bitmap) / 2, 0x8000, 2 );
			break;
		case 2:
			_SYS_vbuf_fill(&cube.vbuf.sys, offsetof(_SYSVideoRAM, bg1_bitmap) / 2, 0xC000, 2 );
			break;
		case 3:
			_SYS_vbuf_fill(&cube.vbuf.sys, offsetof(_SYSVideoRAM, bg1_bitmap) / 2, 0xE000, 2 );
			break;
	}
	

	//draw timer
	for( int i = 0; i < iDigits; i++ )
	{
		//double tall
		for( int j = 0; j < 2; j++ )
		{
			/*_SYS_vbuf_writei(&cube.vbuf.sys, offsetof(_SYSVideoRAM, bg1_tiles) / 2 + iDigits + ( iDigits * ( j + 1 ) ),
							 Font.tiles + ( ( aBuf[i] - ' ' ) * 2 ) + j,
							 0, 1);*/
			_SYS_vbuf_writei(&cube.vbuf.sys, offsetof(_SYSVideoRAM, bg1_tiles) / 2 + i + ( iDigits * ( j ) ),
							 Font.tiles + ( ( aBuf[i] - ' ' ) * 2 ) + j,
							 0, 1);
		}
	}

	_SYS_vbuf_pokeb(&cube.vbuf.sys, offsetof(_SYSVideoRAM, bg1_y), 0);
}


void TimeKeeper::Update(float t)
{
	float dt = t - m_fLastTime;
	m_fLastTime = t;

	m_fTimer -= dt;
}


void TimeKeeper::Init( float t )
{
	Reset();
	m_fLastTime = t;
}


void TimeKeeper::AddTime( int numGems )
{
	m_fTimer += numGems * TIME_RETURN_PER_GEM;
}