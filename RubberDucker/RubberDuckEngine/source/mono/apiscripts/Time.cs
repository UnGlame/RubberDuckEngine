using System;
using System.Runtime.CompilerServices;
using System.Collections.Generic;

namespace Eclipse
{
    public static class Time
    {
        public static float deltaTime
        {
            get => getDeltaTime();
        }

        public static float fixedDeltaTime
        {
            get => getFixedDeltaTime();
        }

        public static float deltaTimeRegardlessPause
        {
            get => GetDeltaTimeRegardlessPause();
        }

        public static float fixedDeltaTimeRegardlessPause
        {
            get => GetFixedDeltaTimeRegardlessPause();
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static float getDeltaTime();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static float getFixedDeltaTime();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static float GetDeltaTimeRegardlessPause();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static float GetFixedDeltaTimeRegardlessPause();
    }
}