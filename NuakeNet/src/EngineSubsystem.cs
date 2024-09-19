namespace Nuake.Net
{
    public class EngineSubsystem
    {
        internal static unsafe delegate*<int, bool, void> SetCanTickIcall;
        internal static unsafe delegate*<int, bool> GetCanTickIcall;
    
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
        public virtual void OnScenePreInit(Scene scene) {}
        public virtual void OnScenePostInit(Scene scene) {}
        public virtual void OnSceneUnloaded(Scene scene) {}
        public virtual void OnTick(float deltaTime) {}
    }
}
