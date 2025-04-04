using Coral.Managed.Interop;
public class Internals
{
    // AudioModule
    public static unsafe delegate*<float,void> AudioModuleSetVolumeICall;
    public static unsafe delegate*<bool,void> AudioModuleSetMutedICall;

    // ExampleModule
    public static unsafe delegate*<void> ExampleModuleExampleFunctionICall;
    public static unsafe delegate*<NativeString,void> ExampleModuleExampleModuleLogICall;

}
