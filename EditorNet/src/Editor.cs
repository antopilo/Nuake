using Nuake.Commands;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace Nuake
{
    class Editor
    {
        internal static unsafe delegate* unmanaged<int, void> SetSelectedEntityIcall;
        internal static unsafe delegate* unmanaged<int> GetSelectedEntityIcall;

        static int STACK_CAPACITY = 32;
        private static Stack<ICommand> CommandStack = new(STACK_CAPACITY);


        public static int SelectedEntity
        {
            get
            {
                unsafe
                {
                    return GetSelectedEntityIcall();
                }
            }

            set
            {
                unsafe
                {
                    SetSelectedEntityIcall(value);
                }
            }
        }

        static void Initialize()
        {
            RegisterCommand();
        }

        static void RegisterCommand()
        {
            SelectEntity testCommand = new SelectEntity(10);
            testCommand.Do();
            testCommand.Undo();

            SetEntityFieldProperty test = new SetEntityFieldProperty(1, "LightComponent.Intensity", "10");
            test.Do();
            test.Undo();
            test.Do();
        }

        public static void PushCommand(String command, String value)
        {

        }

        public static void PushCommand(ICommand command)
        {
            CommandStack.Push(command);
            command.Do();
        }

        static void Undo()
        {

        }

        static void Redo()
        {

        }
    }


    namespace Commands
    {
        interface ICommand
        {
            abstract string GetName();
            public abstract void Do();
            public virtual bool Undo() { return false; }
        }

        class SetEntityFieldProperty : ICommand
        {
            private object Instance;
            private object PropertyValue;
            private object PropertyOldValue;
            private PropertyInfo PropertyInfo;

            public SetEntityFieldProperty(int id, string name, string value)
            {
                // Parse the input string into component name and field name
                string[] parts = name.Split('.');
                if (parts.Length != 2)
                {
                    throw new ArgumentException("The input format should be 'ComponentName.FieldName'");
                }

                string componentName = parts[0];
                string fieldName = parts[1];

                // Load the Nuake.Net assembly
                Assembly nuakeAssembly = AppDomain.CurrentDomain.GetAssemblies()
                    .FirstOrDefault(a => a.GetName().Name == "NuakeNet");

                if (nuakeAssembly == null)
                {
                    throw new InvalidOperationException("Nuake.Net assembly not found.");
                }

                // Find the component class within the specified namespace that matches the component name
                var componentType = nuakeAssembly.GetTypes()
                    .FirstOrDefault(t => t.Name.Equals(componentName, StringComparison.OrdinalIgnoreCase) && t.IsClass);

                if (componentType == null)
                {
                    Nuake.Net.Engine.Log($"Component '{componentName}' not found in the Nuake.Net assembly.");
                }

                // Find the field within the component class
                PropertyInfo = componentType.GetProperty(fieldName, BindingFlags.Public | BindingFlags.Instance | BindingFlags.IgnoreCase);
                if (PropertyInfo == null)
                {
                    Nuake.Net.Engine.Log(($"Field '{fieldName}' not found in component '{componentName}'."));
                }

                // Assume we have an instance of the component (you would need to pass the correct instance here)
                Instance = Activator.CreateInstance(componentType, id); // Example: creating an instance for demonstration

                // Convert the value to the correct type and set the property
                object convertedValue = Convert.ChangeType(value, PropertyInfo.PropertyType);

                PropertyOldValue = PropertyInfo.GetValue(Instance);
                PropertyValue = convertedValue;

                Nuake.Net.Engine.Log($"Set '{fieldName}' of '{componentName}' to '{value}' in Nuake.Net.");
            }

            public virtual void Do()
            {
                Nuake.Net.Engine.Log($"'{PropertyValue}' in Nuake.Net.");
                PropertyInfo.SetValue(Instance, PropertyValue);

            }

            public string GetName()
            {
                return "SetFieldProperty";
            }

            public virtual bool Undo()
            {
                PropertyInfo.SetValue(Instance, PropertyOldValue);

                Nuake.Net.Engine.Log($"'{PropertyOldValue}' in Nuake.Net.");

                return true;
            }
        }

        class SetStaticFieldProperty : ICommand
        {
            private object PropertyValue;
            private object PropertyOldValue;
            private PropertyInfo PropertyInfo;

            public SetStaticFieldProperty(string name, string value)
            {
                // Parse the input string into component name and field name
                string[] parts = name.Split('.');
                if (parts.Length != 2)
                {
                    throw new ArgumentException("The input format should be 'ComponentName.FieldName'");
                }

                string componentName = parts[0];
                string fieldName = parts[1];

                // Load the Nuake.Net assembly
                Assembly nuakeAssembly = AppDomain.CurrentDomain.GetAssemblies()
                    .FirstOrDefault(a => a.GetName().Name == "NuakeNet");

                if (nuakeAssembly == null)
                {
                    throw new InvalidOperationException("Nuake.Net assembly not found.");
                }

                // Find the component class within the specified namespace that matches the component name
                var componentType = nuakeAssembly.GetTypes()
                    .FirstOrDefault(t => t.Name.Equals(componentName, StringComparison.OrdinalIgnoreCase) && t.IsClass);

                if (componentType == null)
                {
                    Nuake.Net.Engine.Log($"Component '{componentName}' not found in the Nuake.Net assembly.");
                }

                // Find the field within the component class
                PropertyInfo = componentType.GetProperty(fieldName, BindingFlags.Static | BindingFlags.Public | BindingFlags.Instance | BindingFlags.IgnoreCase);
                if (PropertyInfo == null)
                {
                    Nuake.Net.Engine.Log(($"Static Field '{fieldName}' not found in component '{componentName}'."));
                }
                
                // Convert the value to the correct type and set the property
                object convertedValue = Convert.ChangeType(value, PropertyInfo.PropertyType);

                PropertyOldValue = PropertyInfo.GetValue(null);
                PropertyValue = convertedValue;

                Nuake.Net.Engine.Log($"Set '{fieldName}' of '{componentName}' to '{value}' in Nuake.Net.");
            }

            public virtual void Do()
            {
                PropertyInfo.SetValue(null, PropertyValue);

                Nuake.Net.Engine.Log($"'{PropertyValue}' in Nuake.Net.");
            }

            public string GetName()
            {
                return "SetFieldProperty";
            }

            public virtual bool Undo()
            {
                PropertyInfo.SetValue(null, PropertyOldValue);

                Nuake.Net.Engine.Log($"'{PropertyOldValue}' in Nuake.Net.");

                return true;
            }
        }

        class SelectEntity(int entityID) : ICommand
        {
            private readonly int EntityID = entityID;
            private readonly int PreviousEntityID = Editor.SelectedEntity;

            public void Do()
            {
                Editor.SelectedEntity = EntityID;
            }

            public bool Undo()
            {
                Editor.SelectedEntity = PreviousEntityID;
                return true;
            }

            public string GetName()
            {
                return "SelectEntity";
            }
        }
    }
}

