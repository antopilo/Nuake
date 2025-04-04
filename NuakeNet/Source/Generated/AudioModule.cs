using System;
namespace Nuake
{
    public static class AudioModule
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
    }
}
