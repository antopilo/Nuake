using Coral.Managed.Interop;
using System;

namespace Nuake.Net
{
    public class Scene
    {
        internal static unsafe delegate*<NativeString, int> GetEntityIcall;
        internal static unsafe delegate*<NativeString, NativeInstance<Entity>> GetEntityScriptIcall;

        public static T? GetEntity<T>(string entityName) where T : class
        {
            NativeInstance<Entity> handle;
            unsafe { handle = GetEntityScriptIcall(entityName); }

            Entity? entity = handle.Get();
            if (entity != null && entity is T)
            {
                return entity as T;
            }

            return null;
        }

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