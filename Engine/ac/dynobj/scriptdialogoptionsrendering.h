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

#ifndef __AC_SCRIPTDIALOGOPTIONSRENDERING_H
#define __AC_SCRIPTDIALOGOPTIONSRENDERING_H

#include "ac/dynobj/scriptdrawingsurface.h"

struct ScriptDialogOptionsRendering final : AGSCCDynamicObject {
    int x, y, width, height;
    int parserTextboxX, parserTextboxY;
    int parserTextboxWidth;
    int dialogID;
    int activeOptionID;
    int chosenOptionID;
    ScriptDrawingSurface *surfaceToRenderTo;
    bool surfaceAccessed;
    bool needRepaint;

    // return the type name of the object
    const char *GetType() override;
    void Unserialize(int index, AGS::Common::Stream *in, size_t data_sz) override;
    void Reset();
    ScriptDialogOptionsRendering();

protected:
    // Calculate and return required space for serialization, in bytes
    size_t CalcSerializeSize(void *address) override;
    // Write object data into the provided stream
    void Serialize(void *address, AGS::Common::Stream *out) override;
};


#endif // __AC_SCRIPTDIALOGOPTIONSRENDERING_H