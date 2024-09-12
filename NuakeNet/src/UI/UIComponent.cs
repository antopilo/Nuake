using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Nuake.Net
{
    [AttributeUsage(AttributeTargets.Field)]
    public sealed class UIWidgetInput : Attribute
    {
        private bool HasDefaultValue = false;
        private int DefaultValueInternalInt;
        private float DefaultValueInternalFloat;
        private bool DefaultValueInternalBool;
        private string DefaultValueInternalString;

        public UIWidgetInput(int DefaultValue)
        {
            DefaultValueInternalInt = DefaultValue;
            HasDefaultValue = true;
        }

        public UIWidgetInput(float DefaultValue)
        {
            this.DefaultValueInternalFloat = DefaultValue;
            HasDefaultValue = true;
        }

        public UIWidgetInput(string DefaultValue)
        {
            this.DefaultValueInternalString = DefaultValue;
            HasDefaultValue = true;
        }

        public UIWidgetInput(bool DefaultValue)
        {
            this.DefaultValueInternalBool = DefaultValue;
            HasDefaultValue = true;
        }

        public UIWidgetInput() { }
    }

    public class Style
    {
        protected ulong ID;

        public Style(ulong id)
        {
            ID = id;
        }
    }

    [AttributeUsage(AttributeTargets.Class)]
    public sealed class ExternalHTML : Attribute
    {
        private string HTMLPath;
        public ExternalHTML(string path)
        {
            HTMLPath = path;
        }
    }

    public class UIWidget
    {
        public Style Style { get; set; }

        private string HTML;

        protected ulong ID { get; set; }

        public UIWidget()
        { }

        protected UIWidget(ulong id)
        {
            ID = id;
            Style = new Style(id);
            HTML = "";
        }

        public virtual void OnInit() { }
        public virtual void OnTick(float dt) { }

        // Events
        public virtual void OnClick()
        {

        }
    }
}
