-- Function to scan the "Modules" directory and load all module definitions
function loadModules(modulesDir)
    local modules = {}

    -- Get all folders in the Modules directory
    local dirs = os.matchdirs(path.join(modulesDir, "*"))

    for _, dir in ipairs(dirs) do
        local moduleName = path.getname(dir)
        local moduleFile = path.join(dir, "Module.lua")

        if os.isfile(moduleFile) then
            -- Load the module data
            local moduleData = dofile(moduleFile)

            -- Set the module name from the directory
            moduleData._name = moduleName

            print("Found module \"".. moduleData.name .."\" (".. moduleData._name ..")")
            
            table.insert(modules, moduleData)
        end
    end

    return modules
end

-- Function to generate the final C++ file with startup and shutdown functions
function generateModulesFile(modules, outputFilePath, sourceDir)
    local outputFile = io.open(outputFilePath, "w")

    if not outputFile then
        print("Error: Could not write to file ".. outputFilePath)
        return
    end

    outputFile:write("// Auto-generated modules management file\n")
    outputFile:write("#include \"Modules.h\"\n\n")

    -- Include module header files
    for _, module in ipairs(modules) do
        outputFile:write("#include \"".. module._name .."/".. module.module_header .."\"\n")
    end
    outputFile:write("\n")
    
    outputFile:write("#include \"Nuake/Core/Logger.h\"\n\n")

    -- Generate StartupModules function
    outputFile:write("void Nuake::Modules::StartupModules()\n{\n")
    for _, module in ipairs(modules) do
        outputFile:write("    Logger::Log(\"Starting ".. module._name .."\", \"modules\");\n")
        outputFile:write("    Module_".. module._name .."_Startup();\n")
    end
    outputFile:write("}\n\n")

    -- Generate ShutdownModules function
    outputFile:write("void Nuake::Modules::ShutdownModules()\n{\n")
    for _, module in ipairs(modules) do
        outputFile:write("    Logger::Log(\"Shutting down ".. module._name .."\", \"modules\");\n")
        outputFile:write("    Module_".. module._name .."_Shutdown();\n")
    end
    outputFile:write("}\n")

    outputFile:close()
    
    local sources = {}
    for _, module in ipairs(modules) do
        table.insert(sources, sourceDir .. "/" .. module._name .. "/" .. module.module_header)
        for _, v in ipairs(module.sources) do
            table.insert(sources, sourceDir .. "/" .. module._name .. "/" .. v)
        end
    end
    return sources
end