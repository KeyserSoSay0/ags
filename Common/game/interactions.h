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
#ifndef __AGS_CN_GAME__INTEREACTIONS_H
#define __AGS_CN_GAME__INTEREACTIONS_H

#include <memory>
#include "util/stream.h"
#include "util/string.h"

namespace AGS
{
namespace Common
{

// A list of script function names for all supported events
struct InteractionScripts
{
    std::vector<String> ScriptFuncNames;

    static InteractionScripts *CreateFromStream(Stream *in);
};

typedef std::shared_ptr<InteractionScripts> PInteractionScripts;

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_GAME__INTEREACTIONS_H
