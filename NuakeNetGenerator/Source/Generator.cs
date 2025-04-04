using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

struct Arg
{
    public string Name { get; set; }
    public string Type { get; set; }
}

struct Function
{
    public string Name { get; set; }
    public int NumArgs { get; set; }
    public Arg[] Args { get; set; }
    public string ReturnType { get; set; }
}

struct Module
{
    public string Name { get; set; }
    public Function[] Functions { get; set; }
}

struct Bindings
{
    public Module[] Modules { get; set; }
}

class Generator
{
    static string ConvertTypes(string type)
    {
        return type switch
        {
            "float" => "float",
            "int" => "int",
            "bool" => "bool",
            "void" => "void",
            "char" => "byte",
            "string" => "NativeString"
        };
    }

    static string GenerateInternals(Bindings bindings)
    {
        string generatedInternals = string.Empty;

        generatedInternals += "using Coral.Managed.Interop;\n";
        generatedInternals += "namespace Nuake.Net\r\n{\n";
        generatedInternals += "public class Internals\n";
        generatedInternals += "{\n";

        foreach (var module in bindings.Modules)
        {
            generatedInternals += $"    // {module.Name}\n";
            foreach (var function in module.Functions)
            {
                generatedInternals += $"    internal static unsafe delegate*<";

                if (function.NumArgs > 0)
                {
                    generatedInternals += $"{string.Join(", ", function.Args.Select(a => ConvertTypes(a.Type)))},";
                }

                generatedInternals += $"{function.ReturnType}>{module.Name}{function.Name}ICall;\n";
            }

            generatedInternals += "\n";
        }

        generatedInternals += "}\n";
        generatedInternals += "}\n";
        return generatedInternals;
    }

    static string GenerateModuleAPI(Module module)
    {
        string moduleApi = string.Empty;
        moduleApi += "using System;\n";
        moduleApi += "namespace Nuake.Net\n";
        moduleApi += "{\n";
        moduleApi += "    public class " + module.Name + "\n";
        moduleApi += "    {\n";

        foreach (Function func in module.Functions)
        {
            moduleApi += "        public static " + func.ReturnType + " " + func.Name + "(";
            if (func.NumArgs > 0)
            {
                moduleApi += string.Join(", ", func.Args.Select(a => a.Type + " " + a.Name));
            }
            moduleApi += ")\n";
            moduleApi += "        {\n";
            moduleApi += "            unsafe\n";
            moduleApi += "            {\n";
            if (func.ReturnType != "void")
            {
                moduleApi += $"            return ";
            }
            moduleApi += $"                 Internals.{module.Name}{func.Name}ICall(";
            if (func.NumArgs > 0)
            {
                moduleApi += string.Join(", ", func.Args.Select(a => a.Name));
            }

            moduleApi += ");\n";

            moduleApi += "            }\n";
            moduleApi += "        }\n";

        }
        moduleApi += "    }\n";
        moduleApi += "}\n";
        return moduleApi;
    }

    struct FileToWrite
    { 
        public string FileName { get; set; }
        public string Content { get; set; }
    }

    static void Main()
    {
        // Read file bindings.json
        string fullPath = Path.Combine(AppContext.BaseDirectory, "../../../../Editor/Build/Debug/Binaries/bindings.json");
        Console.WriteLine(fullPath);
        string json = File.ReadAllText(fullPath);

        // parse the json
        Bindings bindings = System.Text.Json.JsonSerializer.Deserialize<Bindings>(json);

        List<FileToWrite> filesToWrite = new List<FileToWrite>();

        string internals = GenerateInternals(bindings);
        filesToWrite.Add(new FileToWrite
        {
            FileName = "Internals.cs",
            Content = internals
        });

        Console.WriteLine("Generated Internals:\n");
        Console.WriteLine(internals);

        foreach (var module in bindings.Modules)
        {
            string fileName = module.Name + ".cs";
            Console.WriteLine("Generated " + fileName);
            string api = GenerateModuleAPI(module);
            Console.WriteLine(api);

            filesToWrite.Add(new FileToWrite
            {
                FileName = fileName,
                Content = api
            });
        }

        // WriteFiles
        foreach (var module in filesToWrite)
        {
            string fullfilePath = Path.Combine(AppContext.BaseDirectory, "../../../../NuakeNet/Source/Generated");
            string filePath = Path.Combine(fullfilePath, module.FileName);
            File.WriteAllText(filePath, module.Content);
            Console.WriteLine($"Wrote {module.FileName} to {filePath}");
        }
    }
}




