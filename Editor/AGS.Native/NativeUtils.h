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
// Declarations of the utility functions used on native side.
//
//=============================================================================
#pragma once

#include "util/string.h"

typedef AGS::Common::String AGSString;

System::String^ ToStr(const AGS::Common::String &str);
System::String^ ToStrUTF8(const AGS::Common::String &str);

AGSString ConvertStringToNativeString(System::String^ clrString);
AGSString ConvertStringToNativeString(System::String^ clrString, size_t buf_len);
AGSString ConvertPathToNativeString(System::String^ clrString);
void ConvertStringToCharArray(System::String^ clrString, char *buf, size_t buf_len);
void ConvertFileNameToCharArray(System::String^ clrString, char *buf, size_t buf_len);

extern AGSString editorVersionNumber;
