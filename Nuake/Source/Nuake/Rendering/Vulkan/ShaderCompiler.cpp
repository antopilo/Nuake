#include "ShaderCompiler.h"

#include "VulkanShader.h"

#include <atlbase.h>
#include "dxcapi.h"
#include <stdexcept>
#include <string>
#include <codecvt>
#include <locale>
#include <vector>
#include "Nuake/Core/Logger.h"


using namespace Nuake;

Ref<VulkanShader> ShaderCompiler::CompileShader(const std::string& path)
{
    HRESULT hres;

    // Initialize DXC library
    CComPtr<IDxcLibrary> library;
    hres = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
    if (FAILED(hres)) 
    {
        throw std::runtime_error("Could not init DXC Library");
    }

    // Initialize DXC compiler
    CComPtr<IDxcCompiler3> compiler;
    hres = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
    if (FAILED(hres)) 
    {
        throw std::runtime_error("Could not init DXC Compiler");
    }

    // Initialize DXC utility
    CComPtr<IDxcUtils> utils;
    hres = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
    if (FAILED(hres)) 
    {
        throw std::runtime_error("Could not init DXC Utility");
    }

    // Convert std::string to std::wstring
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wpath = converter.from_bytes(path);

    // Load the HLSL text shader from disk
    uint32_t codePage = DXC_CP_ACP;
    CComPtr<IDxcBlobEncoding> sourceBlob;
    hres = utils->LoadFile(wpath.c_str(), &codePage, &sourceBlob);
    if (FAILED(hres)) 
    {
        throw std::runtime_error("Could not load shader file");
    }

    // Select target profile based on shader file extension
    ShaderType shaderType;
    LPCWSTR targetProfile{};
    size_t idx = path.rfind('.');
    if (idx != std::string::npos) 
    {
        std::wstring extension = wpath.substr(idx + 1);
        if (extension == L"vert") 
        {
            targetProfile = L"vs_6_1";
            shaderType = ShaderType::Vertex;
        }
        else if (extension == L"frag") 
        {
            targetProfile = L"ps_6_1";
            shaderType = ShaderType::Fragment;
        }
        else if (extension == L"comp")
        {
            targetProfile = L"cs_6_1";
            shaderType = ShaderType::Compute;
        }
    }

    // Configure the compiler arguments for compiling the HLSL shader to SPIR-V
    std::vector<LPCWSTR> arguments = {
        // (Optional) name of the shader file to be displayed e.g. in an error message
        wpath.c_str(),
        // Shader main entry point
        L"-E", L"main",
        // Shader target profile
        L"-T", targetProfile,
        // Compile to SPIRV
        L"-spirv"
    };

    // Compile shader
    DxcBuffer buffer{};
    buffer.Encoding = DXC_CP_ACP;
    buffer.Ptr = sourceBlob->GetBufferPointer();
    buffer.Size = sourceBlob->GetBufferSize();

    CComPtr<IDxcResult> result{ nullptr };
    hres = compiler->Compile(
        &buffer,
        arguments.data(),
        (uint32_t)arguments.size(),
        nullptr,
        IID_PPV_ARGS(&result)
    );

    if (SUCCEEDED(hres)) 
    {
        result->GetStatus(&hres);
    }

    // Output error if compilation failed
    if (FAILED(hres) && (result)) 
    {
        CComPtr<IDxcBlobEncoding> errorBlob;     
        hres = result->GetErrorBuffer(&errorBlob);            
        if (SUCCEEDED(hres) && errorBlob) 
        {
            CComPtr<IDxcBlobEncoding> errorBlobUtf8;
             
            library->GetBlobAsUtf8(errorBlob, &errorBlobUtf8); 
             
            const char* errorMsg = reinterpret_cast<const char*>(errorBlobUtf8->GetBufferPointer());
            std::string errorMsgStr(errorMsg, errorBlobUtf8->GetBufferSize());
              
            Logger::Log("Shader compilation failed: " + errorMsgStr, "DXC", CRITICAL);         
              
            throw std::runtime_error("Shader compilation failed: " + errorMsgStr);   
        }     
        else
        { 
            throw std::runtime_error("Shader compilation failed, but no error message was retrieved.");  
        }
    } 
     
    // Get compilation result
    CComPtr<IDxcBlob> code;  
    result->GetResult(&code);
	
	Ref<VulkanShader> shader = CreateRef<VulkanShader>(        
        static_cast<uint32_t*>(code->GetBufferPointer()),     
        static_cast<uint32_t>(code->GetBufferSize()), shaderType);     

    shader->SetSourcePath(path);
    return shader;    
} 
  