﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace AGS.Types
{
    public class GraphicsFilterTypeConverter : BaseListSelectTypeConverter<string, string>
    {
        protected override Dictionary<string, string> GetValueList(ITypeDescriptorContext context)
        {
            if (context == null) return _invalidValueList;
            RuntimeSetup setup = context.Instance as RuntimeSetup;
            return setup.GraphicsFilterOptions;
        }
    }
}
