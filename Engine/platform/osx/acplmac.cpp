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

#include "core/platform.h"

#if AGS_PLATFORM_OS_MACOS

// ********* MacOS PLACEHOLDER DRIVER *********

#include <SDL.h>
#include "platform/base/agsplatformdriver.h"
#include "util/directory.h"
#include "ac/common.h"
#include "main/main.h"

using namespace AGS::Common;

void AGSMacInitPaths(char appdata[PATH_MAX]);
void AGSMacGetBundleDir(char gamepath[PATH_MAX]);
int AGSMacGetFreeSpaceInMB(const char *path);
//bool PlayMovie(char const *name, int skipType);

static char libraryApplicationSupport[PATH_MAX];
static FSLocation commonDataPath;

struct AGSMac : AGSPlatformDriver
{
  AGSMac();
  void PreBackendInit() override;

  uint64_t GetDiskFreeSpaceMB(const String &path) override;
  eScriptSystemOSID GetSystemOSID() override;
  
  FSLocation GetUserSavedgamesDirectory() override;
  FSLocation GetAllUsersDataDirectory() override;
  FSLocation GetUserConfigDirectory() override;
  FSLocation GetAppOutputDirectory() override;
  const char *GetIllegalFileChars() override;
};

AGSMac::AGSMac()
{
  AGSMacInitPaths(libraryApplicationSupport);
  
  commonDataPath = FSLocation(libraryApplicationSupport).Concat("uk.co.adventuregamestudio");
}

void AGSMac::PreBackendInit()
{
  // Simulate right click with Ctrl + LMB
  // TODO: consider reading this option from user config in the future?
  SDL_SetHint(SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK, "1");
}

uint64_t AGSMac::GetDiskFreeSpaceMB(const String &path) {
  return AGSMacGetFreeSpaceInMB(path.GetCStr());
}

eScriptSystemOSID AGSMac::GetSystemOSID() {
  // override performed if `override.os` is set in config.
  return eOS_Mac;
}

FSLocation AGSMac::GetAllUsersDataDirectory()
{
  return commonDataPath;
}

FSLocation AGSMac::GetUserSavedgamesDirectory()
{
  return FSLocation(libraryApplicationSupport);
}

FSLocation AGSMac::GetUserConfigDirectory()
{
  return FSLocation(libraryApplicationSupport);
}

FSLocation AGSMac::GetAppOutputDirectory()
{
  return commonDataPath;
}

const char *AGSMac::GetIllegalFileChars()
{
  return "\\/:?\"<>|*"; // keep same as Windows so we can sync.
}

AGSPlatformDriver* AGSPlatformDriver::CreateDriver()
{
    return new AGSMac();
}

#endif
