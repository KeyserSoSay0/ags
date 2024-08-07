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
#ifndef __AGS_EE_AC__DIALOG_H
#define __AGS_EE_AC__DIALOG_H

#include <vector>
#include "ac/dialogtopic.h"
#include "ac/dynobj/scriptobjects.h"

int  Dialog_GetID(ScriptDialog *sd);
int  Dialog_GetOptionCount(ScriptDialog *sd);
int  Dialog_GetShowTextParser(ScriptDialog *sd);
const char* Dialog_GetOptionText(ScriptDialog *sd, int option);
int  Dialog_DisplayOptions(ScriptDialog *sd, int sayChosenOption);
int  Dialog_GetOptionState(ScriptDialog *sd, int option);
int  Dialog_HasOptionBeenChosen(ScriptDialog *sd, int option);
void Dialog_SetOptionState(ScriptDialog *sd, int option, int newState);
void Dialog_Start(ScriptDialog *sd);

// Starts a dialog
void do_conversation(int dlgnum);
// Displays dialog options, and returns the chosen number, or CHOSE_TEXTPARSER if parser input was activated
int show_dialog_options(int dlgnum, bool runGameLoopsInBackground);
// Handles a dialog option, optionally "sais" its text, optionally run corresponding dialog script's entry
int run_dialog_option(int dlgnum, int dialog_choice, int sayChosenOption, bool run_script);

extern std::vector<ScriptDialog> scrDialog;
extern std::vector<DialogTopic> dialog;

#endif // __AGS_EE_AC__DIALOG_H
