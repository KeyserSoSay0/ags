﻿using System;
using System.Collections.Generic;
using System.Text;
using WeifenLuo.WinFormsUI.Docking;
using System.IO;
using System.Windows.Forms;

namespace AGS.Editor
{
    public class WindowsLayoutManager
    {
        public enum LayoutResult
        {
            OK,
            NoFile,
            LayoutException
        }

        private DockPanel _dockPanel;
        private List<DockContent> _startupPanes;
        private const string LAYOUT_FILENAME = "Layout.xml";
        private const string LAYOUT_RESOURCE = "LayoutDefault.xml";

        public WindowsLayoutManager(DockPanel dockPanel,
            List<DockContent> startupPanes)
        {
            _dockPanel = dockPanel;
            _startupPanes = startupPanes;            
        }

        public void SaveLayout()
        {
            string configFile = GetLayoutFile();
            SaveLayout(configFile);
        }

        public void SaveLayout(string path)
        {
            string folder = Path.GetDirectoryName(path);
            Directory.CreateDirectory(folder);
            File.SetAttributes(folder, FileAttributes.Normal);
            _dockPanel.SaveAsXml(path);
        }

        public LayoutResult LoadLayout()
        {
            string configFile = GetLayoutFile();
            return LoadLayout(configFile);
        }

        public LayoutResult LoadLayout(string path)
        {
            try
            {
                if (!File.Exists(path))
                    return LayoutResult.NoFile;
                DetachExistingPanes();
                _dockPanel.LoadFromXml(path, new
                    DeserializeDockContent(DeserializeContents));
                return LayoutResult.OK;
            }
            catch (Exception)
            {
                return LayoutResult.LayoutException;
            }
        }

        public bool ResetToDefaults()
        {
            string layout = Resources.ResourceManager.GetResourceAsString(LAYOUT_RESOURCE, Encoding.Unicode);
            if (string.IsNullOrEmpty(layout)) return false;
            byte[] byteArray = Encoding.Unicode.GetBytes(layout);
            Stream mems = new MemoryStream(byteArray, false);
            DetachExistingPanes();
            _dockPanel.LoadFromXml(mems, new
                    DeserializeDockContent(DeserializeContents));
            return true;
        }

        public void DetachAll()
        {
            DetachExistingPanes();
        }

        private void DetachExistingPanes()
        {
            for (int i = _dockPanel.Contents.Count - 1; i >= 0; i--)
            {
                IDockContent iContent = _dockPanel.Contents[i];
                DockContent content = (DockContent)iContent;
                content.DockPanel = null;
            }
        }

        private string GetLayoutFile()
        {
            return Path.Combine(Factory.AGSEditor.LocalAppData, LAYOUT_FILENAME);
        }

        private IDockContent DeserializeContents(string type)
        {
            foreach (DockContent pane in _startupPanes)
            {
                if (pane.GetType().ToString().Equals(type))
                {
                    return pane;
                }
            }
            return null;
        }
    }
}
