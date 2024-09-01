using Coral.Managed.Interop;
using Nuake.Net.Shapes;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

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
        public static void Log(string input)
        {
            unsafe { LoggerLogIcall(input); }
        }
    }

    public struct AABB
    {
        public Vector3 A;
        public Vector3 B;

        public AABB(Vector3 a, Vector3 b)
        {
            A = a;
            B = b;
        }

        public AABB(float ax, float ay, float az, float bx, float by, float bz)
        {
            A = new Vector3(ax, ay, az);
            B = new Vector3(bx, by, bz);
        }
    }

    [AttributeUsage(AttributeTargets.Field)]
    public sealed class ExposedAttribute : Attribute
    {
    }

    [AttributeUsage(AttributeTargets.Class)]
    public sealed class BrushScript : Attribute
    {
        public string Description = "";
        public bool IsTrigger = false;

        public BrushScript(string description = "", bool isTrigger = false)
        {
            Description = description;
            IsTrigger = isTrigger;
        }
    }

    [AttributeUsage(AttributeTargets.Class)]
    public sealed class PointScript : Attribute
    {
        public string Description = "";
        public Vector3 AABBA;
        public Vector2 AABBB;

        public List<float> AABBData = new();
        public List<float> ColorData = new();

        public Color Color
        {
            get { return Color; }
            set
            {
                ColorData.Add(value.R);
                ColorData.Add(value.G);
                ColorData.Add(value.B);
            }
        }

        string Model;
        float ModelScale = 1.0f;
        string Sprite;

        public PointScript(string description = "")
        {
            Description = description;
        }
    }

    public class Debug
    {
        internal static unsafe delegate*</* start */ float, float, float,
                                         /* end */   float, float, float,
                                         /* color */ float, float, float, float,
                                        /* life */   float,
                                                     float,
                                                     void> DrawLineIcall;

        internal static unsafe delegate*</* position */ Vector3,
                                         /* rotation */ Quaternion,
                                         /* shape */    Vector3,
                                         /* color */    Vector4,
                                         /* life */     float,
                                                        float,
                                                        void> DrawShapeBoxIcall;

        internal static unsafe delegate*</* position */ Vector3,
                                         /* rotation */ Quaternion,
                                         /* shape */    float,
                                         /* color */    Vector4,
                                         /* life */     float,
                                                        float,
                                                        void> DrawShapeSphereIcall;

        internal static unsafe delegate*</* position */ Vector3,
                                         /* rotation */ Quaternion,
                                         /* shape */    float, float,
                                         /* color */    Vector4,
                                         /* life */     float,
                                                        float,
                                                        void> DrawShapeCylinderIcall;

        internal static unsafe delegate*</* position */ Vector3,
                                         /* rotation */ Quaternion,
                                         /* shape */    float, float,
                                         /* color */    Vector4,
                                         /* life */     float,
                                                        float,
                                                        void> DrawShapeCapsuleIcall;



        public Debug() { }

        public static void DrawLine(Vector3 start, Vector3 end, Vector4 color, float life = 0.0f, float width = 1.0f)
        {
            unsafe
            {
                DrawLineIcall(start.X, start.Y, start.Z, end.X, end.Y, end.Z, color.X, color.Y, color.Z, color.W, life, width);
            }
        }

        public static void DrawShape(Vector3 position, Quaternion rotation, Shapes.Box shape, Vector4 color, float life = 0.0f, float width = 1.0f)
        {
            unsafe
            {
                DrawShapeBoxIcall(position, rotation, new Vector3(shape.Width, shape.Height, shape.Depth), color, life, width);
            }
        }

        public static void DrawShape(Vector3 position, Quaternion rotation, Shapes.Sphere shape, Vector4 color, float life = 0.0f, float width = 1.0f)
        {
            unsafe
            {
                DrawShapeSphereIcall(position, rotation, shape.Radius, color, life, width);
            }
        }

        public static void DrawShape(Vector3 position, Quaternion rotation, Shapes.Cylinder shape, Vector4 color, float life = 0.0f, float width = 1.0f)
        {
            unsafe
            {
                DrawShapeCylinderIcall(position, rotation, shape.Radius, shape.Height, color, life, width);
            }
        }

        public static void DrawShape(Vector3 position, Quaternion rotation, Shapes.Capsule shape, Vector4 color, float life = 0.0f, float width = 1.0f)
        {
            unsafe
            {
                DrawShapeCapsuleIcall(position, rotation, shape.Radius, shape.Height, color, life, width);
            }
        }
    }

}