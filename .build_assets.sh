#!/bin/bash

# Directory containing the files
input_directory="Resources"

# Output C++ header file for variable declarations
output_header="Nuake/src/Resource/StaticResources.h"

# Output C++ source file for data storage
output_source="Nuake/src/Resource/StaticResources.cpp"

# Remove existing header and source files (if any)
rm -f "$output_header" "$output_source"

# Create the C++ header file with an initial guard
echo "#ifndef FILES_DATA_H" >> "$output_header"
echo "#define FILES_DATA_H" >> "$output_header"

echo "#include <string> " >> "$output_header"

echo "namespace Nuake { " >> "$output_header"
echo "	namespace StaticResources { " >> "$output_header"

# Create the C++ source file
echo "#include \"StaticResources.h\"" >> "$output_source"

echo "namespace Nuake { " >> "$output_source"
echo "	namespace StaticResources { " >> "$output_source"
# Use find to search for files in the directory and its subdirectories
find "$input_directory" -type f -print0 | while IFS= read -r -d $'\0' file_path; do
    # Get the relative path of the file within the input_directory
    relative_path="${file_path/}"

    # Sanitize the relative path to make it suitable for C++ variable names
    sanitized_path="${relative_path//[^[:alnum:]_]/_}"

    # Generate C++ variable declarations for file path and size
    echo "extern const std::string ${sanitized_path}_path;" >> "$output_header"
    echo "extern unsigned int ${sanitized_path}_len;" >> "$output_header"
	echo "extern unsigned char ${sanitized_path}[];" >> "$output_header"
	
    # Append C++ code to the source file for storing file data
    echo -e "\n// Data for file: ${sanitized_path}_path" >> "$output_source"
    echo "const std::string ${sanitized_path}_path = R\"(${relative_path})\";" >> "$output_source"

    xxd -i "$file_path" | sed -e 's/^/    /' >> "$output_source"
done

echo "	}" >> "$output_source"
echo "}">> "$output_source"

echo "	}" >> "$output_header"
echo "}">> "$output_header"

# Close the header file guard
echo "#endif // FILES_DATA_H" >> "$output_header"

echo "Header file '$output_header' generated with variable declarations."
echo "Source file '$output_source' generated with file data."