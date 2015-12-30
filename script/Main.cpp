#include <iostream>
#include <fstream>
#include <string>
#include <conio.h>
#include <windows.h>

#include "Parser/lexer.h"
#include "Syntax/syntax.h"
#include "Interpret/Interpret.h"

using namespace std;

namespace ScriptCompile
{
	class ScriptPlugin : public Plugin
	{
	public:
		using Callback = Variable(ScriptPlugin::*)(vector<wstring>& params);

		ScriptPlugin() 
		{
			name.insert(L"print");
			name.insert(L"input");
			pluginName[L"print"] = -1;
			pluginName[L"input"] = 0;
			pluginFunction[L"print"] = &ScriptPlugin::Print; 
			pluginFunction[L"input"] = &ScriptPlugin::Input; 
		}

		Variable Call(wstring& name, vector<wstring>& params)
		{
			if (pluginName.find(name) == pluginName.end())
			{
				throw wstring(L"δ���庯�� " + name);
			}

			if (pluginName.find(name)->second != -1 &&
				pluginName.find(name)->second != params.size())
				throw wstring(L"���� " + name + L"������ƥ��");

			return (this->*(pluginFunction.find(name)->second))(params);
		}

		Variable Print(vector<wstring>& params)
		{
			for (auto i : params)
			{
				wcout << i;
			}
			wcout << endl;

			return{ ScriptCompile::TK_END, L"END" };
		}

		Variable Input(vector<wstring>& params)
		{
			wstring temp = L"20";
			wcin >> temp;

			return ConstructVariable(temp);
		}

	private:
		unordered_map<wstring, Callback> pluginFunction;
	};
}


int wmain(int argc, wchar_t* argv[])
{
	std::locale::global(std::locale(""));
	wcout.imbue(locale(""));

	if (argc < 2)
	{
		wcout << L"Use: Script.exe input.txt" << endl;
		_getch();
		return 0;
	}

	wstring Code;
	{
		FILE* f;
		if (_wfopen_s(&f, argv[1], L"rb") == EINVAL)
		{
			wcout << L"�򲻿��ļ�" << argv[1] << endl;
			_getch();
			return 0;
		}

	/*wstring Code;
	{
		FILE* f;
		if (_wfopen_s(&f, L"TEST.TXT", L"rb") == EINVAL)
		{
			wcout << L"�򲻿��ļ�" << L"TEST.TXT" << endl;
			_getch();
			return 0;
		}

		fpos_t fsize;
		size_t size;
		fseek(f, 0, SEEK_END);
		fgetpos(f, &fsize);
		size = (size_t)fsize;
		fseek(f, 0, SEEK_SET);
		char* AnsiBuffer = new char[size + 1];
		fread(AnsiBuffer, 1, size, f);
		AnsiBuffer[size] = '\0';
		fclose(f);

		size_t wsize = mbstowcs(0, AnsiBuffer, size);
		wchar_t* Buffer = new wchar_t[wsize + 1];
		mbstowcs(Buffer, AnsiBuffer, size);
		Buffer[size] = L'\0';
		Code = Buffer;

		delete[] Buffer;
		delete[] AnsiBuffer;*/
	}

	ScriptCompile::Program program;
	ScriptCompile::ScriptPlugin plugin;
	program.AddPlugin(&plugin);

	try
	{
		auto beginTime = GetTickCount();
		ScriptCompile::ParserASTreeUnit(Code, program);
		auto endTime = GetTickCount();

		wcout << L"���뻨ʱ��" << double(endTime - beginTime) / 1000 << endl;
	}
	catch (ScriptCompile::ASTError e)
	{
		wcout << L"Line:" << e.GetToken().line << L" ������Ϣ��" << e.GetMsg() << endl;
		_getch();
		return 0;
	}

	try
	{
		auto beginTime = GetTickCount();
		ScriptCompile::Run(&program);
		auto endTime = GetTickCount();
		wcout << L"���л�ʱ��" << double(endTime - beginTime) / 1000 << endl;
	}
	catch (wstring e)
	{
		wcout << L"������Ϣ��" << e << endl;
	}
	
	_getch();
	return 0;
}