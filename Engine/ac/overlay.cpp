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
#include "ac/overlay.h"
#include "ac/common.h"
#include "ac/view.h"
#include "ac/character.h"
#include "ac/characterextras.h"
#include "ac/display.h"
#include "ac/draw.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/global_overlay.h"
#include "ac/global_translation.h"
#include "ac/runtime_defines.h"
#include "ac/screenoverlay.h"
#include "ac/string.h"
#include "debug/debug_log.h"
#include "gfx/graphicsdriver.h"
#include "gfx/bitmap.h"
#include "script/runtimescriptvalue.h"

using namespace AGS::Common;
using namespace AGS::Engine;

extern GameSetupStruct game;
extern int displayed_room;
extern int face_talking;
extern std::vector<ViewStruct> views;
extern CharacterExtras *charextra;
extern IGraphicsDriver *gfxDriver;



std::vector<ScreenOverlay> screenover;

void Overlay_Remove(ScriptOverlay *sco) {
    sco->Remove();
}

void Overlay_SetText(ScriptOverlay *scover, int wii, int fontid, int text_color, const char *text) {
    int ovri=find_overlay_of_type(scover->overlayId);
    if (ovri<0)
        quit("!Overlay.SetText: invalid overlay ID specified");
    int xx = screenover[ovri].x;
    int yy = screenover[ovri].y;

    RemoveOverlay(scover->overlayId);
    const int disp_type = scover->overlayId;

    if (CreateTextOverlay(xx, yy, wii, fontid, text_color, get_translation(text), disp_type) != scover->overlayId)
        quit("SetTextOverlay internal error: inconsistent type ids");
}

int Overlay_GetX(ScriptOverlay *scover) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");

    int tdxp, tdyp;
    get_overlay_position(screenover[ovri], &tdxp, &tdyp);

    return tdxp;
}

void Overlay_SetX(ScriptOverlay *scover, int newx) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");

    screenover[ovri].x = newx;
}

int Overlay_GetY(ScriptOverlay *scover) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");

    int tdxp, tdyp;
    get_overlay_position(screenover[ovri], &tdxp, &tdyp);

    return tdyp;
}

void Overlay_SetY(ScriptOverlay *scover, int newy) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");

    screenover[ovri].y = newy;
}

int Overlay_GetWidth(ScriptOverlay *scover) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");
    return screenover[ovri].scaleWidth;
}

int Overlay_GetHeight(ScriptOverlay *scover) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");
    return screenover[ovri].scaleHeight;
}

int Overlay_GetGraphicWidth(ScriptOverlay *scover) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");
    return screenover[ovri].pic->GetWidth();
}

int Overlay_GetGraphicHeight(ScriptOverlay *scover) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");
    return screenover[ovri].pic->GetHeight();
}

void Overlay_SetScaledSize(ScreenOverlay &over, int width, int height) {
    if (width < 1 || height < 1)
    {
        debug_script_warn("Overlay.SetSize: invalid dimensions: %d x %d", width, height);
        return;
    }
    if ((width == over.scaleWidth) && (height == over.scaleHeight))
        return; // no change
    over.scaleWidth = width;
    over.scaleHeight = height;
    over.MarkChanged();
}

void Overlay_SetWidth(ScriptOverlay *scover, int width) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");
    Overlay_SetScaledSize(screenover[ovri], width, screenover[ovri].scaleHeight);
}

void Overlay_SetHeight(ScriptOverlay *scover, int height) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");
    Overlay_SetScaledSize(screenover[ovri], screenover[ovri].scaleWidth, height);
}

int Overlay_GetValid(ScriptOverlay *scover) {
    if (scover->overlayId == -1)
        return 0;

    if (!IsOverlayValid(scover->overlayId)) {
        scover->overlayId = -1;
        return 0;
    }

    return 1;
}

ScriptOverlay* Overlay_CreateGraphical(int x, int y, int slot, int transparent) {
    ScriptOverlay *sco = new ScriptOverlay();
    sco->overlayId = CreateGraphicOverlay(x, y, slot, transparent);
    ccRegisterManagedObject(sco, sco);
    return sco;
}

ScriptOverlay* Overlay_CreateTextual(int x, int y, int width, int font, int colour, const char* text) {
    ScriptOverlay *sco = new ScriptOverlay();

    sco->overlayId = CreateTextOverlayCore(x, y, width, font, colour, text, DISPLAYTEXT_NORMALOVERLAY, 0);
    ccRegisterManagedObject(sco, sco);
    return sco;
}

int Overlay_GetBlendMode(ScriptOverlay *scover) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");

    return screenover[ovri].blendMode;
}

void Overlay_SetBlendMode(ScriptOverlay *scover, int blendMode) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");
    if ((blendMode < 0) || (blendMode >= kNumBlendModes))
        quitprintf("!SetBlendMode: invalid blend mode %d, supported modes are %d - %d", blendMode, 0, kNumBlendModes - 1);

    screenover[ovri].blendMode = (BlendMode)blendMode;
}

int Overlay_GetTransparency(ScriptOverlay *scover) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");

    return GfxDef::LegacyTrans255ToTrans100(screenover[ovri].transparency);
}

void Overlay_SetTransparency(ScriptOverlay *scover, int trans) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");
    if ((trans < 0) | (trans > 100))
        quit("!SetTransparency: transparency value must be between 0 and 100");

    screenover[ovri].transparency = GfxDef::Trans100ToLegacyTrans255(trans);
}

float Overlay_GetRotation(ScriptOverlay *scover) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");
    return screenover[ovri].rotation;
}

void Overlay_SetRotation(ScriptOverlay *scover, float degrees) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");
    screenover[ovri].rotation = Math::ClampAngle360(degrees);
}

int Overlay_GetZOrder(ScriptOverlay *scover) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");

    return screenover[ovri].zorder;
}

void Overlay_SetZOrder(ScriptOverlay *scover, int zorder) {
    int ovri = find_overlay_of_type(scover->overlayId);
    if (ovri < 0)
        quit("!invalid overlay ID specified");

    screenover[ovri].zorder = zorder;
}

//=============================================================================

// Creates and registers a managed script object for existing overlay object
ScriptOverlay* create_scriptobj_for_overlay(ScreenOverlay &over)
{
    ScriptOverlay *scover = new ScriptOverlay();
    scover->overlayId = over.type;
    int handl = ccRegisterManagedObject(scover, scover);
    over.associatedOverlayHandle = handl;
    return scover;
}

// Creates managed script object for overlay and adds internal engine's reference to it,
// so that it does not get disposed even if there are no user references in script.
static ScriptOverlay* create_scriptobj_addref(ScreenOverlay &over)
{
    ScriptOverlay* scover = create_scriptobj_for_overlay(over);
    ccAddObjectReference(over.associatedOverlayHandle);
    return scover;
}

// Invalidates existing script object to let user know that previous overlay is gone,
// and releases engine's internal reference (script object may exist while there are user refs)
static void invalidate_and_subref(ScreenOverlay &over, ScriptOverlay *&scover)
{
    scover->overlayId = -1;
    scover = nullptr;
    ccReleaseObjectReference(over.associatedOverlayHandle);
}

// Frees overlay resources and disposes script object if there are no more refs
static void dispose_overlay(ScreenOverlay &over)
{
    delete over.pic;
    over.pic = nullptr;
    if (over.ddb != nullptr)
        gfxDriver->DestroyDDB(over.ddb);
    over.ddb = nullptr;
    if (over.associatedOverlayHandle) // dispose script object if there are no more refs
        ccAttemptDisposeObject(over.associatedOverlayHandle);
}

void remove_screen_overlay_index(size_t over_idx)
{
    ScreenOverlay &over = screenover[over_idx];
    // TODO: move these custom settings outside of this function
    if (over.type == play.complete_overlay_on)
    {
        play.complete_overlay_on = 0;
    }
    else if (over.type == play.text_overlay_on)
    {
        if (play.speech_text_scover)
            invalidate_and_subref(over, play.speech_text_scover);
        play.text_overlay_on = 0;
    }
    else if (over.type == OVER_PICTURE)
    {
        if (play.speech_face_scover)
            invalidate_and_subref(over, play.speech_face_scover);
        face_talking = -1;
    }
    dispose_overlay(over);
    screenover.erase(screenover.begin() + over_idx);
    // if an overlay before the sierra-style speech one is removed, update the index
    // TODO: this is bad, need more generic system to store overlay references
    if ((size_t)face_talking > over_idx)
        face_talking--;
}

void remove_screen_overlay(int type)
{
    for (size_t i = 0; i < screenover.size();)
    {
        if (type < 0 || screenover[i].type == type)
            remove_screen_overlay_index(i);
        else
            i++;
    }
}

int find_overlay_of_type(int type)
{
    for (size_t i = 0; i < screenover.size(); ++i)
    {
        if (screenover[i].type == type) return i;
    }
    return -1;
}

size_t add_screen_overlay(int x, int y, int type, Bitmap *piccy, bool alphaChannel)
{
    return add_screen_overlay(x, y, type, piccy, 0, 0, alphaChannel);
}

size_t add_screen_overlay(int x, int y, int type, Common::Bitmap *piccy, int pic_offx, int pic_offy, bool alphaChannel, BlendMode blendMode)
{
    if (type == OVER_CUSTOM) {
        // find an unused custom ID; TODO: find a better approach!
        for (int id = OVER_CUSTOM + 1; (size_t)id <= screenover.size() + OVER_CUSTOM + 1; ++id) {
            if (find_overlay_of_type(id) == -1) { type=id; break; }
        }
    }
    ScreenOverlay over;
    over.pic=piccy;
    over.x=x;
    over.y=y;
    over.offsetX = pic_offx;
    over.offsetY = pic_offy;
    over.scaleWidth = piccy->GetWidth();
    over.scaleHeight = piccy->GetHeight();
    // by default draw speech and portraits over GUI, and the rest under GUI
    over.zorder = (type == OVER_TEXTMSG || type == OVER_PICTURE || type == OVER_TEXTSPEECH) ?
        INT_MAX : INT_MIN;
    over.type=type;
    over.timeout=0;
    over.bgSpeechForChar = -1;
    over.associatedOverlayHandle = 0;
    over.hasAlphaChannel = alphaChannel;
    over.positionRelativeToScreen = true;
    over.blendMode = blendMode;
    // TODO: move these custom settings outside of this function
    if (type == OVER_COMPLETE) play.complete_overlay_on = type;
    else if (type == OVER_TEXTMSG || type == OVER_TEXTSPEECH)
    {
        play.text_overlay_on = type;
        // only make script object for blocking speech now, because messagebox blocks all script
        // and therefore cannot be accessed, so no practical reason for that atm
        if (type == OVER_TEXTSPEECH)
            play.speech_text_scover = create_scriptobj_addref(over);
    }
    else if (type == OVER_PICTURE)
    {
        play.speech_face_scover = create_scriptobj_addref(over);
    }
    over.MarkChanged();
    screenover.push_back(std::move(over));
    return screenover.size() - 1;
}



void get_overlay_position(const ScreenOverlay &over, int *x, int *y) {
    int tdxp, tdyp;
    const Rect &ui_view = play.GetUIViewport();

    if (over.x == OVR_AUTOPLACE) {
        // auto place on character
        int charid = over.y;

        auto view = FindNearestViewport(charid);
        const int charpic = views[game.chars[charid].view].loops[game.chars[charid].loop].frames[0].pic;
        const int height = (charextra[charid].height < 1) ? game.SpriteInfos[charpic].Height : charextra[charid].height;
        Point screenpt = view->RoomToScreen(
            game.chars[charid].x,
            game.chars[charid].get_effective_y() - height).first;
        tdxp = screenpt.X - over.pic->GetWidth() / 2;
        if (tdxp < 0) tdxp = 0;
        tdyp = screenpt.Y - 5;
        tdyp -= over.pic->GetHeight();
        if (tdyp < 5) tdyp = 5;

        if ((tdxp + over.pic->GetWidth()) >= ui_view.GetWidth())
            tdxp = (ui_view.GetWidth() - over.pic->GetWidth()) - 1;
        if (game.chars[charid].room != displayed_room) {
            tdxp = ui_view.GetWidth()/2 - over.pic->GetWidth()/2;
            tdyp = ui_view.GetHeight()/2 - over.pic->GetHeight()/2;
        }
    }
    else {
        // Note: the internal offset is only needed when x,y coordinates are specified
        // and only in the case where the overlay is using a GUI. See issue #1098
        tdxp = over.x + over.offsetX;
        tdyp = over.y + over.offsetY;

        if (!over.positionRelativeToScreen)
        {
            Point tdxy = play.RoomToScreen(tdxp, tdyp);
            tdxp = tdxy.X;
            tdyp = tdxy.Y;
        }
    }
    *x = tdxp;
    *y = tdyp;
}

Point update_overlay_graphicspace(ScreenOverlay &over)
{
    int atx, aty;
    get_overlay_position(over, &atx, &aty);
    over._gs = GraphicSpace(atx, aty, over.pic->GetWidth(), over.pic->GetHeight(),
        over.pic->GetWidth(), over.pic->GetHeight(), over.rotation);
    return Point(atx, aty);
}

void recreate_overlay_image(ScreenOverlay &over, bool is_3d_render,
    Bitmap *&scalebmp, Bitmap *&rotbmp)
{
    Bitmap *use_bmp = over.pic;
    // For software renderer - apply all supported Overlay transforms
    if (!is_3d_render && (over.rotation != 0.0))
    {
        Size final_sz = RotateSize(over.pic->GetSize(), over.rotation);
        rotbmp = recycle_bitmap(rotbmp, over.pic->GetColorDepth(),
            final_sz.Width, final_sz.Height, false);
        const int dst_w = final_sz.Width;
        const int dst_h = final_sz.Height;
        // (+ width%2 fixes one pixel offset problem)
        rotbmp->ClearTransparent();
        rotbmp->RotateBlt(over.pic, dst_w / 2 + dst_w % 2, dst_h / 2,
            over.pic->GetWidth() / 2, over.pic->GetHeight() / 2, over.rotation); // clockwise
        use_bmp = rotbmp;
    }
    if (!is_3d_render && (over.pic->GetSize() != Size(over.scaleWidth, over.scaleHeight)))
    {
        Size final_sz = RotateSize(Size(over.scaleWidth, over.scaleHeight), over.rotation);
        scalebmp = recycle_bitmap(scalebmp, over.pic->GetColorDepth(), over.scaleWidth, over.scaleHeight);
        scalebmp->StretchBlt(use_bmp, RectWH(scalebmp->GetSize()));
        use_bmp = scalebmp;
    }
    over.ddb = recycle_ddb_bitmap(over.ddb, use_bmp, over.hasAlphaChannel);
}

void recreate_overlay_ddbs()
{
    for (auto &over : screenover)
    {
        if (over.ddb)
            gfxDriver->DestroyDDB(over.ddb);
        over.ddb = nullptr; // is generated during first draw pass
        over.MarkChanged();
    }
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

#include "debug/out.h"
#include "script/script_api.h"
#include "script/script_runtime.h"

// ScriptOverlay* (int x, int y, int slot, int transparent)
RuntimeScriptValue Sc_Overlay_CreateGraphical(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_OBJAUTO_PINT4(ScriptOverlay, Overlay_CreateGraphical);
}

// ScriptOverlay* (int x, int y, int width, int font, int colour, const char* text, ...)
RuntimeScriptValue Sc_Overlay_CreateTextual(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_SCRIPT_SPRINTF(Overlay_CreateTextual, 6);
    ScriptOverlay *overlay = Overlay_CreateTextual(params[0].IValue, params[1].IValue, params[2].IValue,
                                                   params[3].IValue, params[4].IValue, scsf_buffer);
    return RuntimeScriptValue().SetDynamicObject(overlay, overlay);
}

// void (ScriptOverlay *scover, int wii, int fontid, int clr, char*texx, ...)
RuntimeScriptValue Sc_Overlay_SetText(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_SCRIPT_SPRINTF(Overlay_SetText, 4);
    Overlay_SetText((ScriptOverlay*)self, params[0].IValue, params[1].IValue, params[2].IValue, scsf_buffer);
    return RuntimeScriptValue((int32_t)0);
}

// void (ScriptOverlay *sco)
RuntimeScriptValue Sc_Overlay_Remove(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID(ScriptOverlay, Overlay_Remove);
}

// int (ScriptOverlay *scover)
RuntimeScriptValue Sc_Overlay_GetValid(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptOverlay, Overlay_GetValid);
}

// int (ScriptOverlay *scover)
RuntimeScriptValue Sc_Overlay_GetX(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptOverlay, Overlay_GetX);
}

// void (ScriptOverlay *scover, int newx)
RuntimeScriptValue Sc_Overlay_SetX(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptOverlay, Overlay_SetX);
}

// int (ScriptOverlay *scover)
RuntimeScriptValue Sc_Overlay_GetY(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptOverlay, Overlay_GetY);
}

// void (ScriptOverlay *scover, int newy)
RuntimeScriptValue Sc_Overlay_SetY(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptOverlay, Overlay_SetY);
}

RuntimeScriptValue Sc_Overlay_GetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptOverlay, Overlay_GetWidth);
}

RuntimeScriptValue Sc_Overlay_SetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptOverlay, Overlay_SetWidth);
}

RuntimeScriptValue Sc_Overlay_GetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptOverlay, Overlay_GetHeight);
}

RuntimeScriptValue Sc_Overlay_SetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptOverlay, Overlay_SetHeight);
}

RuntimeScriptValue Sc_Overlay_GetGraphicWidth(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptOverlay, Overlay_GetGraphicWidth);
}

RuntimeScriptValue Sc_Overlay_GetGraphicHeight(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptOverlay, Overlay_GetGraphicHeight);
}

RuntimeScriptValue Sc_Overlay_GetBlendMode(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptOverlay, Overlay_GetBlendMode);
}

RuntimeScriptValue Sc_Overlay_SetBlendMode(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptOverlay, Overlay_SetBlendMode);
}

RuntimeScriptValue Sc_Overlay_GetRotation(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_FLOAT(ScriptOverlay, Overlay_GetRotation);
}

RuntimeScriptValue Sc_Overlay_SetRotation(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PFLOAT(ScriptOverlay, Overlay_SetRotation);
}

RuntimeScriptValue Sc_Overlay_GetTransparency(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptOverlay, Overlay_GetTransparency);
}

RuntimeScriptValue Sc_Overlay_SetTransparency(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptOverlay, Overlay_SetTransparency);
}


RuntimeScriptValue Sc_Overlay_GetZOrder(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptOverlay, Overlay_GetZOrder);
}

RuntimeScriptValue Sc_Overlay_SetZOrder(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptOverlay, Overlay_SetZOrder);
}

//=============================================================================
//
// Exclusive API for Plugins
//
//=============================================================================

// ScriptOverlay* (int x, int y, int width, int font, int colour, const char* text, ...)
ScriptOverlay* ScPl_Overlay_CreateTextual(int x, int y, int width, int font, int colour, const char *text, ...)
{
    API_PLUGIN_SCRIPT_SPRINTF(text);
    return Overlay_CreateTextual(x, y, width, font, colour, scsf_buffer);
}

// void (ScriptOverlay *scover, int wii, int fontid, int clr, char*texx, ...)
void ScPl_Overlay_SetText(ScriptOverlay *scover, int wii, int fontid, int clr, char *texx, ...)
{
    API_PLUGIN_SCRIPT_SPRINTF(texx);
    Overlay_SetText(scover, wii, fontid, clr, scsf_buffer);
}


void RegisterOverlayAPI()
{
    ccAddExternalStaticFunction("Overlay::CreateGraphical^4",   Sc_Overlay_CreateGraphical);
    ccAddExternalStaticFunction("Overlay::CreateTextual^106",   Sc_Overlay_CreateTextual);
    ccAddExternalObjectFunction("Overlay::SetText^104",         Sc_Overlay_SetText);
    ccAddExternalObjectFunction("Overlay::Remove^0",            Sc_Overlay_Remove);
    ccAddExternalObjectFunction("Overlay::get_Valid",           Sc_Overlay_GetValid);
    ccAddExternalObjectFunction("Overlay::get_X",               Sc_Overlay_GetX);
    ccAddExternalObjectFunction("Overlay::set_X",               Sc_Overlay_SetX);
    ccAddExternalObjectFunction("Overlay::get_Y",               Sc_Overlay_GetY);
    ccAddExternalObjectFunction("Overlay::set_Y",               Sc_Overlay_SetY);
    ccAddExternalObjectFunction("Overlay::get_Width",           Sc_Overlay_GetWidth);
    ccAddExternalObjectFunction("Overlay::set_Width",           Sc_Overlay_SetWidth);
    ccAddExternalObjectFunction("Overlay::get_Height",          Sc_Overlay_GetHeight);
    ccAddExternalObjectFunction("Overlay::set_Height",          Sc_Overlay_SetHeight);
    ccAddExternalObjectFunction("Overlay::get_GraphicWidth",    Sc_Overlay_GetGraphicWidth);
    ccAddExternalObjectFunction("Overlay::get_GraphicHeight",   Sc_Overlay_GetGraphicHeight);
    ccAddExternalObjectFunction("Overlay::get_BlendMode",       Sc_Overlay_GetBlendMode);
    ccAddExternalObjectFunction("Overlay::set_BlendMode",       Sc_Overlay_SetBlendMode);
    ccAddExternalObjectFunction("Overlay::get_Rotation",        Sc_Overlay_GetRotation);
    ccAddExternalObjectFunction("Overlay::set_Rotation",        Sc_Overlay_SetRotation);
    ccAddExternalObjectFunction("Overlay::get_Transparency",    Sc_Overlay_GetTransparency);
    ccAddExternalObjectFunction("Overlay::set_Transparency",    Sc_Overlay_SetTransparency);
    ccAddExternalObjectFunction("Overlay::get_ZOrder",          Sc_Overlay_GetZOrder);
    ccAddExternalObjectFunction("Overlay::set_ZOrder",          Sc_Overlay_SetZOrder);

    /* ----------------------- Registering unsafe exports for plugins -----------------------*/

    ccAddExternalFunctionForPlugin("Overlay::CreateGraphical^4",   (void*)Overlay_CreateGraphical);
    ccAddExternalFunctionForPlugin("Overlay::CreateTextual^106",   (void*)ScPl_Overlay_CreateTextual);
    ccAddExternalFunctionForPlugin("Overlay::SetText^104",         (void*)ScPl_Overlay_SetText);
    ccAddExternalFunctionForPlugin("Overlay::Remove^0",            (void*)Overlay_Remove);
    ccAddExternalFunctionForPlugin("Overlay::get_Valid",           (void*)Overlay_GetValid);
    ccAddExternalFunctionForPlugin("Overlay::get_X",               (void*)Overlay_GetX);
    ccAddExternalFunctionForPlugin("Overlay::set_X",               (void*)Overlay_SetX);
    ccAddExternalFunctionForPlugin("Overlay::get_Y",               (void*)Overlay_GetY);
    ccAddExternalFunctionForPlugin("Overlay::set_Y",               (void*)Overlay_SetY);
}
