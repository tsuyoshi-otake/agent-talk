// AgentTalk.cpp : ���C�� �v���W�F�N�g �t�@�C���ł��B

#include "stdafx.h"
#include "MainForm.h"
#include "SettingsForm.h"
#include "IniFile.h"

using namespace System;
using namespace System::Windows::Forms;
using namespace AgentTalk;
using namespace System::Security::Principal;
using namespace System::Diagnostics;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
    // Check if the application is not running with admin rights
    WindowsIdentity^ identity = WindowsIdentity::GetCurrent();
    WindowsPrincipal^ principal = gcnew WindowsPrincipal(identity);
    bool isAdmin = principal->IsInRole(WindowsBuiltInRole::Administrator);

    if (!isAdmin)
    {
        // Restart the application with admin rights
        ProcessStartInfo^ startInfo = gcnew ProcessStartInfo();
        startInfo->UseShellExecute = true;
        startInfo->WorkingDirectory = Environment::CurrentDirectory;
        startInfo->FileName = Application::ExecutablePath;
        startInfo->Verb = "runas";
        try
        {
            Process::Start(startInfo);
            Application::Exit();
            return 0;
        }
        catch(...)
        {
            // The user refused to allow privileges elevation.
            // Do nothing and continue to run the application normally.
        }
    }

    // �R���g���[�����쐬�����O�ɁAWindows XP �r�W���A�����ʂ�L���ɂ��܂�
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    IniFile^ ini = gcnew IniFile();
    System::Collections::Specialized::NameValueCollection^ values = ini->ReadAllValues("agenttalk.ini");

    System::String^ english = values["english"];
    System::String^ host = values["host"];
    System::String^ endpoint = values["endpoint"];
    System::String^ apikey = values["apikey"];
    System::String^ model = values["model"];

    if (String::IsNullOrEmpty(english) || String::IsNullOrEmpty(host) ||
        String::IsNullOrEmpty(endpoint) || String::IsNullOrEmpty(apikey) || 
        String::IsNullOrEmpty(model)) 
    {
        // If any value is null or empty, run the SettingsForm
        Application::Run(gcnew SettingsForm(values));
    }
    else
    {
        // ���C�� �E�B���h�E���쐬���āA���s���܂�
        Application::Run(gcnew MainForm());
    }
    
    return 0;
}