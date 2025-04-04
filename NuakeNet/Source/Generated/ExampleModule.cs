using System;
namespace Nuake
{
    public static class ExampleModule
    {
        public static void ExampleFunction()
        {
            unsafe
            {
                 Internals.ExampleModuleExampleFunctionICall();
            }
        }
        public static void ExampleModuleLog(string hi)
        {
            unsafe
            {
                 Internals.ExampleModuleExampleModuleLogICall(hi);
            }
        }
    }
}
