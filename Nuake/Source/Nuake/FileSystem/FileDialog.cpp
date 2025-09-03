#include "FileDialog.h"

#include "Engine.h"

#include <GLFW/glfw3.h>

// Platform specific stuff
// Windows
#ifdef NK_WIN
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#include <commdlg.h>
#include <ShlObj.h>
#endif

// Linux
#ifdef NK_LINUX
#include "gtk/gtk.h"
#endif


using namespace Nuake;

std::string FileDialog::OpenFile(const std::string_view& filter)
{
	std::string filePath;
#ifdef NK_WIN
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = glfwGetWin32Window(Engine::GetCurrentWindow()->GetHandle());
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter.data();
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetOpenFileNameA(&ofn) == TRUE)
	{
		filePath = std::string(ofn.lpstrFile);
	}

#endif

#ifdef NK_LINUX
	GtkWidget *dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	gint res;

	dialog = gtk_file_chooser_dialog_new("Open File",
		NULL,
		action,
		"_Cancel",
		GTK_RESPONSE_CANCEL,
		"_Open",
		GTK_RESPONSE_ACCEPT,
		NULL);

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	if (filter) {
		GtkFileFilter *file_filter = gtk_file_filter_new();
		gtk_file_filter_set_name(file_filter, "Filter Name");
		gtk_file_filter_add_pattern(file_filter, filter);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), file_filter);
	}

	res = gtk_dialog_run(GTK_DIALOG(dialog));

	if (res == GTK_RESPONSE_ACCEPT) {
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		filePath = filename;
		g_free(filename);
	}

	gtk_widget_destroy(dialog);
#endif
	return filePath;
}

std::string FileDialog::SaveFile(const std::string_view& filter)
{
#ifdef NK_WIN
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = glfwGetWin32Window(Engine::GetCurrentWindow()->GetHandle());
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter.data();
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;
	if (GetSaveFileNameA(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}
#endif

#ifdef NK_LINUX
	GtkWidget *dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
	gint res;

	gtk_init(NULL, NULL);

	dialog = gtk_file_chooser_dialog_new("Save File",
										 NULL,
										 action,
										 "_Cancel",
										 GTK_RESPONSE_CANCEL,
										 "_Save",
										 GTK_RESPONSE_ACCEPT,
										 NULL);

	GtkFileFilter *file_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(file_filter, filter);
	gtk_file_filter_add_pattern(file_filter, "*.*"); // You can customize this pattern
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), file_filter);

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
		filename = gtk_file_chooser_get_filename(chooser);
		std::string result(filename);
		g_free(filename);
		gtk_widget_destroy(dialog);
		return result;
	}
	else
	{
		gtk_widget_destroy(dialog);
		return std::string();
	}
#endif

	return std::string();
}

std::string FileDialog::OpenFolder()
{
	std::string folderPath;

#ifdef NK_WIN
	// Initialize COM library for the current thread
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog* pFileOpen = NULL;

		// Create the FileOpenDialog object
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, (void**)&pFileOpen);

		if (SUCCEEDED(hr))
		{
			// Set the options for the dialog to pick folders
			DWORD dwOptions;
			hr = pFileOpen->GetOptions(&dwOptions);
			if (SUCCEEDED(hr))
			{
				hr = pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS); // Use FOS_PICKFOLDERS to select folders
			}

			// Show the dialog to the user
			hr = pFileOpen->Show(NULL); // NULL here means no parent window

			// If the user selects a folder
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);

				if (SUCCEEDED(hr))
				{
					// Get the selected folder's path
					PWSTR pszFolderPath = NULL;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);

					if (SUCCEEDED(hr))
					{
						// Convert from wide string (PWSTR) to std::string
						char szFolderPath[MAX_PATH];
						wcstombs(szFolderPath, pszFolderPath, MAX_PATH);
						folderPath = std::string(szFolderPath);

						// Free memory
						CoTaskMemFree(pszFolderPath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}

		// Uninitialize COM library
		CoUninitialize();
	}
#endif
	return folderPath;
}