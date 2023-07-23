#pragma once
#include <string>

class PopupHelper
{
public:
    static void OpenPopup(const std::string& id);
    static bool DefineConfirmationDialog(const std::string& id, const std::string& text);
    static bool PopupHelper::DefineTextDialog(const std::string& id, std::string& currentText);
};
