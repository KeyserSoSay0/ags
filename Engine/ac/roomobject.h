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
//
// Runtime room object definition.
//
//=============================================================================
#ifndef __AGS_EE_AC__ROOMOBJECT_H
#define __AGS_EE_AC__ROOMOBJECT_H

#include "core/types.h"
#include "ac/common_defines.h"
#include "gfx/gfx_def.h"
#include "util/string.h"

namespace AGS { namespace Common { class Stream; }}
using namespace AGS; // FIXME later

struct RoomObject {
    static const uint16_t NoView = UINT16_MAX;

    int   x,y;
    int   transparent;    // current transparency setting
    short tint_r, tint_g;   // specific object tint
    short tint_b, tint_level;
    short tint_light;
    short zoom;           // zoom level, either manual or from the current area
    int   spr_width, spr_height; // last used sprite's size
    short last_width, last_height; // width/height last time drawn (includes scaling)
    uint16_t num;            // sprite slot number
    short baseline;       // <=0 to use Y co-ordinate; >0 for specific baseline
    uint16_t view,loop,frame; // only used to track animation - 'num' holds the current sprite
    short wait,moving;
    char  cycling;        // is it currently animating?
    char  overall_speed;
    char  on;
    char  flags;
    // Down to here is a part of the plugin API
    short blocking_width, blocking_height;
    int   anim_volume = -1; // current animation volume
    Common::String name;
    Common::BlendMode blend_mode;
    float rotation;

    RoomObject();

    int get_width();
    int get_height();
    int get_baseline();

    inline bool has_explicit_light() const { return (flags & OBJF_HASLIGHT) != 0; }
    inline bool has_explicit_tint()  const { return (flags & OBJF_HASTINT) != 0; }

    inline const Common::GraphicSpace &GetGraphicSpace() const { return _gs; }
    void UpdateGraphicSpace();

    void UpdateCyclingView(int ref_id);

    void ReadFromSavegame(Common::Stream *in, int cmp_ver);
    void WriteToSavegame(Common::Stream *out) const;

private:
    Common::GraphicSpace _gs;
};

#endif // __AGS_EE_AC__ROOMOBJECT_H
