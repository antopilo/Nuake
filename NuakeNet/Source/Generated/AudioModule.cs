using System;
namespace Nuake.Net
{
    public class AudioModule
    {
        public static void SetVolume(float volume)
        {
            unsafe
            {
                 Internals.AudioModuleSetVolumeICall(volume);
            }
        }
        public static void SetMuted(bool muted)
        {
            unsafe
            {
                 Internals.AudioModuleSetMutedICall(muted);
            }
        }
        public static void HelloWorld()
        {
            unsafe
            {
                 Internals.AudioModuleHelloWorldICall();
            }
        }
        public static void SetName(string name)
        {
            unsafe
            {
                 Internals.AudioModuleSetNameICall(name);
            }
        }
    }
}
