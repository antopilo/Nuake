using Coral.Managed.Interop;

using System;

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
        public void Log()
        {
            NativeString str = "Hello from C#";
            unsafe { LoggerLogIcall(str); }
        }
    }
}