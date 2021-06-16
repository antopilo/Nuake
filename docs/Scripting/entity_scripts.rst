Entity scripts
==============

Entity scripts are Wren scripts that are attached to an entity using the WrenScript component.

After attaching the WrenScript component on an entity, you can select the script file directly.
In addition to pointing to the script file, you need to specify which class in the script is considered as the
entity script.

The class chosen must inherit from ScriptableEntity.

Here is an example of a barebone entity scripts::

    import "Scripts/ScriptableEntity" for ScriptableEntity

    class TestScript is ScriptableEntity {
        construct new() {
        }
        
        init() {

        }

        update(ts) {
           


        }

        exit() {
            
        }
    }