using System;
using System.Runtime.CompilerServices;
using System.Collections.Generic;

namespace Eclipse
{
    public class Vector3
    {
        static Vector3()
        {
            zero = new Vector3(0, 0, 0);
            forward = new Vector3(0, 0, 1);
            up = new Vector3(0, 1, 0);
        }

        public Vector3(float _x, float _y, float _z)
        {
            x = _x;
            y = _y;
            z = _z;
        }

        public float x;
        public float y;
        public float z;

        public static readonly Vector3 zero;
        public static readonly Vector3 forward;
        public static readonly Vector3 up;

        public static Vector3 operator *(Vector3 a, float val)
        {
            return new Vector3(a.x * val, a.y * val, a.z * val);
        }

        public static Vector3 operator /(Vector3 a, float val)
        {
            return new Vector3(a.x / val, a.y / val, a.z / val);
        }

        public static Vector3 operator +(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
        }

        public static Vector3 operator -(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
        }

        public static float Distance(Vector3 a, Vector3 b)
        {
            return (float)Math.Sqrt((a.x - b.x) * (a.x - b.x) + 
                    (a.y - b.y) * (a.y - b.y) +
                    (a.z - b.z) * (a.z - b.z));
        }

        public static float Angle(Vector3 a, Vector3 b)
        {
            float dotproduct = a.x * b.x + a.y * b.y + a.z * b.z;
            float magA = (float)Math.Sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
            float magB = (float)Math.Sqrt(b.x * b.x + b.y * b.y + b.z * b.z);
            return (float)Math.Acos(dotproduct / (magA * magB));
        }

        public static Vector3 CrossProduct(Vector3 a, Vector3 b)
        {
            Vector3 cross = new Vector3(0, 0, 0);
            cross.x = a.y * b.z - a.z * b.y;
            cross.y = a.z * b.x - a.x * b.z;
            cross.z = a.x * b.y - a.y * b.x;
            return cross;
        }

        public static float DotProduct(Vector3 a, Vector3 b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        public override string ToString() => $"{x} {y} {z}";
    }
}