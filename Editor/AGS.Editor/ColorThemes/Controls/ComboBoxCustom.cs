﻿using System.Drawing;
using System.Drawing.Drawing2D;
using System.Reflection;
using System.Windows.Forms;

namespace AGS.Editor
{
    public class ComboBoxCustom : ComboBox
    {
        private const int WM_PAINT = 0x000F;

        private readonly IColorThemes _themes;
        private readonly string _root;

        public ComboBoxCustom(IColorThemes themes, string root, ComboBox original)
        {
            _themes = themes;
            _root = root;

            Dock = original.Dock;
            DropDownStyle = original.DropDownStyle;
            FormattingEnabled = original.FormattingEnabled;
            Location = original.Location;
            MaxDropDownItems = original.MaxDropDownItems;
            Name = original.Name;
            Size = original.Size;
            TabIndex = original.TabIndex;

            BeginUpdate();
            foreach (var i in original.Items)
            {
                Items.Add(i);
            }
            EndUpdate();

            SelectedIndex = original.SelectedIndex;

            var eventsField = typeof(System.ComponentModel.Component).GetField("events", BindingFlags.NonPublic | BindingFlags.Instance);
            var eventHandlerList = eventsField.GetValue(original);
            eventsField.SetValue(this, eventHandlerList);
        }

        protected override void OnCreateControl()
        {
            base.OnCreateControl();

            DrawMode = DrawMode.OwnerDrawVariable;
            FlatStyle = FlatStyle.Flat;
            BackColor = _themes.Current.GetColor(_root + "/background");
            ForeColor = _themes.Current.GetColor(_root + "/foreground");

            DropDown += (s, a) =>
            {
                BackColor = _themes.Current.GetColor(_root + "/drop-down/background");
                ForeColor = _themes.Current.GetColor(_root + "/drop-down/foreground");
            };

            DropDownClosed += (s, a) =>
            {
                BackColor = _themes.Current.GetColor(_root + "/drop-down-closed/background");
                ForeColor = _themes.Current.GetColor(_root + "/drop-down-closed/foreground");
            };

            DrawItem += (s, a) =>
            {
                if (a.Index >= 0)
                {
                    a.DrawBackground();

                    if ((a.State & DrawItemState.Selected) == DrawItemState.Selected)
                    {
                        a.Graphics.FillRectangle(new SolidBrush(_themes.Current.GetColor(_root + "/item-selected/background")),
                            a.Bounds);
                        a.Graphics.DrawString(Items[a.Index].ToString(), Font,
                            new SolidBrush(_themes.Current.GetColor(_root + "/item-selected/foreground")), a.Bounds);
                    }
                    else
                    {
                        a.Graphics.FillRectangle(
                            new SolidBrush(_themes.Current.GetColor(_root + "/item-not-selected/background")), a.Bounds);
                        a.Graphics.DrawString(Items[a.Index].ToString(), Font,
                            new SolidBrush(_themes.Current.GetColor(_root + "/item-not-selected/foreground")), a.Bounds);
                    }

                    a.DrawFocusRectangle();
                }
            };
        }

        protected override void WndProc(ref Message m)
        {
            base.WndProc(ref m);

            switch (m.Msg)
            {
                case WM_PAINT:
                    Graphics graphics = Graphics.FromHwnd(Handle);
                    Rectangle rectBorder = new Rectangle(0, 0, Width, Height);
                    Rectangle rectButton = new Rectangle(Width - 19, 0, 19, Height);
                    Brush arrow = new SolidBrush(SystemColors.ControlText);
                    graphics.SmoothingMode = SmoothingMode.HighQuality;
                    ControlPaint.DrawBorder(graphics, rectBorder, _themes.Current.GetColor(_root + "/border/background"),
                        ButtonBorderStyle.Solid);

                    if (DroppedDown)
                    {
                        graphics.FillRectangle(
                            new SolidBrush(_themes.Current.GetColor(_root + "/button-dropped-down/background")), rectButton);
                        arrow = new SolidBrush(_themes.Current.GetColor(_root + "/button-dropped-down/foreground"));
                    }
                    else
                    {
                        graphics.FillRectangle(
                            new SolidBrush(_themes.Current.GetColor(_root + "/button-not-dropped-down/background")), rectButton);
                        arrow = new SolidBrush(_themes.Current.GetColor(_root + "/button-not-dropped-down/foreground"));
                    }

                    GraphicsPath path = new GraphicsPath();
                    PointF topLeft = new PointF(Width - 13, (Height - 5) / 2);
                    PointF topRight = new PointF(Width - 6, (Height - 5) / 2);
                    PointF bottom = new PointF(Width - 9, (Height + 2) / 2);
                    path.AddLine(topLeft, topRight);
                    path.AddLine(topRight, bottom);

                    //Draw button arrow
                    graphics.FillPath(arrow, path);
                    break;
            }
        }
    }
}
