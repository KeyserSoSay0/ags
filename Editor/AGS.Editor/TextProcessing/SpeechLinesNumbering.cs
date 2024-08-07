using AGS.Types;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace AGS.Editor
{
    public class SpeechLinesNumbering : BaseTextProcess
    {
        public const string SPEECH_REFERENCE_FILE_NAME = "speechref.txt";

        private Dictionary<string, FunctionCallType> _speechableFunctionCalls;
        private CompileMessages _errors;
        private bool _includeNarrator;
        private bool _combineIdenticalLines;
        private bool _removeNumbering;
        private int? _characterID;
        private StreamWriter _referenceFile;

        public CompileMessages NumberSpeechLines(Game game, bool includeNarrator, bool combineIdenticalLines, bool removeNumbering, int? characterID)
        {
			_speechableFunctionCalls = GetFunctionCallsToProcessForSpeech(includeNarrator);

            _errors = new CompileMessages();
            _includeNarrator = includeNarrator;
            _combineIdenticalLines = combineIdenticalLines;
            _removeNumbering = removeNumbering;
            _characterID = characterID;

            if (Factory.AGSEditor.AttemptToGetWriteAccess(SPEECH_REFERENCE_FILE_NAME))
            {
                using (_referenceFile = new StreamWriter(SPEECH_REFERENCE_FILE_NAME, false))
                {
                    _referenceFile.WriteLine("// AGS auto-numbered speech lines output. This file was automatically generated.");
                    PerformNumbering(game);
                }
            }
            else
            {
                _errors.Add(new CompileError("unable to create file " + SPEECH_REFERENCE_FILE_NAME));
            }

            return _errors;
        }

        private void PerformNumbering(Game game)
        {
            SpeechLineProcessor processor = new SpeechLineProcessor(game, _includeNarrator,
                _combineIdenticalLines, _removeNumbering, _characterID, _speechableFunctionCalls, _errors, _referenceFile);

            foreach (Dialog dialog in game.RootDialogFolder.AllItemsFlat)
            {
                foreach (DialogOption option in dialog.Options)
                {
                    if (option.Say)
                    {
                        option.Text = processor.ProcessText(option.Text, GameTextType.DialogOption, game.PlayerCharacter.ID);
                    }
                }

                dialog.Script = processor.ProcessText(dialog.Script, GameTextType.DialogScript);
            }

            foreach (ScriptAndHeader script in game.RootScriptFolder.AllItemsFlat)
            {
                script.Script.Text = processor.ProcessText(script.Script.Text, GameTextType.Script);                
            }

            if (_errors.HasErrors)
            {
                // we're on a different thread here, so Invoke to show the errors
                if (Factory.GUIController.InvokeRequired)
                {
                    Factory.GUIController.Invoke(new Action(() => {
                        Factory.GUIController.ShowOutputPanel(_errors);
                    }));
                };
                throw new Exception("Errors encountered. Check the output window for details.");
            }

            Factory.AGSEditor.RunProcessAllGameTextsEvent(processor, _errors);
        }

    }
}
