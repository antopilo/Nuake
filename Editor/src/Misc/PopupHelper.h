#pragma once
#include <string>

class PopupHelper
{
public:
    static void Confirmation(const std::string& id);
    static bool DefineDialog(std::string& id, std::string& text);
};
