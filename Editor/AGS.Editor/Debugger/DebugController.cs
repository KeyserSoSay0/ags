using AGS.Types;
using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using AGS.Types.Enums;

namespace AGS.Editor
{
    public class DebugController
    {
        /// <summary>
        /// VariableInfo is a struct returned for a variable's value request.
        /// </summary>
        public struct VariableInfo
        {
            public string Type;
            public string Value;
            public string TypeHint;
            public string ErrorText;

            public VariableInfo(string type, string value, string hint, string errorText)
            {
                Type = type;
                Value = value;
                TypeHint = hint;
                ErrorText = errorText;
            }
        }

        public delegate void DebugStateChangedHandler(DebugState newState);
        public event DebugStateChangedHandler DebugStateChanged;
        public delegate void BreakAtLocationHandler(DebugCallStack callStack);
        public event BreakAtLocationHandler BreakAtLocation;
        public delegate void ReceiveVariableHandler(uint requestID, VariableInfo info);
        public event ReceiveVariableHandler ReceiveVariable;

        private DebugState _debugState = DebugState.NotRunning;
        private IEngineCommunication _communicator;
        private IntPtr _engineWindowHandle = IntPtr.Zero;

        public DebugController(IEngineCommunication communicator)
        {
            _communicator = communicator;
            _communicator.MessageReceived += new MessageReceivedHandler(_communicator_MessageReceived);
        }

		public bool CanUseDebugger
		{
			get { return _communicator.SupportedOnCurrentSystem; }
		}

        /// <summary>
        /// Tells whether Debugger is in a active working state.
        /// </summary>
        public bool IsActive
        {
            get { return _debugState != DebugState.NotRunning; }
        }

        /// <summary>
        /// Allows more than one instance of the AGS Editor to run simulatenously
        /// </summary>
        public string InstanceIdentifier
        {
            get { return _communicator.InstanceIdentifier; }
        }

        private void ChangeDebugState(DebugState newState)
        {
            _debugState = newState;

            if (DebugStateChanged != null)
            {
                DebugStateChanged(_debugState);
            }
        }

        private void _communicator_MessageReceived(XmlDocument doc)
        {
            string command = doc.DocumentElement.Attributes["Command"].InnerText;

            _engineWindowHandle = new IntPtr(Convert.ToInt32(doc.DocumentElement.SelectSingleNode("EngineWindow").InnerText));

            if ((command == "BREAK") || (command == "ERROR"))
            {
                ChangeDebugState(DebugState.Paused);
                if (BreakAtLocation != null)
                {
                    string errorMessage = null;
                    XmlNode errorMsgNode = doc.DocumentElement.SelectSingleNode("ErrorMessage");
                    if (errorMsgNode != null)
                    {
                        errorMessage = errorMsgNode.InnerText;
                    }
                    string callStack = doc.DocumentElement.SelectSingleNode("ScriptState").InnerText;
                    BreakAtLocation(ParseCallStackIntoObjectForm(callStack, errorMessage));
                }
            }
            else if (command == "EXIT")
            {
				EngineHasExited();
            }
            else if (command == "LOG")
            {
                XmlNode logTextNode = doc.DocumentElement.SelectSingleNode("Text");
                XmlNode logGroupIDNode = doc.DocumentElement.SelectSingleNode("GroupID");
                XmlNode logMTIDNode = doc.DocumentElement.SelectSingleNode("MTID");
                LogGroup group;
                LogLevel level;
                try
                {
                    group = (LogGroup)Convert.ToInt32(logGroupIDNode.InnerText);
                    level = (LogLevel)Convert.ToInt32(logMTIDNode.InnerText);
                }
                catch
                {
                    return;
                }
                LogMessage(logTextNode.InnerText, group, level);
            }
            else if (command == "RECVVAR")
            {
                if (ReceiveVariable != null)
                {
                    string reqID = doc.DocumentElement.SelectSingleNode("ReqID").InnerText;
                    var typeNode = doc.DocumentElement.SelectSingleNode("Type");
                    var valueNode = doc.DocumentElement.SelectSingleNode("Value");
                    var hintNode = doc.DocumentElement.SelectSingleNode("Hint");
                    var errorNode = doc.DocumentElement.SelectSingleNode("Error");
                    if (valueNode != null)
                    {
                        ReceiveVariable.Invoke(uint.Parse(reqID), new VariableInfo(
                            typeNode != null ? typeNode.InnerText : "",
                            valueNode.InnerText,
                            hintNode != null ? hintNode.InnerText : "",
                            errorNode != null ? errorNode.InnerText : null));
                    }
                    else
                    {
                        ReceiveVariable.Invoke(uint.Parse(reqID), new VariableInfo(
                            null, null, null,
                            errorNode != null ? errorNode.InnerText : null
                            ));
                    }
                }
            }
        }

        private DebugCallStack ParseCallStackIntoObjectForm(string callStackFromEngine, string errorMessage)
        {
            DebugCallStack result = new DebugCallStack(errorMessage);
            string[] callStackLines = callStackFromEngine.Split(new char[] { '\n' }, StringSplitOptions.RemoveEmptyEntries);
            foreach (string callStackEntry in callStackLines)
            {
                // There's a  (and more...)  if the call stack is too long, ignore it
                int firstSpeechMark = callStackEntry.IndexOf('"');
                if (firstSpeechMark >= 0)
                {
                    string callStack = callStackEntry.Substring(firstSpeechMark + 1);
                    string scriptName = callStack.Substring(0, callStack.IndexOf('"'));
                    string lineNumberText = callStack.Substring(callStack.IndexOf('"'));
                    lineNumberText = lineNumberText.Substring(lineNumberText.IndexOf("line ") + 5);
                    int i = 0;
                    while ((i < lineNumberText.Length) &&
                           (Char.IsDigit(lineNumberText, i)))
                    {
                        i++;
                    }
                    lineNumberText = lineNumberText.Substring(0, i);
                    result.AddLine(scriptName, Convert.ToInt32(lineNumberText));
                }
            }
            return result;
        }

        public void InitializeEngine(Game game, IntPtr editorHwnd)
        {
            _communicator.NewClient();
            _communicator.SendMessage("<Engine Command=\"START\" EditorWindow=\"" + editorHwnd + "\" />");
            ChangeDebugState(DebugState.Running);

            foreach (Script script in game.GetAllGameAndLoadedRoomScripts())
            {
                foreach (int line in script.BreakpointedLines)
                {
                    SetBreakpoint(script, line);
                }
            }

            _communicator.SendMessage("<Engine Command=\"READY\" EditorWindow=\"" + editorHwnd + "\" />");
        }

        public void LogMessage(string message, LogGroup group, LogLevel level)
        {
            Factory.GUIController.PrintEngineLog(message, group, level);
        }

		public void EngineHasExited()
		{
            _communicator.ClientHasExited();
            ChangeDebugState(DebugState.NotRunning);
			ClearCurrentLineMarker();
		}

        public void AddedBreakpoint(Script script, int lineNumber)
        {
            if (IsActive)
            {
                SetBreakpoint(script, lineNumber);
            }
        }

        public void RemovedBreakpoint(Script script, int lineNumber)
        {
            if (IsActive)
            {
                UnsetBreakpoint(script, lineNumber);
            }
        }

        private void SetBreakpoint(Script script, int lineNumber)
        {
            _communicator.SendMessage("<Engine Command=\"SETBREAK $" + script.FileName + "$" + lineNumber + "$\"></Engine>");
        }

        private void UnsetBreakpoint(Script script, int lineNumber)
        {
            _communicator.SendMessage("<Engine Command=\"DELBREAK $" + script.FileName + "$" + lineNumber + "$\"></Engine>");
        }

        private uint queryVariableCounter = 0; // for "unique" packet ids

        public bool QueryVariable(string varId, out uint requestKey)
        {
            if (!IsActive)
            {
                requestKey = 0;
                return false;
            }

            uint reqId = queryVariableCounter++;
            _communicator.SendMessage($"<Engine Command=\"GETVAR ${reqId}${varId}$\"></Engine>");
            requestKey = reqId;
            return true;
        }

        private void ClearCurrentLineMarker()
        {
            Factory.GUIController.ZoomToFile(string.Empty, 0, true, null);
        }

        private void SendCommandAndSwitchWindows(string command)
        {
            ClearCurrentLineMarker();

            ChangeDebugState(DebugState.Running);
            if (_engineWindowHandle != IntPtr.Zero)
            {
                if (Utilities.IsMonoRunning())
                {
                    //Is there any way of doing this in mono? I couldn't find any.
                    //I guess the user will have to bring the game back to focus by himself...                    
                }
                else
                {
                    NativeProxy.SetForegroundWindow(_engineWindowHandle);
                }
            }
            _communicator.SendMessage("<Engine Command=\"" + command + "\" />");
        }

        public void Resume()
        {
            SendCommandAndSwitchWindows("RESUME");
        }

        public void StepInto()
        {
            SendCommandAndSwitchWindows("STEP");
        }

        public void PauseExecution()
        {
            // the STEP command will break out at the next line
            SendCommandAndSwitchWindows("STEP");
        }

        public void StopDebugging()
        {
            SendCommandAndSwitchWindows("EXIT");
        }

        public void EditorShutdown()
        {
            _communicator.Dispose();
        }
    }
}
