#pragma once
#include <string>

class PopupHelper
{
public:
    static void Confirmation(const std::string& id);
    static bool DefineDialog(const std::string& id, const std::string& text);
};
