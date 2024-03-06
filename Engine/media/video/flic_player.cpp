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
#include "media/video/flic_player.h"
#include "ac/asset_helper.h"

#ifndef AGS_NO_VIDEO_PLAYER

namespace AGS
{
namespace Engine
{

using namespace Common;

FlicPlayer::~FlicPlayer()
{
    CloseImpl();
}

HError FlicPlayer::OpenImpl(std::unique_ptr<Common::Stream> data_stream,
    const String &/*name*/, int& /*flags*/, int /*target_depth*/)
{
    data_stream->Seek(8);
    const int fliwidth = data_stream->ReadInt16();
    const int fliheight = data_stream->ReadInt16();
    data_stream->Seek(0, kSeekBegin);

    PACKFILE *pf = PackfileFromStream(std::move(data_stream));
    if (open_fli_pf(pf) != FLI_OK)
    {
        pack_fclose(pf);
        return new Error("Failed to open FLI/FLC animation; could be an invalid or unsupported format");
    }
    _pf = pf;

    get_palette_range(_oldpal, 0, 255);

    _frameDepth = 8;
    _frameSize = Size(fliwidth, fliheight);
    _frameRate = 1000 / fli_speed;
    return HError::None();
}

void FlicPlayer::CloseImpl()
{
    close_fli();
    if (_pf)
        pack_fclose(_pf);
    _pf = nullptr;

    set_palette_range(_oldpal, 0, 255, 0);
}

bool FlicPlayer::NextVideoFrame(Bitmap *dst)
{
    // actual FLI playback state, base on original Allegro 4's do_play_fli

    /* get next frame */
    if (next_fli_frame(IsLooping() ? 1 : 0) != FLI_OK)
        return false;

    /* update the palette */
    if (fli_pal_dirty_from <= fli_pal_dirty_to)
        set_palette_range(fli_palette, fli_pal_dirty_from, fli_pal_dirty_to, TRUE);

    /* blit the changed portion of the frame */
    if (fli_bmp_dirty_from <= fli_bmp_dirty_to) {
        blit(fli_bitmap, dst->GetAllegroBitmap(), 0, fli_bmp_dirty_from, 0, fli_bmp_dirty_from,
            fli_bitmap->w, 1 + fli_bmp_dirty_to - fli_bmp_dirty_from);
    }

    reset_fli_variables();
    return true;
}

} // namespace Engine
} // namespace AGS

#endif // AGS_NO_VIDEO_PLAYER
