using Coral.Managed.Interop;

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Numerics;
using System.Runtime.InteropServices;

namespace Nuake.Net
{
    /// <summary>
    /// This is the core Nuake.Net API. 
    /// All internal call should happen in this file.
    /// </summary>

    public class Engine
    {
        internal static unsafe delegate*<NativeString, void> LoggerLogIcall;

        public Engine() { }

        /// <summary>
        /// Prints a message to the console log
        /// </summary>
        /// <param name="message">message to be printed</param>
        public static void Log(string input)
        {
            unsafe { LoggerLogIcall(input); }
        }
    }

    public class Debug
    {
        internal static unsafe delegate*</* start */ float, float, float,
                                         /* end */   float, float, float,
                                         /* color */ float, float, float, float,
                                        /* life */   float, 
                                                     void> DrawLineIcall;

        public Debug() { }

        public static void DrawLine(Vector3 start, Vector3 end, Vector4 color, float life = 0.0f)
        {
            unsafe
            {
                DrawLineIcall(start.X, start.Y, start.Z, end.X, end.Y, end.Z, color.X, color.Y, color.Z, color.W, life);
            }
        }
    }
    


}