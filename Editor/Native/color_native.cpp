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
//
// Implementation from wgt2allg.cpp specific to Engine runtime
//
//=============================================================================

#include "util/wgt2allg.h"
#include "gfx/bitmap.h"

// win32 uses BGR not RGB
#define SWAP_RB_HICOL_FOR_32to24_32
#define SWAP_RB_HICOL_FOR_32to15_16

void __my_setcolor(int *ctset, int newcol, int wantColDep)
  {
    if (wantColDep == 8)
      ctset[0] = newcol;
    else if (newcol & 0x40000000) // already calculated it
      ctset[0] = newcol;
    else if ((newcol >= 32) && (wantColDep > 16)) {
      // true-color
#ifdef SWAP_RB_HICOL_FOR_32to24_32
      ctset[0] = makeacol32(getb24(newcol), getg24(newcol), getr24(newcol), 255);
#else
      ctset[0] = makeacol32(getr24(newcol), getg24(newcol), getb24(newcol), 255);
#endif
    }
    else if (newcol >= 32) {

#ifdef SWAP_RB_HICOL_FOR_32to15_16
      ctset[0] = makecol16(getb16(newcol), getg16(newcol), getr16(newcol));
#else
      // If it's 15-bit, convert the color
      if (wantColDep == 15)
        ctset[0] = (newcol & 0x001f) | ((newcol >> 1) & 0x7fe0);
      else
        ctset[0] = newcol;
#endif
    } 
    else
    {
      ctset[0] = makecol_depth(wantColDep, col_lookups[newcol] >> 16,
                               (col_lookups[newcol] >> 8) & 0x000ff, col_lookups[newcol] & 0x000ff);

      // in case it's used on an alpha-channel sprite, make sure it's visible
      if (wantColDep > 16)
        ctset[0] |= 0xff000000;
    }

    // if it's 32-bit color, signify that the colour has been calculated
    //if (wantColDep >= 24)
//      ctset[0] |= 0x40000000;
  }
