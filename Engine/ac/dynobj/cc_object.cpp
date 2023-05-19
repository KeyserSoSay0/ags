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
#include "ac/dynobj/cc_object.h"
#include "ac/dynobj/scriptobject.h"
#include "ac/dynobj/dynobj_manager.h"
#include "ac/common_defines.h"
#include "game/roomstruct.h"
#include "util/stream.h"

using namespace AGS::Common;

extern ScriptObject scrObj[MAX_ROOM_OBJECTS];

// return the type name of the object
const char *CCObject::GetType() {
    return "Object";
}

size_t CCObject::CalcSerializeSize(void* /*address*/)
{
    return sizeof(int32_t);
}

void CCObject::Serialize(void *address, Stream *out) {
    ScriptObject *shh = (ScriptObject*)address;
    out->WriteInt32(shh->id);
}

void CCObject::Unserialize(int index, Stream *in, size_t /*data_sz*/) {
    int num = in->ReadInt32();
    ccRegisterUnserializedPersistentObject(index, &scrObj[num], this);
}
