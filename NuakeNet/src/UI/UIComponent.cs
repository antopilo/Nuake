using Coral.Managed;
using Coral.Managed.Interop;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
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

    public class Node
    {
        internal static unsafe delegate*<NativeString, NativeString, NativeString, NativeString, ulong> FindChildByIDICall;
        internal static unsafe delegate*<NativeString, NativeString, NativeString, bool> HasNativeInstanceICall;
        internal static unsafe delegate*<NativeString, NativeString, NativeInstance<Node>> GetNativeInstanceNodeICall;

        public string UUID;
        public string CanvasUUID;

        public string ID { get; set; } = "";
        public List<string> Classes { get; set; } = new();

        /// <summary>
        /// Traverse the DOM starting from this node to find a child from a unique ID
        /// </summary>
        /// <typeparam name="T">Type of node</typeparam>
        /// <param name="id">Unique id of the node to find</param>
        /// <returns></returns>
        /// <exception cref="Exception">Node not found</exception>
        public T FindChildByID<T>(string id) where T : Node
        {
            unsafe
            {
                Engine.Log("Calling FindChildByIDICall with params: " + CanvasUUID + " , " + UUID + " , " + id);

                ulong uuid = FindChildByIDICall(CanvasUUID, CanvasUUID, UUID, id);
                if (uuid == 0)
                {
                    throw new Exception("Node not found");
                }

                Node? newNode = null;
                //if (HasNativeInstanceICall(CanvasUUID, CanvasUUID, UUID))
                //{
                //    NativeInstance<Node> handle;
                //    unsafe { handle = GetNativeInstanceNodeICall(CanvasUUID, UUID); }
                //    newNode = handle.Get();
                //}

                if(newNode == null)
                {
                    newNode = new Node()
                    {
                        UUID = uuid.ToString(),
                        CanvasUUID = CanvasUUID
                    };

                    return newNode as T;
                }

                return null;
            }
        }
    }

    public class TextNode : Node
    {
        internal static unsafe delegate*<NativeString, NativeString, NativeString, void> SetTextNodeTextICall;
        internal static unsafe delegate*<NativeString, NativeString, NativeString> GetTextNodeTextICall;

        public string Text 
        { 
            get
            {
                unsafe
                {
                    return GetTextNodeTextICall(this.CanvasUUID, this.UUID).ToString();
                }
            }
            set
            {
                unsafe
                {
                    SetTextNodeTextICall(this.CanvasUUID, this.UUID, value);
                }
            }
        }
    }

    public class UIWidget : Node
    {
        public Style Style { get; set; }

        public UIWidget() { }

        public virtual void OnInit() { }
        public virtual void OnTick(float dt) { }

        // Events
        public virtual void OnClick() { }
    }
}
