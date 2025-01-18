using Coral.Managed.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Nuake.Net
{
    public class Environment
    {
        internal static unsafe delegate* unmanaged<float, void> SetFocusDistanceIcall;
        internal static unsafe delegate* unmanaged<float> GetFocusDistanceIcall;

        public static void SetFocusDistance(float distance)
        {
            unsafe
            {
                SetFocusDistanceIcall(distance);
            }
        }

        public static float GetFocusDistance()
        {
            unsafe
            {
                return GetFocusDistanceIcall();
            }
        }
    }
}
