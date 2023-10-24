using Coral.Managed.Interop;
using System;

namespace Nuake.Net
{
    public class Scene
    {
        internal static unsafe delegate*<NativeString, int> GetEntityIcall;

        public static Entity GetEntity(string entityName)
        {
            int handle;
            unsafe { handle = GetEntityIcall(entityName); }

            if (handle == -1)
            {
                throw new Exception("Entity not found");
            }

            Entity entity = new Entity
            {
                ECSHandle = handle
            };
            return entity;
        }
    }
}