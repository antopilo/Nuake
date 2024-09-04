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
	BROWSEINFOA bi;
	CHAR szFolder[260] = { 0 };
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.lpszTitle = "Select a Folder";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.hwndOwner = glfwGetWin32Window(Engine::GetCurrentWindow()->GetHandle());
	bi.pszDisplayName = szFolder;
	LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
	if (pidl != NULL)
	{
		SHGetPathFromIDListA(pidl, szFolder);
		folderPath = std::string(szFolder);
		CoTaskMemFree(pidl);
	}
#endif

#ifdef NK_LINUX
	GtkWidget* dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	gint res;

	dialog = gtk_file_chooser_dialog_new("Select Folder",
		NULL,
		action,
		"_Cancel",
		GTK_RESPONSE_CANCEL,
		"_Select",
		GTK_RESPONSE_ACCEPT,
		NULL);

	res = gtk_dialog_run(GTK_DIALOG(dialog));

	if (res == GTK_RESPONSE_ACCEPT) {
		char* foldername = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		folderPath = foldername;
		g_free(foldername);
	}

	gtk_widget_destroy(dialog);
#endif

	return folderPath;
}