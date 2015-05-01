#include "Program.h"

namespace ScriptCompile
{
	void Program::AddStatements(ASTreeStatements* s)
	{
		// ��Ϊ��ҪΪstatements����ռ�
		if (statements == nullptr)
			statements = s;
		else
			statements->InsertStatement(*s);
	}

	void Program::AddFunction(Function* f)
	{
		functions[f->GetFunctionName()] = f;
		identifier.insert(f->GetFunctionName());
	}

	void Program::AddPlugin(Plugin* p)
	{
		auto name = p->GetName();

		for (auto i : name)
			identifier.insert(i);

		plugin = p;
	}

	void Program::Check(BaseProgram* base)
	{
		for (auto i : base->GetIdentifierUse())
		{
			if (base->GetVariable().find(i) == base->GetVariable().end())
				throw ASTError({ TK_ERROR, 0, L"" }, L"δ����ı�ʶ����" + i);
		}

		for (auto i : base->GetFuntionUse())
		{
			if (FindFunction(i.first) && functions[i.first]->GetParams().size() == i.second)
				continue;
			else if (plugin->GetName().find(i.first) != plugin->GetName().end())
				continue;
			else
			{
				wstring temp = L"���ú�����" + i.first;
				temp += L"ʱ����";
				throw ASTError({ TK_ERROR, 0, L"" }, temp);
			}
				
		}
	}

	void Program::CheckAll()
	{
		Check(this);

		for (auto i : functions)
		{
			Check(i.second);
		}
	}
}