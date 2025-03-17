// IniFile.h

#pragma once

using namespace System;
using namespace System::Collections::Specialized;
using namespace System::IO;

public ref class IniFile
{
public:
    NameValueCollection^ ReadAllValues(String^ iniPath)
    {
        NameValueCollection^ values = gcnew NameValueCollection();
        StreamReader^ sr = gcnew StreamReader(iniPath);
        String^ line;

        try
        {
            while(line = sr->ReadLine())
            {
                array<String^>^ parts = line->Split('=');
                if(parts->Length == 2)
                {
                    values->Add(parts[0], parts[1]);
                }
            }
        }
        finally
        {
            if(sr)
                delete (IDisposable^)sr;
        }

        return values;
    }
};
