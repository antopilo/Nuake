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

    public enum Visibility
    {
        Hidden,
        Visible,
    }


    public class Node
    {
        internal static unsafe delegate* unmanaged<NativeString, NativeString, NativeString, NativeString> FindChildByIDICall;
        internal static unsafe delegate* unmanaged<NativeString, NativeString, bool> HasNativeInstanceICall;
        internal static unsafe delegate* unmanaged<NativeString, NativeString, NativeInstance<Node>> GetNativeInstanceNodeICall;
        internal static unsafe delegate* unmanaged<NativeString, NativeString, Bool32, void> SetVisibilityICall;
        internal static unsafe delegate* unmanaged<NativeString, NativeString, Bool32> GetVisibilityICall;

        internal static unsafe delegate* unmanaged<NativeString, NativeString, float> GetHeightPercentageICall;
        internal static unsafe delegate* unmanaged<NativeString, NativeString, float, void> SetHeightPercentageICall;
        internal static unsafe delegate* unmanaged<NativeString, NativeString, float> GetWidthPercentageICall;
        internal static unsafe delegate* unmanaged<NativeString, NativeString, float, void> SetWidthPercentageICall;

        public string UUID;
        public string CanvasUUID;

        public Node() { }
        public Node(string handle, string canvasHandle )
        {
            UUID = handle;
            CanvasUUID = canvasHandle; 
        }

        public string ID { get; set; } = "";
        public List<string> Classes { get; set; } = new();

        public float GetWidthPercentage()
        {
            unsafe
            {
                return GetHeightPercentageICall(CanvasUUID, UUID);
            }
        }

        public void SetWidthPercentage(float percentage)
        {
            unsafe
            {
                if (percentage < 0.0f)
                {
                    throw new ArgumentException("Percentage has to be greater than 0.");
                }

                SetWidthPercentageICall(CanvasUUID, UUID, percentage);
            }
        }

        public float GetHeightPercentage()
        {
            unsafe
            {
                return GetHeightPercentageICall(CanvasUUID, UUID);
            }
        }

        public void SetHeightPercentage(float percentage)
        {
            unsafe
            {
                if(percentage < 0.0f)
                {
                    throw new ArgumentException("Percentage has to be greater than 0.");
                }

                SetHeightPercentageICall(CanvasUUID, UUID, percentage);
            }
        }

        public Visibility Visibility
        {
            get
            {
                unsafe
                {
                    return GetVisibilityICall(CanvasUUID, UUID) ? Visibility.Visible : Visibility.Hidden;
                }
            }
            set
            {
                unsafe
                {
                    SetVisibilityICall(CanvasUUID, UUID, value == Visibility.Visible);
                }
            }
        }

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

                NativeString uuid = FindChildByIDICall(CanvasUUID, UUID, id);
                if (uuid.ToString() == "0")
                {
                    throw new Exception("Node not found");
                }

                if (HasNativeInstanceICall(CanvasUUID, uuid))
                {
                    NativeInstance<Node> handle;
                    unsafe { handle = GetNativeInstanceNodeICall(CanvasUUID, uuid); }
                    Node? newNodeInstance = handle.Get();
                    if(newNodeInstance != null && newNodeInstance is T)
                    {
                        return newNodeInstance as T;
                    }
                }

                T? newNode = Activator.CreateInstance<T>();
                newNode.UUID = uuid;
                newNode.CanvasUUID = CanvasUUID;
                return newNode as T;
            }
        }
    }

    public class TextNode : Node
    {
        internal static unsafe delegate* unmanaged<NativeString, NativeString, NativeString, void> SetTextNodeTextICall;
        internal static unsafe delegate* unmanaged<NativeString, NativeString, NativeString> GetTextNodeTextICall;

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
