//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-2024 various contributors
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// https://opensource.org/license/artistic-2-0/
//
//=============================================================================
#include "ac/common.h"
#include "ac/draw.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/global_palette.h"
#include "util/wgt2allg.h"

extern GameSetupStruct game;
extern RGB palette[256];

// CLNUP remove everything and all references to palettes ?

void CyclePalette(int strt,int eend) {
    // hi-color game must invalidate screen since the palette changes
    // the effect of the drawing operations
    if (game.color_depth > 1)
        invalidate_screen();

    if ((strt < 0) || (strt > 255) || (eend < 0) || (eend > 255))
        quit("!CyclePalette: start and end must be 0-255");

    if (eend > strt) {
        // forwards
        wcolrotate(strt, eend, 0, palette);
        set_palette_range(palette, strt, eend, 0);
    }
    else {
        // backwards
        wcolrotate(eend, strt, 1, palette);
        set_palette_range(palette, eend, strt, 0);
    }

}
void SetPalRGB(int inndx,int rr,int gg,int bb) {
    if (game.color_depth > 1)
        invalidate_screen();

    wsetrgb(inndx,rr,gg,bb,palette);
    set_palette_range(palette, inndx, inndx, 0);
}

void UpdatePalette() {
    if (game.color_depth > 1)
        invalidate_screen();

    if (!play.fast_forward)  
        setpal();
}
