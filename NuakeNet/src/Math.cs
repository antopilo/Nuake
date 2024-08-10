using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Nuake.Net
{
    public class Maths
    {
        public static float AngleRepeat(float value, float max)
        {
            return (value % max + max) % max;
        }

        /// <summary>
        /// Lerps euler angles and prevents flipping.
        /// </summary>
        /// <param name="from"></param>
        /// <param name="to"></param>
        /// <param name="t"></param>
        /// <returns></returns>
        public static float LerpAngle(float from, float to, float t)
        {
            float delta = AngleRepeat(to - from, 360f);
            if (delta > 180f)
            {
                delta -= 360f;
            }
            return from + delta * Math.Max(0f, Math.Min(1f, t));
        }

        /// <summary>
        /// Normalize a 3D vector but if the length of the input vector is 0, then the return vector is of length 0.
        /// </summary>
        /// <param name="v"></param>
        /// <returns></returns>
        public static Vector3 NormalizeSafe(Vector3 v)
        {
            if (v.LengthSquared() == 0.0)
                return new Vector3();
            return Vector3.Normalize(v);
        }

        /// <summary>
        /// Normalize a 2D vector but if the length of the input vector is 0, then the return vector is of length 0.
        /// </summary>
        /// <param name="v"></param>
        /// <returns></returns>
        public static Vector2 NormalizeSafe(Vector2 v)
        {
            if (v.LengthSquared() == 0.0)
                return new Vector2();
            return Vector2.Normalize(v);
        }


        /// <summary>
        /// Normalize a 3D vector, and sets the Y component to 0. If the input length of the input vector is zero, then the return vector is of length 0.
        /// </summary>
        /// <param name="v"></param>
        /// <returns></returns>
        public static Vector3 NormalizeSafe2D(Vector3 v)
        {
            v.Y = 0f;
            if (v.LengthSquared() == 0.0)
                return new Vector3();

            return Vector3.Normalize(v);
        }


        /// <summary>
        /// Returns the length of a 3D vector taking only the X & Z components into account.
        /// </summary>
        /// <param name="input"></param>
        /// <returns></returns>
        private float Length2D(Vector3 input)
        {
            input.Y = 0;
            return input.Length();
        }
    }

    
}
