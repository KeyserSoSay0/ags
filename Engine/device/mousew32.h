//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
#include "ac/sys_events.h"
#include "util/geometry.h"

#define MAXCURSORS 20


namespace AGS { namespace Common { class Bitmap; } }
using namespace AGS; // FIXME later

void msetgraphpos(int,int);
void mgetgraphpos();
// Sets the area of the game frame (zero-based coordinates) where the mouse cursor is allowed to move;
// this function was meant to be used to achieve gameplay effect
void msetcursorlimit(int x1, int y1, int x2, int y2);
void msetgraphpos(int xa, int ya);
void msethotspot(int xx, int yy);
int minstalled();

namespace Mouse
{
    // Get if mouse is locked to the game window
    bool IsLockedToWindow();
    // Try locking mouse to the game window
    bool TryLockToWindow();
    // Unlock mouse from the game window
    void UnlockFromWindow();

    // Enable or disable mouse movement control
    void SetMovementControl(bool on);
    // Tell if the mouse movement control is enabled
    bool IsControlEnabled();
    // Set the touch2mouse motion mode: absolute/relative
    void SetTouch2MouseMode(TouchMouseEmulation mode, bool relative, float speed);
    // Set base speed factor, which would serve as a mouse speed unit
    void SetSpeedUnit(float f);
    // Get base speed factor
    float GetSpeedUnit();
    // Set speed factors
    void SetSpeed(float speed);
    // Get speed factor
    float GetSpeed();
}

namespace Mouse
{
    // Updates limits of the area inside which the standard OS cursor is not shown;
    // uses game's main viewport (in native coordinates) to calculate real area on screen
    void UpdateGraphicArea();
    // Limits the area where the game cursor can move on virtual screen;
    // parameter must be in native game coordinates
    void SetMoveLimit(const Rect &r);
    // Set actual OS cursor position on screen; parameter must be in native game coordinates
    void SetPosition(const Point p);
}


extern int mousex, mousey;
extern int hotx, hoty;
extern char currentcursor;

extern Common::Bitmap *mousecurs[MAXCURSORS];
