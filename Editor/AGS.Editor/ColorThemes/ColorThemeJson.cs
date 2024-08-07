﻿using System;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Windows.Forms;
using Newtonsoft.Json.Linq;

namespace AGS.Editor
{
    public class ColorThemeJson : ColorTheme
    {
        private readonly string _dir;
        private JObject _json;

        public ColorThemeJson(string name, string dir) : base(name)
        {
            _dir = dir;
        }

        public override void Init() => _json = JObject.Parse(File.ReadAllText(_dir));

        public override bool Has(string id)
        {
            string[] tokens = id.Replace('.', '/').Split('/');
            JToken token = _json[tokens[0]];
            if (token == null) return false;

            bool found = true;
            tokens.Skip(1).ToList().ForEach(t => {
                if (!found) return; // can't break from ForEach
                if (token[t] == null)
                    found = false;
                else
                    token = token[t];
            });
            return found;
        }

        public override Color GetColor(string id)
        {
            JToken token = GetJToken(id, _json);
            if (token == null) throw new Exception();

            if (token.Type == JTokenType.String)
            {
                return DoTransform(id, t => System.Drawing.ColorTranslator.FromHtml((string)t) );
            }

            // old style compatibility
            if (token.Type == JTokenType.Object)
            {
                return DoTransform(id, t => Color.FromArgb((int)t["a"], (int)t["r"], (int)t["g"], (int)t["b"]));
            }

            throw new Exception();
        }

        public override int GetInt(string id)
        {
            return DoTransform(id, t => (int)t);
        }

        public override bool GetBool(string id)
        {
            return DoTransform(id, t => (bool)t);
        }

        public override ToolStripRenderer GetMainMenuRenderer(string id)
        {
            return DoTransform(id, t => new ToolStripProfessionalRenderer(new MainMenuColorTable(this, t.Path)));
        }

        public override ToolStripRenderer GetToolStripRenderer(string id)
        {
            return DoTransform(id, t => {
                var tspr = new ToolStripProfessionalRenderer(new ToolStripColorTable(this, t.Path));
                tspr.RoundedEdges = false;
                return tspr;
            });
        }

        public override ComboBox GetComboBox(string id, ComboBox original)
        {
            return DoTransform(id, t => new ComboBoxCustom(Factory.GUIController.ColorThemes, t.Path, original));
        }

        public override Image GetImage(string id, Image original)
        {
            return DoTransform(id, t =>
            {
                Bitmap b = new Bitmap(original, original.Size);

                for (int y = 0; y < original.Height; y++)
                {
                    for (int x = 0; x < original.Width; x++)
                    {
                        if (b.GetPixel(x, y).A > 0)
                        {
                            Color color;
                            try
                            {
                                color =  System.Drawing.ColorTranslator.FromHtml((string)t);
                            }
                            catch {
                                // old style compatibility
                                color = Color.FromArgb((int)t["a"], (int)t["r"], (int)t["g"], (int)t["b"]);
                            }
                            b.SetPixel(x, y, color);
                        }
                    }
                }

                return b;
            });
        }

        private T DoTransform<T>(string id, Func<JToken, T> transform) => transform(GetJToken(id, _json));

        private static JToken GetJToken(string id, JObject json)
        {
            string[] tokens = id.Replace('.', '/').Split('/');
            JToken token = json[tokens[0]];
            tokens.Skip(1).ToList().ForEach(t => token = token[t]);
            return token;
        }
    }
}
