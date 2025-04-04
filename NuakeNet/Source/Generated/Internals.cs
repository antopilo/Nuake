using Coral.Managed.Interop;
namespace Nuake.Net
{
public class Internals
{
    // AudioModule
    internal static unsafe delegate*<float,void>AudioModuleSetVolumeICall;
    internal static unsafe delegate*<bool,void>AudioModuleSetMutedICall;
    internal static unsafe delegate*<void>AudioModuleHelloWorldICall;
    internal static unsafe delegate*<NativeString,void>AudioModuleSetNameICall;

    // ExampleModule
    internal static unsafe delegate*<void>ExampleModuleExampleFunctionICall;
    internal static unsafe delegate*<NativeString,void>ExampleModuleExampleModuleLog16ICall;

}
}
