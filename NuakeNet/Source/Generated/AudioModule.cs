using System;
namespace Nuake.Net
{
    public class AudioModule
    {
        public static void SetVolume(float volume)
        {
            unsafe
            {
                 Internals.SetVolumeICall(volume);
            }
        }
    }
}
