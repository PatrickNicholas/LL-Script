#include "Interpret.h"
#include <algorithm> 

namespace ScriptCompile
{
	map<wstring, wstring> g_envir, empty;
	Program* g_program;
	int g_InFunction = 0;

	bool IsInt(const wstring& String)
	{
		const wchar_t* Buffer = String.c_str();
		wcstol(Buffer, const_cast<wchar_t**>(&Buffer), 10);
		return *Buffer == L'\0';
	}

	bool IsReal(const wstring& String)
	{
		const wchar_t* Buffer = String.c_str();
		wcstod(Buffer, const_cast<wchar_t**>(&Buffer));
		return *Buffer == L'\0';
	}

	bool IsStr(const wstring& String)
	{
		const wchar_t* Buffer = String.c_str();
		while (*Buffer != L'\0')
		{
			if (iswalpha(*Buffer))
				return true;
			Buffer++;
		}
		return false;
	}

	int ToInt(const wstring& String)
	{
		const wchar_t* Buffer = String.c_str();
		return wcstol(Buffer, const_cast<wchar_t**>(&Buffer), 10);
	}

	double ToReal(const wstring& String)
	{
		const wchar_t* Buffer = String.c_str();
		return wcstod(Buffer, const_cast<wchar_t**>(&Buffer));
	}

	wstring ToStr(int Value)
	{
		wchar_t Buffer[100];
		_itow(Value, Buffer, 10);
		return Buffer;
	}

	wstring ToStr(double Value)
	{
		char AnsiBuffer[100];
		wchar_t Buffer[100];
		sprintf(AnsiBuffer, "%lf", Value);
		mbstowcs(Buffer, AnsiBuffer, 100);
		return Buffer;
	}

	bool Condition(Variable& slove, map<wstring, wstring>& envir)
	{
		if (slove.kind == TK_STRING)
		{
			throw wstring(L"�������ʽ����Ϊ�ַ���");
		}

		if (slove.kind == TK_IDENTIFIER) 
		{
			wstring value = GetValue(slove.value, envir);

			if (IsStr(value))
			{
				throw wstring(L"�������ʽ����Ϊ�ַ���");
			}
			else if (IsReal(value))
			{
				slove.kind = TK_REAL;
				return ToReal(value);
			}
			else
			{
				slove.kind = TK_INTEGER;
				return ToInt(value);
			}
		}

		if (slove.kind == TK_REAL)
			return ToReal(slove.value);
		else
			return ToInt(slove.value);
	}

	inline Variable ConstructVariable(wstring& value)
	{
		if (IsStr(value))
			return{ TK_STRING, value };
		else if (IsReal(value))
			return{ TK_REAL, value };
		else
			return{ TK_INTEGER, value };
	}

	inline wstring& GetValue(wstring& name, map<wstring, wstring>& envir)
	{
		if (envir.find(name) != envir.end())
			return envir.find(name)->second;
		else
			return g_envir.find(name)->second;
	}

	Variable Caculate(Variable& left, Variable& right, int kind, OpFunction op)
	{
		double slove = op(ToReal(left.value), ToReal(right.value));
		wstring temp;
		if (kind == TK_REAL)
			temp = ToStr(slove);
		else
			temp = ToStr((int)slove);

		return{ kind, temp };
	}

	Variable Caculate(Variable left, Variable right, int op, map<wstring, wstring>& envir)
	{
		if (op == TK_EQUAL)
		{
			if (left.kind != TK_IDENTIFIER)
				throw wstring(L" = ��߱���Ϊ������");

			if (right.kind == TK_IDENTIFIER)
				right.value = GetValue(right.value, envir);

			return ConstructVariable(GetValue(left.value, envir) = right.value);
		}

		if (left.kind == TK_IDENTIFIER)
			left = ConstructVariable(GetValue(left.value, envir));
		if (right.kind == TK_IDENTIFIER)
			right = ConstructVariable(GetValue(right.value, envir));
		int kind = std::max(right.kind, left.kind);

		switch (op)
		{	
		case TK_ADD:
			{
				if (kind == TK_STRING)
					return{ TK_STRING, left.value + right.value };
				
				return Caculate(left, right, TK_REAL, [](double l, double r){
					return l + r;
				});
			}

		case TK_SUB:
			{
				if (kind == TK_STRING)
					throw wstring(L"�ַ���ֻ֧�� + ������");

				return Caculate(left, right, TK_REAL, [](double l, double r){
					return l - r;
				});
			}

		case TK_MUL:
			{
				if (kind == TK_STRING)
					throw wstring(L"�ַ���ֻ֧�� + ������");

				return Caculate(left, right, TK_REAL, [](double l, double r){
					return l * r;
				});
			}
		case TK_DIV:
			{
				if (kind == TK_STRING)
					throw wstring(L"�ַ���ֻ֧�� + ������");

				return Caculate(left, right, TK_REAL, [](double l, double r){
					if (r == 0.0) throw wstring(L"��������Ϊ��");
					return l / r;
				});
			}
		case TK_MOD:
			{
				if (kind == TK_STRING)
					throw wstring(L"�ַ���ֻ֧�� + ������");

				if (kind == TK_REAL)
					throw wstring(L"ȡģ����ֻ�������");

				if (ToInt(right.value))
					throw wstring(L"��������Ϊ��");

				return ConstructVariable(ToStr(ToInt(left.value) % ToInt(right.value)));
			}
		}
	}

	Variable EvalExpression(ASTreeExpression& expression, map<wstring, wstring>& envir)
	{
		switch (expression.GetBinary()->GetKind())
		{
		case AST_BINARY:
			{
				ASTreeBinary* binary = static_cast<ASTreeBinary*>(expression.GetBinary());
				ASTreeExpression binaryExpression;

				if (binary->GetOperator() == TK_OR)
				{
					binaryExpression.SetBinary(binary->GetLeft());
					Variable left = EvalExpression(binaryExpression, envir);
					bool open = Condition(left, envir);
					if (open)
					{
						return{ TK_REAL, L"1" };
					}
					else
					{
						binaryExpression.SetBinary(binary->GetRight());
						left = EvalExpression(binaryExpression, envir);
						open = Condition(left, envir);
						if (open)
							return{ TK_REAL, L"1" };
						else 
							return{ TK_REAL, L"0" };
					}
				}
				else if (binary->GetOperator() == TK_AND)
				{
					binaryExpression.SetBinary(binary->GetLeft());
					Variable left = EvalExpression(binaryExpression, envir);
					bool open = Condition(left, envir);
					if (open)
					{
						binaryExpression.SetBinary(binary->GetRight());
						left = EvalExpression(binaryExpression, envir);
						open = Condition(left, envir);
						if (open)
							return{ TK_REAL, L"1" };
						else
							return{ TK_REAL, L"0" };
					}
					else
					{
						return{ TK_REAL, L"0" };
					}
				}
				else
				{
					binaryExpression.SetBinary(binary->GetLeft());
					Variable left = EvalExpression(binaryExpression, envir);
					binaryExpression.SetBinary(binary->GetRight());
					Variable right = EvalExpression(binaryExpression, envir);

					return Caculate(left, right, binary->GetOperator(), envir);
				}
			}
			
		case AST_UNARY:
			{
				ASTreeUnary* unary = static_cast<ASTreeUnary*>(expression.GetBinary());
				throw wstring(L"��ʱ���ṩ��Ŀ����");
				break;
			}
			
		case AST_VAR:
			{
				ASTreeVar* var = static_cast<ASTreeVar*>(expression.GetBinary());

				if (var->GetValueKind() == TK_IDENTIFIER && 
					envir.find(var->GetValue()) == envir.end() && 
					g_envir.find(var->GetValue()) == g_envir.end())
				{
					throw wstring(L"û���ҵ���Ϊ��" + var->GetValue() + L"�ı���");
				}
				
				return{ var->GetValueKind(), var->GetValue() };
			}
			
		case AST_CALL:
			{
				ASTreeCall* call = static_cast<ASTreeCall*>(expression.GetBinary());
				// ��麯���Ƿ����
				if (!g_program->FindFunction(call->GetName()))
				{
					if (!g_program->FindPlugin(call->GetName()))
						throw wstring(L"û���ҵ���Ϊ��" + call->GetName() + L"�ĺ���");
					else
					{
						vector<wstring> params;
						// �������Ƿ����
						for (size_t i = 0; i < call->Size(); i++)
						{
							if (envir.find(call->GetParam(i)) == envir.end())
							{
								if (g_envir.find(call->GetParam(i)) == g_envir.end())
								{
									throw wstring(L"û���ҵ���Ϊ��" + call->GetParam(i) + L"�ı���");
								}
								else
								{
									params.push_back(g_envir.find(call->GetParam(i))->second);
								}
							}
							else
							{
								params.push_back(envir.find(call->GetParam(i))->second);
							}
						}
						return g_program->GetPlugin(call->GetName()).Call(params);
					}
				}
					

				Function* function = g_program->GetFunction(call->GetName());
				if (function->GetParamLength() != call->Size())
					throw wstring(L"���ú��� " + call->GetName() + L" ʱ������ƥ��");

				map<wstring, wstring> temp;
				set<wstring>& identifier = function->GetIdentifier();
				for (const auto i : identifier)
					temp[i] = L"";

				// �������Ƿ����
				for (size_t i = 0; i < call->Size(); i++)
				{
					if (envir.find(call->GetParam(i)) == envir.end())
					{
						if (g_envir.find(call->GetParam(i)) == g_envir.end())
						{
							throw wstring(L"û���ҵ���Ϊ��" + call->GetParam(i) + L"�ı���");
						}
						else
						{
							temp[function->GetParamName(i)] = g_envir.find(call->GetParam(i))->second;
						}
					}
					else
					{
						temp[function->GetParamName(i)] = envir.find(call->GetParam(i))->second;
					}
				}

				return RunFunction(function, temp);
			}
		case AST_EXPR:
			{
				// �������
				ASTreeExpression* expr = static_cast<ASTreeExpression*>(expression.GetBinary());
				return EvalExpression(*expr, envir);
			}
			
		default:
			throw wstring(L"��Ҳ��֪����ʲô");
		}
	}

	ReturnValue RunStatements(ASTreeStatements& statements, map<wstring, wstring>& envir)
	{
		for (size_t i = 0; i < statements.GetSize(); i++)
		{
			ASTree* tree = statements.GetStatement(i);
			switch (tree->GetKind())
			{
			case AST_IF:
				{
					ASTreeIf* astIf = static_cast<ASTreeIf*>(tree);
					Variable slove = EvalExpression(astIf->GetCondition(), envir);
					bool open = Condition(slove, envir);

					if (open)
					{
						ReturnValue var = RunStatements(astIf->GetIfStatements(), envir);
						if (var.kind == TK_RETURN || var.kind == TK_BREAK)
							return var;
					}
					else if (astIf->GetHasElse())
					{
						ReturnValue var = RunStatements(astIf->GetElseStatements(), envir);
						if (var.kind == TK_RETURN || var.kind == TK_BREAK)
							return var;
					}
					break;
				}

			case AST_WHILE:
				{
					ASTreeWhile* astWhile = static_cast<ASTreeWhile*>(tree);
					Variable slove = EvalExpression(astWhile->GetCondition(), envir);
					while (Condition(slove, envir))
					{
						ReturnValue var = RunStatements(astWhile->GetStatements(), envir);
						if (var.kind == TK_BREAK)
							break;
						else if (var.kind == TK_RETURN)
							return var;

						slove = EvalExpression(astWhile->GetCondition(), envir);
					}
					break;
				}

			case AST_BREAK:
				{
					ReturnValue value = { TK_BREAK, { TK_BREAK, L"break" } };
					return value;
				}

			case AST_RETURN:
				{
					if (g_InFunction == 0)
						throw wstring(L"��ͼ�ں�����ʹ�� return");
				
					ASTreeReturn* astReturn = static_cast<ASTreeReturn*>(tree);

					// ��EvalExpression���صı���ת���ɶ�Ӧ��ֵ����
					Variable var = EvalExpression(astReturn->GetExpression(), envir);
					if (var.kind == TK_IDENTIFIER)
					{
						var.value = GetValue(var.value, envir);
						var = ConstructVariable(var.value);
					}
					return{ TK_RETURN, var };
				}

			case AST_EXPR:
				{
					ASTreeExpression* expression = static_cast<ASTreeExpression*>(tree);
					EvalExpression(*expression, envir);
					break;
				}

			default:
				break;
			}
		}
		return{ TK_END, { TK_END, L"end" } };
	}

	Variable RunFunction(Function* function, map<wstring, wstring>& envir)
	{
		++g_InFunction;
		ReturnValue var = RunStatements(function->GetStatements(), envir);
		if (var.kind == TK_BREAK)
			throw wstring(L"break û���ҵ���Ӧ�� while ѭ��");
		--g_InFunction;
		return var.var;
	}

	void Run(Program* pro)
	{
		g_program = pro;

		set<wstring>& identifier = g_program->GetIdentifier();
		for (auto i : identifier)
		{
			if (!g_program->FindFunction(i))
				g_envir[i] = L"";
		}
			
		ASTreeStatements& states = g_program->GetStatements();
		ReturnValue var = RunStatements(states, empty);
		if (var.kind == TK_BREAK)
			throw wstring(L"break û���ҵ���Ӧ�� while ѭ��");
	}
}