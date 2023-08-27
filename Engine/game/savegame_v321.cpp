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
// These functions are restoring old savegames used by engines 3.2.1 - 3.5.0.
// The main point of keeping these today are to be able to compare game
// behavior when running with original/older binaries and latest engine.
// Perhaps the optimal solution would be to have a savegame converter instead.
//
//=============================================================================
#include <vector>
#include "core/types.h"
#include "ac/button.h"
#include "ac/characterextras.h"
#include "ac/common.h"
#include "ac/dialog.h"
#include "ac/draw.h"
#include "ac/dynamicsprite.h"
#include "ac/game.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/movelist.h"
#include "ac/overlay.h"
#include "ac/spritecache.h"
#include "ac/roomstatus.h"
#include "ac/view.h"
#include "ac/dynobj/cc_serializer.h"
#include "ac/dynobj/dynobj_manager.h"
#include "game/savegame.h"
#include "game/savegame_components.h"
#include "game/savegame_internal.h"
#include "gui/animatingguibutton.h"
#include "gui/guimain.h"
#include "gui/guibutton.h"
#include "gui/guiinv.h"
#include "gui/guilabel.h"
#include "gui/guilistbox.h"
#include "gui/guislider.h"
#include "gui/guitextbox.h"
#include "media/audio/audio.h"
#include "plugin/plugin_engine.h"
#include "script/script.h"
#include "script/cc_common.h"
#include "util/alignedstream.h"
#include "util/string_utils.h"

using namespace AGS::Common;
using namespace AGS::Engine;

extern GameSetupStruct game;
extern GameState play;
extern std::vector<ViewStruct> views;
extern RGB palette[256];
extern std::vector<AnimatingGUIButton> animbuts;
extern int ifacepopped;
extern int mouse_on_iface;
extern RoomStatus troom;


static const uint32_t MAGICNUMBER = 0xbeefcafe;

// List of game objects, used to compare with the save contents
struct ObjectCounts
{
    int CharacterCount = 0;
    int DialogCount = 0;
    int InvItemCount = 0;
    int ViewCount = 0;
    int GUICount = 0;
    int GUILabelCount = 0;
    int GUIButtonCount = 0;
    int GUIInvWindowCount = 0;
    int GUIListBoxCount = 0;
    int GUISliderCount = 0;
    int GUITextBoxCount = 0;
};

static HSaveError restore_game_head_dynamic_values(Stream *in, RestoredData &r_data)
{
    r_data.FPS = in->ReadInt32();
    r_data.CursorMode = in->ReadInt32();
    r_data.CursorID = in->ReadInt32();
    SavegameComponents::ReadLegacyCameraState(in, r_data);
    set_loop_counter(in->ReadInt32());
    return HSaveError::None();
}

static void restore_game_spriteset(Stream *in)
{
    // ensure the sprite set is at least as large as it was
    // when the game was saved
    spriteset.EnlargeTo(in->ReadInt32() - 1); // they saved top_index + 1
    // get serialized dynamic sprites
    int sprnum = in->ReadInt32();
    while (sprnum) {
        unsigned char spriteflag = in->ReadByte();
        std::unique_ptr<Bitmap> image(read_serialized_bitmap(in));
        add_dynamic_sprite(sprnum, std::move(image));
        game.SpriteInfos[sprnum].Flags = spriteflag; // FIXME, don't set directly; are these flags even necessary?
        sprnum = in->ReadInt32();
    }
}

static HSaveError restore_game_scripts(Stream *in, const PreservedParams &pp, RestoredData &r_data)
{
    // read the global script data segment
    size_t gdatasize = (uint32_t)in->ReadInt32();
    if (pp.GlScDataSize != gdatasize)
    {
        return new SavegameError(kSvgErr_GameContentAssertion, "Mismatching size of global script data.");
    }
    r_data.GlobalScript.Len = gdatasize;
    r_data.GlobalScript.Data.resize(gdatasize);
    if (gdatasize > 0)
        in->Read(&r_data.GlobalScript.Data.front(), gdatasize);

    if ((uint32_t)in->ReadInt32() != numScriptModules)
    {
        return new SavegameError(kSvgErr_GameContentAssertion, "Mismatching number of script modules.");
    }
    r_data.ScriptModules.resize(numScriptModules);
    for (size_t i = 0; i < numScriptModules; ++i)
    {
        size_t module_size = in->ReadInt32();
        if (pp.ScMdDataSize[i] != module_size)
        {
            return new SavegameError(kSvgErr_GameContentAssertion, String::FromFormat("Mismatching size of script module data, module %d.", i));
        }
        r_data.ScriptModules[i].Len = module_size;
        r_data.ScriptModules[i].Data.resize(module_size);
        if (module_size > 0)
            in->Read(&r_data.ScriptModules[i].Data.front(), module_size);
    }
    return HSaveError::None();
}

static void ReadRoomStatus_Aligned(RoomStatus *roomstat, Stream *in, GameDataVersion data_ver)
{
    AlignedStream align_s(in, Common::kAligned_Read);
    roomstat->ReadFromFile_v321(&align_s, data_ver);
}

static void restore_game_room_state(Stream *in, GameDataVersion data_ver)
{
    displayed_room = in->ReadInt32();

    // read the room state for all the rooms the player has been in
    for (int vv = 0; vv < MAX_ROOMS; vv++)
    {
        int beenhere = in->ReadByte();
        if (beenhere)
        {
            RoomStatus *roomstat = getRoomStatus(vv);
            roomstat->beenhere = beenhere;

            if (roomstat->beenhere)
            {
                ReadRoomStatus_Aligned(roomstat, in, data_ver);
                if (roomstat->tsdatasize > 0)
                {
                    roomstat->tsdata.resize(roomstat->tsdatasize);
                    in->Read(roomstat->tsdata.data(), roomstat->tsdatasize);
                }
            }
        }
    }
}

static void ReadGameState_Aligned(Stream *in, GameDataVersion data_ver, RestoredData &r_data)
{
    AlignedStream align_s(in, Common::kAligned_Read);
    play.ReadFromSavegame(&align_s, data_ver, kGSSvgVersion_OldFormat, r_data);
}

static void restore_game_play(Stream *in, GameDataVersion data_ver, RestoredData &r_data)
{
    int screenfadedout_was = play.screen_is_faded_out;
    int roomchanges_was = play.room_changes;

    ReadGameState_Aligned(in, data_ver, r_data);
    r_data.Cameras[0].Flags = r_data.Camera0_Flags;

    play.screen_is_faded_out = screenfadedout_was;
    play.room_changes = roomchanges_was;

    char rbuffer[200]; // old doonceonly token length
    for (size_t i = 0; i < r_data.DoOnceCount; ++i)
    {
        StrUtil::ReadCStr(rbuffer, in, sizeof(rbuffer));
        play.do_once_tokens.insert(rbuffer);
    }

    // Skip gui_draw_order (no longer applied from saves)
    in->Seek(game.numgui * sizeof(int32_t));
}

static void ReadMoveList_Aligned(Stream *in)
{
    AlignedStream align_s(in, Common::kAligned_Read);
    for (int i = 0; i < game.numcharacters + MAX_ROOM_OBJECTS_v300 + 1; ++i)
    {
        mls[i].ReadFromFile_Legacy(&align_s);
        align_s.Reset();
    }
}

static void ReadGameSetupStructBase_Aligned(Stream *in, GameDataVersion data_ver, GameSetupStruct::SerializeInfo &info)
{
    AlignedStream align_s(in, Common::kAligned_Read);
    game.GameSetupStructBase::ReadFromFile(&align_s, data_ver, info);
}

static void ReadCharacterExtras_Aligned(Stream *in)
{
    AlignedStream align_s(in, Common::kAligned_Read);
    for (int i = 0; i < game.numcharacters; ++i)
    {
        charextra[i].ReadFromSavegame(&align_s, 0);
        align_s.Reset();
    }
}

static void restore_game_palette(Stream *in)
{
    in->ReadArray(&palette[0],sizeof(RGB),256);
}

static void restore_game_dialogs(Stream *in)
{
    for (int vv=0;vv<game.numdialog;vv++)
        in->ReadArrayOfInt32(&dialog[vv].optionflags[0],MAXTOPICOPTIONS);
}

static void restore_game_more_dynamic_values(Stream *in)
{
    mouse_on_iface=in->ReadInt32();
    in->ReadInt32(); // mouse_on_iface_button
    in->ReadInt32(); // mouse_pushed_iface
    ifacepopped = in->ReadInt32();
    game_paused=in->ReadInt32();
}

void ReadAnimatedButtons_Aligned(Stream *in, int num_abuts)
{
    AlignedStream align_s(in, Common::kAligned_Read);
    for (int i = 0; i < num_abuts; ++i)
    {
        AnimatingGUIButton abtn;
        abtn.ReadFromSavegame(&align_s, 0);
        AddButtonAnimation(abtn);
        align_s.Reset();
    }
}

inline bool AssertGameContent(HSaveError &err, int new_val, int original_val, const char *content_name)
{
    if (new_val != original_val)
    {
        err = new SavegameError(kSvgErr_GameContentAssertion,
            String::FromFormat("Mismatching number of %s (game: %d, save: %d).",
            content_name, original_val, new_val));
    }
    return new_val == original_val;
}

static HSaveError restore_game_gui(Stream *in, const ObjectCounts &guiwas)
{
    HError guierr = GUI::ReadGUI(in, true);
    if (!guierr)
        return new SavegameError(kSvgErr_GameObjectInitFailed, guierr);
    game.numgui = guis.size();

    HSaveError err;
    if (!AssertGameContent(err, game.numgui,      guiwas.GUICount,          "GUIs") ||
        !AssertGameContent(err, guibuts.size(),   guiwas.GUIButtonCount,    "GUI Buttons") ||
        !AssertGameContent(err, guiinv.size(),    guiwas.GUIInvWindowCount, "GUI InvWindows") ||
        !AssertGameContent(err, guilabels.size(), guiwas.GUILabelCount,     "GUI Labels") ||
        !AssertGameContent(err, guilist.size(),   guiwas.GUIListBoxCount,   "GUI ListBoxes") ||
        !AssertGameContent(err, guislider.size(), guiwas.GUISliderCount,    "GUI Sliders") ||
        !AssertGameContent(err, guitext.size(),   guiwas.GUITextBoxCount,   "GUI TextBoxes"))
        return err;

    RemoveAllButtonAnimations();
    int anim_count = in->ReadInt32();
    ReadAnimatedButtons_Aligned(in, anim_count);
    return HSaveError::None();
}

static HSaveError restore_game_audiocliptypes(Stream *in)
{
    if ((uint32_t)in->ReadInt32() != game.audioClipTypes.size())
    {
        return new SavegameError(kSvgErr_GameContentAssertion, "Mismatching number of Audio Clip Types.");
    }

    for (size_t i = 0; i < game.audioClipTypes.size(); ++i)
    {
        game.audioClipTypes[i].ReadFromFile(in);
    }
    return HSaveError::None();
}

static void restore_game_thisroom(Stream *in, RestoredData &r_data)
{
    in->ReadArrayOfInt16(r_data.RoomLightLevels, MAX_ROOM_REGIONS);
    in->ReadArrayOfInt32(r_data.RoomTintLevels, MAX_ROOM_REGIONS);
    in->ReadArrayOfInt16(r_data.RoomZoomLevels1, MAX_WALK_AREAS + 1);
    in->ReadArrayOfInt16(r_data.RoomZoomLevels2, MAX_WALK_AREAS + 1);
}

static void restore_game_ambientsounds(Stream *in, RestoredData &r_data)
{
    for (int i = 0; i < MAX_GAME_CHANNELS_v320; ++i)
    {
        ambient[i].ReadFromFile(in);
    }

    for (int bb = NUM_SPEECH_CHANS; bb < MAX_GAME_CHANNELS_v320; bb++) {
        if (ambient[bb].channel == 0)
            r_data.DoAmbient[bb] = 0;
        else {
            r_data.DoAmbient[bb] = ambient[bb].num;
            ambient[bb].channel = 0;
        }
    }
}

static void ReadOverlays_Aligned(Stream *in, std::vector<bool> &has_bitmap, size_t num_overs)
{
    AlignedStream align_s(in, Common::kAligned_Read);
    has_bitmap.resize(num_overs);
    // Remember that overlay indexes may be non-sequential
    auto &overs = get_overlays();
    for (size_t i = 0; i < num_overs; ++i)
    {
        bool has_bm;
        ScreenOverlay over;
        over.ReadFromFile(&align_s, has_bm, 0);
        align_s.Reset();
        if (over.type < 0)
            continue; // safety abort
        if (overs.size() <= static_cast<uint32_t>(over.type))
        {
            overs.resize(over.type + 1);
            has_bitmap.resize(over.type + 1);
        }
        overs[over.type] = std::move(over);
        has_bitmap[over.type] = has_bm;
    }
}

static void restore_game_overlays(Stream *in)
{
    size_t num_overs = in->ReadInt32();
    // Remember that overlay indexes may be not sequential,
    // the vector may be resized during read
    auto &overs = get_overlays();
    overs.resize(num_overs);
    std::vector<bool> has_bitmap(num_overs);
    ReadOverlays_Aligned(in, has_bitmap, num_overs);
    for (size_t i = 0; i < overs.size(); ++i) {
        if (has_bitmap[i])
            overs[i].SetImage(std::unique_ptr<Bitmap>(read_serialized_bitmap(in)),
                overs[i].offsetX, overs[i].offsetY);
    }
}

static void restore_game_dynamic_surfaces(Stream *in, RestoredData &r_data)
{
    // load into a temp array since ccUnserialiseObjects will destroy
    // it otherwise
    r_data.DynamicSurfaces.resize(MAX_DYNAMIC_SURFACES);
    for (int i = 0; i < MAX_DYNAMIC_SURFACES; ++i)
    {
        if (in->ReadInt8() == 0)
        {
            r_data.DynamicSurfaces[i] = nullptr;
        }
        else
        {
            r_data.DynamicSurfaces[i].reset(read_serialized_bitmap(in));
        }
    }
}

static void restore_game_displayed_room_status(Stream *in, GameDataVersion data_ver, RestoredData &r_data)
{
    int bb;
    for (bb = 0; bb < MAX_ROOM_BGFRAMES; bb++)
        r_data.RoomBkgScene[bb].reset();

    if (displayed_room >= 0) {

        for (bb = 0; bb < MAX_ROOM_BGFRAMES; bb++) {
            r_data.RoomBkgScene[bb] = nullptr;
            if (play.raw_modified[bb]) {
                r_data.RoomBkgScene[bb].reset(read_serialized_bitmap(in));
            }
        }
        bb = in->ReadInt32();

        if (bb)
            raw_saved_screen.reset(read_serialized_bitmap(in));

        // get the current troom, in case they save in room 600 or whatever
        ReadRoomStatus_Aligned(&troom, in, data_ver);

        if (troom.tsdatasize > 0) {
            troom.tsdata.resize(troom.tsdatasize);
            in->Read(troom.tsdata.data(),troom.tsdatasize);
        }
        else
            troom.tsdata.clear();
    }
}

static HSaveError restore_game_globalvars(Stream *in)
{
    if (in->ReadInt32() != numGlobalVars)
    {
        return new SavegameError(kSvgErr_GameContentAssertion, "Restore game error: mismatching number of Global Variables.");
    }

    for (int i = 0; i < numGlobalVars; ++i)
    {
        globalvars[i].Read(in);
    }
    return HSaveError::None();
}

static HSaveError restore_game_views(Stream *in)
{
    if (in->ReadInt32() != game.numviews)
    {
        return new SavegameError(kSvgErr_GameContentAssertion, "Mismatching number of Views.");
    }

    for (int bb = 0; bb < game.numviews; bb++) {
        for (int cc = 0; cc < views[bb].numLoops; cc++) {
            for (int dd = 0; dd < views[bb].loops[cc].numFrames; dd++)
            {
                views[bb].loops[cc].frames[dd].sound = in->ReadInt32();
                views[bb].loops[cc].frames[dd].pic = in->ReadInt32();
            }
        }
    }
    return HSaveError::None();
}

static HSaveError restore_game_audioclips_and_crossfade(Stream *in, GameDataVersion data_ver, RestoredData &r_data)
{
    if ((uint32_t)in->ReadInt32() != game.audioClips.size())
    {
        return new SavegameError(kSvgErr_GameContentAssertion, "Mismatching number of Audio Clips.");
    }

    for (int i = 0; i < TOTAL_AUDIO_CHANNELS_v320; ++i)
    {
        RestoredData::ChannelInfo &chan_info = r_data.AudioChans[i];
        chan_info.Pos = 0;
        chan_info.ClipID = in->ReadInt32();
        if (chan_info.ClipID >= 0)
        {
            if ((size_t)chan_info.ClipID >= game.audioClips.size())
            {
                return new SavegameError(kSvgErr_GameObjectInitFailed, "Invalid audio clip index.");
            }

            chan_info.Pos = in->ReadInt32();
            if (chan_info.Pos < 0)
                chan_info.Pos = 0;
            chan_info.Priority = in->ReadInt32();
            chan_info.Repeat = in->ReadInt32();
            chan_info.Vol = in->ReadInt32();
            in->ReadInt32(); // unused
            chan_info.VolAsPercent = in->ReadInt32();
            chan_info.Pan = in->ReadInt32();
            chan_info.Speed = 1000;
            if (data_ver >= kGameVersion_340_2)
                chan_info.Speed = in->ReadInt32();
        }
    }
    crossFading = in->ReadInt32();
    crossFadeVolumePerStep = in->ReadInt32();
    crossFadeStep = in->ReadInt32();
    crossFadeVolumeAtStart = in->ReadInt32();
    return HSaveError::None();
}

HSaveError restore_save_data_v321(Stream *in, GameDataVersion data_ver, const PreservedParams &pp, RestoredData &r_data)
{
    HSaveError err = restore_game_head_dynamic_values(in, r_data);
    if (!err)
        return err;
    restore_game_spriteset(in);

    err = restore_game_scripts(in, pp, r_data);
    if (!err)
        return err;
    restore_game_room_state(in, data_ver);
    restore_game_play(in, data_ver, r_data);
    ReadMoveList_Aligned(in);

    ObjectCounts objwas;
    objwas.CharacterCount = game.numcharacters;
    objwas.DialogCount = game.numdialog;
    objwas.InvItemCount = game.numinvitems;
    objwas.ViewCount = game.numviews;
    objwas.GUICount = game.numgui;
    objwas.GUIButtonCount = guibuts.size();
    objwas.GUIInvWindowCount = guiinv.size();
    objwas.GUILabelCount = guilabels.size();
    objwas.GUIListBoxCount = guilist.size();
    objwas.GUISliderCount = guislider.size();
    objwas.GUITextBoxCount = guitext.size();

    GameSetupStruct::SerializeInfo info;
    ReadGameSetupStructBase_Aligned(in, data_ver, info);

    if (!AssertGameContent(err, game.numcharacters, objwas.CharacterCount, "Characters") ||
        !AssertGameContent(err, game.numdialog,     objwas.DialogCount,    "Dialogs") ||
        !AssertGameContent(err, game.numinvitems,   objwas.InvItemCount,   "Inventory Items") ||
        !AssertGameContent(err, game.numviews,      objwas.ViewCount,      "Views"))
        return err;

    game.ReadFromSaveGame_v321(in, data_ver);

    // Modified custom properties are read separately to keep existing save format
    play.ReadCustomProperties_v340(in, data_ver);

    ReadCharacterExtras_Aligned(in);
    restore_game_palette(in);
    restore_game_dialogs(in);
    restore_game_more_dynamic_values(in);
    err = restore_game_gui(in, objwas);
    if (!err)
        return err;
    err = restore_game_audiocliptypes(in);
    if (!err)
        return err;
    restore_game_thisroom(in, r_data);
    restore_game_ambientsounds(in, r_data);
    restore_game_overlays(in);
    restore_game_dynamic_surfaces(in, r_data);
    restore_game_displayed_room_status(in, data_ver, r_data);
    err = restore_game_globalvars(in);
    if (!err)
        return err;
    err = restore_game_views(in);
    if (!err)
        return err;

    if (static_cast<uint32_t>(in->ReadInt32()) != (MAGICNUMBER + 1))
    {
        return new SavegameError(kSvgErr_InconsistentFormat, "MAGICNUMBER not found before Audio Clips.");
    }

    err = restore_game_audioclips_and_crossfade(in, data_ver, r_data);
    if (!err)
        return err;

    ReadPluginSaveData(in);
    if (static_cast<uint32_t>(in->ReadInt32()) != MAGICNUMBER)
        return new SavegameError(kSvgErr_InconsistentPlugin);

    // save the new room music vol for later use
    r_data.RoomVolume = (RoomVolumeMod)in->ReadInt32();

    if (ccUnserializeAllObjects(in, &ccUnserializer))
    {
        return new SavegameError(kSvgErr_GameObjectInitFailed,
            String::FromFormat("Managed pool deserialization failed: %s.",
                cc_get_error().ErrorString.GetCStr()));
    }

    // preserve legacy music type setting
    current_music_type = in->ReadInt32();

    return HSaveError::None();
}
