namespace Nuake.Net
{
    public class EngineSubsystem
    {
        internal static unsafe delegate* unmanaged<int, bool, void> SetCanTickIcall;
        internal static unsafe delegate* unmanaged<int, bool> GetCanTickIcall;
    
        public int EngineSubsystemID { get; protected set; }

        public bool CanTick
        {
            set
            {
                unsafe { SetCanTickIcall(EngineSubsystemID, value); }
            }
            get
            {
                unsafe { return GetCanTickIcall(EngineSubsystemID); }
            }
        }

        public virtual void Initialize() {}
        
        public virtual void OnScenePreInitialize(Scene scene) {}
        public virtual void OnSceneReady(Scene scene) {}
        public virtual void OnScenePreDestroy(Scene scene) {}
        
        public virtual void OnTick(float deltaTime) {}
        
        // Since the engine doesn't have the concept of scene instances, we just pass
        // a new `Scene` here since all functions in Scene are statics. This is largely
        // to keep the API a little more stable going forward.
        private void InternalOnScenePreInitialize()
        {
            OnScenePreInitialize(new Scene());
        }
        
        private void InternalOnSceneReady()
        {
            OnSceneReady(new Scene());
        }

        private void InternalOnScenePreDestroy()
        {
            OnScenePreDestroy(new Scene());
        }
    }
}
