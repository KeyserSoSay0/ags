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
#ifndef __AC_SCRIPTSTRING_H
#define __AC_SCRIPTSTRING_H

#include "ac/dynobj/cc_agsdynamicobject.h"

struct ScriptString final : AGSCCDynamicObject, ICCStringClass {
    // TODO: the preallocated text buffer may be assigned externally;
    // find out if it's possible to refactor while keeping same functionality
    char *text;

    int Dispose(const char *address, bool force) override;
    const char *GetType() override;
    void Unserialize(int index, AGS::Common::Stream *in, size_t data_sz) override;

    DynObjectRef CreateString(const char *fromText) override;

    ScriptString();
    ScriptString(const char *fromText);

protected:
    // Calculate and return required space for serialization, in bytes
    size_t CalcSerializeSize() override;
    // Write object data into the provided stream
    void Serialize(const char *address, AGS::Common::Stream *out) override;

private:
    size_t _len;
};

#endif // __AC_SCRIPTSTRING_H