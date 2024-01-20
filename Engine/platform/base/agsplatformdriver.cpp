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
//
// AGS Platform-specific functions
//
//=============================================================================
#include "platform/base/agsplatformdriver.h"
#include <stdio.h>
#include <thread>
#include <SDL.h>
#include "ac/common.h"
#include "ac/runtime_defines.h"
#include "ac/timer.h"
#include "gfx/bitmap.h"
#include "media/audio/audio_system.h"
#include "platform/base/sys_main.h"
#include "util/string_utils.h"
#include "util/stream.h"

using namespace AGS::Common;
using namespace AGS::Engine;

#if defined (AGS_HAS_CD_AUDIO)
#include "libsrc/libcda-0.5/libcda.h"
#endif

// We don't have many places where we delay longer than a frame, but where we
// do, we should give the audio layer a chance to update.
// 16 milliseconds is rough period for 60fps
const auto MaximumDelayBetweenPolling = std::chrono::milliseconds(16);

std::unique_ptr<AGSPlatformDriver> AGSPlatformDriver::_instance;
AGSPlatformDriver *platform = nullptr;

// ******** DEFAULT IMPLEMENTATIONS *******

AGSPlatformDriver::AGSPlatformDriver()
{
    _writeStdOut = &AGSPlatformDriver::WriteStdOut;
}

void AGSPlatformDriver::AttachToParentConsole() { }
void AGSPlatformDriver::PauseApplication() { }
void AGSPlatformDriver::ResumeApplication() { }

Size AGSPlatformDriver::ValidateWindowSize(const Size &sz, bool borderless) const
{
    // TODO: Ideally we should also test for the minimal / maximal
    // allowed size of the window in current system here;
    // and more importantly: subtract window's border size.
    // SDL2 currently does not allow to get window border size
    // without creating a window first. But this potentially may be
    // acquired, at least on some platforms (e.g. Windows).
    SDL_Rect rc;
    SDL_GetDisplayUsableBounds(0, &rc);
    return Size::Clamp(sz, Size(1, 1), Size(rc.w, rc.h));
}

const char* AGSPlatformDriver::GetBackendFailUserHint()
{
    return "Make sure you have latest version of SDL2 libraries installed, and your system is running in graphical mode.";
}

const char *AGSPlatformDriver::GetDiskWriteAccessTroubleshootingText()
{
    return "Make sure you have write permissions, and also check the disk's free space.";
}

void AGSPlatformDriver::GetSystemTime(ScriptDateTime *sdt) {
    time_t t = time(nullptr);

    //note: subject to year 2038 problem due to shoving time_t in an integer
    sdt->rawUnixTime = static_cast<int>(t);

    struct tm *newtime = localtime(&t);
    sdt->hour = newtime->tm_hour;
    sdt->minute = newtime->tm_min;
    sdt->second = newtime->tm_sec;
    sdt->day = newtime->tm_mday;
    sdt->month = newtime->tm_mon + 1;
    sdt->year = newtime->tm_year + 1900;
}

void AGSPlatformDriver::DisplayAlert(const char *text, ...)
{
    char displbuf[2048];
    va_list ap;
    va_start(ap, text);
    vsnprintf(displbuf, sizeof(displbuf), text, ap);
    va_end(ap);

    // Print alert to the log system, to let other outputs receive it;
    // but use a dirty method to avoid duplicate message in stdout/stderr
    auto old_stdout = _writeStdOut;
    _writeStdOut = nullptr;
    Debug::Printf(kDbgMsg_Alert, "%s", displbuf);
    _writeStdOut = old_stdout;

    // Always write to either stderr or stdout, even if message boxes are enabled.
    (this->*_writeStdOut)("%s", displbuf);

    if (_guiMode)
        DisplayMessageBox(displbuf);
}

void AGSPlatformDriver::WriteStdOut(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
    fflush(stdout);
}

void AGSPlatformDriver::WriteStdErr(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

void AGSPlatformDriver::DisplayMessageBox(const char *text)
{
    if (_guiMode)
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Adventure Game Studio", text, sys_get_window());
}

void AGSPlatformDriver::YieldCPU() {
    // NOTE: this is called yield, but if we actually yield instead of delay,
    // we get a massive increase in CPU usage.
    this->Delay(1);
    //std::this_thread::yield();
}

SetupReturnValue AGSPlatformDriver::RunSetup(const ConfigTree &/*cfg_in*/, ConfigTree &/*cfg_out*/)
{
    return kSetup_Cancel;
}

void AGSPlatformDriver::SetCommandArgs(const char *const argv[], size_t argc)
{
    _cmdArgs = argv;
    _cmdArgCount = argc;
}

void AGSPlatformDriver::SetOutputToErr(bool on)
{
    _logToStdErr = on;
    _writeStdOut = _logToStdErr ? &AGSPlatformDriver::WriteStdErr : &AGSPlatformDriver::WriteStdOut;
}

String AGSPlatformDriver::GetCommandArg(size_t arg_index)
{
    return arg_index < _cmdArgCount ? _cmdArgs[arg_index] : nullptr;
}

//-----------------------------------------------
// IOutputHandler implementation
//-----------------------------------------------
void AGSPlatformDriver::PrintMessage(const Common::DebugMessage &msg)
{
    if (_writeStdOut)
    {
        if (msg.GroupName.IsEmpty())
            (this->*_writeStdOut)("%s", msg.Text.GetCStr());
        else
            (this->*_writeStdOut)("%s : %s", msg.GroupName.GetCStr(), msg.Text.GetCStr());
    }
}

AGSPlatformDriver* AGSPlatformDriver::GetDriver()
{
    if (!_instance)
        _instance.reset(CreateDriver());
    return _instance.get();
}

void AGSPlatformDriver::Shutdown()
{
    _instance.reset();
}

// ********** CD Player Functions common to Win and Linux ********

#if defined (AGS_HAS_CD_AUDIO)

// from ac_cdplayer
extern int use_cdplayer;
extern int need_to_stop_cd;

int numcddrives=0;

int cd_player_init() {
    int erro = cd_init();
    if (erro) return -1;
    numcddrives=1;
    use_cdplayer=1;
    return 0;
}

int cd_player_control(int cmdd, int datt) {
    // WINDOWS & LINUX VERSION
    if (cmdd==1) {
        if (cd_current_track() > 0) return 1;
        return 0;
    }
    else if (cmdd==2) {
        cd_play_from(datt);
        need_to_stop_cd=1;
    }
    else if (cmdd==3) 
        cd_pause();
    else if (cmdd==4) 
        cd_resume();
    else if (cmdd==5) {
        int first,last;
        if (cd_get_tracks(&first,&last)==0)
            return (last-first)+1;
        else return 0;
    }
    else if (cmdd==6)
        cd_eject();
    else if (cmdd==7)
        cd_close();
    else if (cmdd==8)
        return numcddrives;
    else if (cmdd==9) ;
    else quit("!CDAudio: Unknown command code");

    return 0;
}

#endif // AGS_HAS_CD_AUDIO

void AGSPlatformDriver::Delay(int millis) {
  auto now = AGS_Clock::now();
  auto delayUntil = now + std::chrono::milliseconds(millis);

  for (;;) {
    if (now >= delayUntil) { break; }

    auto duration = std::min<std::chrono::nanoseconds>(delayUntil - now, MaximumDelayBetweenPolling);
    std::this_thread::sleep_for(duration);
    now = AGS_Clock::now(); // update now

    if (now >= delayUntil) { break; }

    // don't allow it to check for debug messages, since this Delay()
    // call might be from within a debugger polling loop
    now = AGS_Clock::now(); // update now
  }
}
