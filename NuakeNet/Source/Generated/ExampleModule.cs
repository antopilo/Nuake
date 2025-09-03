using System;
namespace Nuake.Net
{
    public class ExampleModule
    {
        public static void ExampleFunction()
        {
            unsafe
            {
                 Internals.ExampleModuleExampleFunctionICall();
            }
        }
        public static void ExampleModuleLog16(string hi2)
        {
            unsafe
            {
                 Internals.ExampleModuleExampleModuleLog16ICall(hi2);
            }
        }
    }
}
