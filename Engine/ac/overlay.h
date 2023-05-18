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
//
//
//=============================================================================
#ifndef __AGS_EE_AC__OVERLAY_H
#define __AGS_EE_AC__OVERLAY_H
#include <vector>
#include "ac/screenoverlay.h"
#include "ac/dynobj/scriptoverlay.h"
#include "util/geometry.h"

namespace AGS { namespace Common { class Bitmap; } }
using namespace AGS; // FIXME later

void Overlay_Remove(ScriptOverlay *sco);
void Overlay_SetText(ScriptOverlay *scover, int wii, int fontid, int clr, const char*text);
int  Overlay_GetX(ScriptOverlay *scover);
void Overlay_SetX(ScriptOverlay *scover, int newx);
int  Overlay_GetY(ScriptOverlay *scover);
void Overlay_SetY(ScriptOverlay *scover, int newy);
int  Overlay_GetValid(ScriptOverlay *scover);
ScriptOverlay* Overlay_CreateGraphical(int x, int y, int slot, bool clone);
ScriptOverlay* Overlay_CreateTextual(int x, int y, int width, int font, int colour, const char* text);
ScreenOverlay *Overlay_CreateGraphicCore(bool room_layer, int x, int y, int slot, bool clone = false);
ScreenOverlay *Overlay_CreateTextCore(bool room_layer, int x, int y, int width, int font, int text_color,
    const char *text, int disp_type, int allow_shrink);

int  find_overlay_of_type(int type);
void remove_screen_overlay(int type);
// Calculates overlay position in its respective layer (screen or room)
Point get_overlay_position(const ScreenOverlay &over);
size_t add_screen_overlay(bool roomlayer, int x, int y, int type, int sprnum);
size_t add_screen_overlay(bool roomlayer, int x, int y, int type, Common::Bitmap *piccy);
size_t add_screen_overlay(bool roomlayer, int x, int y, int type, Common::Bitmap *piccy, int pic_offx, int pic_offy);
// Creates and registers a managed script object for existing overlay object;
// optionally adds an internal engine reference to prevent object's disposal
ScriptOverlay* create_scriptoverlay(ScreenOverlay &over, bool internal_ref = false);
void remove_screen_overlay_index(size_t over_idx);
void recreate_overlay_ddbs();
// Create or resize GUI surface, accomodating for any GUI transformations
Common::Bitmap *recreate_overlay_image(ScreenOverlay &over, bool is_3d_render,
    Common::Bitmap *&scalebmp, Common::Bitmap *&rotbmp);
// Recalculates overlay's transform matrix and AABB, returns overlay object's position
Point update_overlay_graphicspace(ScreenOverlay &over);

extern std::vector<ScreenOverlay> screenover;
#endif // __AGS_EE_AC__OVERLAY_H
