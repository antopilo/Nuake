local padding = "            "

-- Function to convert a file to a C++ byte array representation
local function fileToByteArray(file_path)
    local file = io.open(file_path, "rb")  -- Open file in binary mode
    if not file then
        print("Error: Could not open file " .. file_path)
        return nil
    end

    local byte_array = {}
    local byte_count = 0

    for byte in file:read("*a"):gmatch(".") do
        table.insert(byte_array, string.format("0x%02X", string.byte(byte)))
        byte_count = byte_count + 1
        if byte_count % 12 == 0 then
            table.insert(byte_array, ",\n".. padding)  -- Insert newline after the comma
        else
            table.insert(byte_array, ", ")  -- Insert comma between bytes
        end
    end

    file:close()
    return table.concat(byte_array)
end

-- Lua function to generate C++ header and source files from files in a directory
function generateStaticResources(input_directory, output_header, output_source)
    -- Remove existing header and source files (if any)
    os.remove(output_header)
    os.remove(output_source)

    -- Open the header file for writing
    local header_file = io.open(output_header, "w")
    local source_file = io.open(output_source, "w")

    -- Error handling
    if not header_file then
        print("Error: Could not open header file " .. output_header)
        return
    end

    if not source_file then
        print("Error: Could not open source file " .. output_source)
        header_file:close()
        return
    end

    -- Write initial guard and includes to header file
    header_file:write("#ifndef FILES_DATA_H\n")
    header_file:write("#define FILES_DATA_H\n\n")
    header_file:write("#include <string>\n\n")
    header_file:write("namespace Nuake {\n")
    header_file:write("    namespace StaticResources {\n")

    -- Write includes and namespace to source file
    source_file:write("#include \"StaticResources.h\"\n\n")
    source_file:write("namespace Nuake {\n")
    source_file:write("    namespace StaticResources {\n")

    -- Iterate over all files in the input directory and its subdirectories
    for _, file_path in ipairs(os.matchfiles(path.join(input_directory, "**"))) do
        -- Get the relative path of the file
        local relative_path = file_path

        -- Sanitize the relative path to make it suitable for C++ variable names
        local sanitized_path = relative_path:gsub("[^%w_]", "_")

        -- Generate C++ variable declarations for file path, size, and data
        header_file:write("        extern const std::string " .. sanitized_path .. "_path;\n")
        header_file:write("        extern unsigned int " .. sanitized_path .. "_len;\n")
        header_file:write("        extern unsigned char " .. sanitized_path .. "[];\n")

        -- Append C++ code to the source file for storing file data
        source_file:write("\n        // Data for file: " .. sanitized_path .. "_path\n")
        source_file:write("        const std::string " .. sanitized_path .. "_path = R\"(" .. relative_path .. ")\";\n")

        -- Convert file content to a C++ byte array
        local byte_array = fileToByteArray(file_path)
        if byte_array then
            source_file:write("        unsigned char " .. sanitized_path .. "[] = {\n".. padding .. byte_array .. "\n        };\n")
            source_file:write("        unsigned int " .. sanitized_path .. "_len = sizeof(" .. sanitized_path .. ");\n")
        else
            print("Error: Could not convert file to byte array " .. file_path)
        end
    end

    -- Close namespaces and guard in header file
    header_file:write("    }\n")
    header_file:write("}\n\n")
    header_file:write("#endif // FILES_DATA_H\n")

    -- Close namespaces in source file
    source_file:write("    }\n")
    source_file:write("}\n")

    -- Close the files
    header_file:close()
    source_file:close()

    print("Header file '" .. output_header .. "' generated with variable declarations.")
    print("Source file '" .. output_source .. "' generated with file data.")
end