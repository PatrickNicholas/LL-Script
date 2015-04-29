#include "lexer.h"
#include "syntax.h"

namespace ScriptCompile
{
	//int Variables::Lookup(wstring& name)
	//{
	//	for (auto i = members.begin(); i != members.end(); ++i)
	//	{
	//		if (i->name == name)
	//			return i - members.begin();
	//	}

	//	return -1;
	//}

	//bool Variables::Insert(Variable& v)
	//{
	//	if (Lookup(v.name) != -1) return false;

	//	members.push_back(v);
	//	return true;
	//}

	//bool Variables::SetValue(wstring& name, wstring& value)
	//{
	//	int pos = Lookup(name);
	//	if (pos == -1) return false;

	//	members.at(pos).value = value;
	//	return true;
	//}

	//Variable& Variables::GetValue(unsigned int pos)
	//{
	//	return members.at(pos);
	//}

	////////////////////////////////////////////////////////////////////////////////

	MemoryManager<ASTree> g_astManager;
	MemoryManager<Function> g_funcManager;
	Token	g_token = { TK_NONE, 0, L"NONE" };

	////////////////////////////////////////////////////////////////////////////////

	template<class AIM, class T>
	T* NewMemory(MemoryManager<T>& manager)
	{
		AIM* temp = new AIM;
		manager.Insert(temp);

		return temp;
	}

	inline void Except(int k, wchar_t s)
	{
		if (g_token.kind != k)
		{
			wstring msg(L"��Ԥ�ڵģ�");
			msg.append(1, s);
			throw ASTError(g_token, msg);
		}
	}

	void Check(BaseProgram* base, Program *program)
	{
		for (auto i : base->GetIdentifierUse())
		{
			if (base->GetVariable().find(i) == base->GetVariable().end())
				throw ASTError(g_token, L"δ����ı�ʶ����" + i);
		}

		for (auto i : base->GetFuntionUse())
		{
			if (program->FindFunction(i.first) &&
				program->GetFunction(i.first)->GetParams().size() == i.second)
				continue;
			else if (program->GetPlugin().GetName().find(i.first)
				!= program->GetPlugin().GetName().end())
				continue;
			else
				throw ASTError(g_token, L"���ú�����" + i.first + L"����");
		}
	}
	////////////////////////////////////////////////////////////////////////////////

	ASTreeExpression* Expression(Lexer& lexer, BaseProgram* base)
	{
		ASTreeExpression* expression = static_cast<ASTreeExpression*>(
			NewMemory<ASTreeExpression>(g_astManager)
			);

		map<int, Operator> sign = {
			{ TK_EQUAL, { TK_EQUAL, 0, 2 } },
			{ TK_VERTICAL, { TK_VERTICAL, 1, 2 } },
			{ TK_AND, { TK_AND, 2, 2 } },
			{ TK_PLUS, { TK_PLUS, 3, 2 } },
			{ TK_MINUS, { TK_MINUS, 3, 2 } },
			{ TK_STAR, { TK_STAR, 4, 2 } },
			{ TK_SLASH, { TK_SLASH, 4, 2 } },
			{ TK_PERCENT, { TK_PERCENT, 4, 2 } },
			{ TK_NOT, { TK_NOT, 5, 1 } },
		};

		queue<ASTree*> origin;
		bool flag = false;
		do {
			switch (g_token.kind)
			{
			case TK_IDENTIFIER:
				{
					wstring name = g_token.value;
					
					// ���ﲢû�м���ʶ���Ƿ��Ѿ����壬ԭ������ϣ��������������������λ��������
					// ��������ʹ��
					g_token = lexer.GetNextToken();
					if (g_token.kind == TK_LBRA)
					{
						// ��������
						ASTreeCall* call = static_cast<ASTreeCall*>(
							NewMemory<ASTreeCall>(g_astManager)
							);
						call->SetName(name);
						
						int index = 0;
						g_token = lexer.GetNextToken();
						if (g_token.kind != TK_RBRA) 
						{
							do 
							{
								if (g_token.kind == TK_IDENTIFIER)
								{
									call->AddParam(g_token.value);
									g_token = lexer.GetNextToken();
									++index;
								}
								else
								{
									throw ASTError(g_token, L"ʵ��ֻ��Ϊ����");
								}
							} while (g_token.kind == TK_COMMA);
						}
						base->AddFuntionUse(name, index);
						Except(TK_RBRA, L')');
						origin.push(call);
						call = nullptr;
						g_token = lexer.GetNextToken();
						break;
					}
					else
					{
						ASTreeVariable* identifier = static_cast<ASTreeVariable*>(
							NewMemory<ASTreeVariable>(g_astManager)
							);
						base->AddIdentifierUse(name);
						identifier->SetValueKind(TK_IDENTIFIER);
						identifier->SetValue(name);
						origin.push(identifier);
						identifier = nullptr;
						continue;
					}
				}

			case TK_STRING: case TK_INTEGER: case TK_REAL:
				{
					ASTreeVariable* var = static_cast<ASTreeVariable*>(
						NewMemory<ASTreeVariable>(g_astManager)
						);
					var->SetValueKind(g_token.kind);
					var->SetValue(g_token.value);
					origin.push(var);
					var = nullptr;
					g_token = lexer.GetNextToken();
					break;
				}

			case TK_VERTICAL: case TK_AND: case TK_PLUS: case TK_MINUS:
			case TK_STAR: case TK_SLASH: case TK_PERCENT: case TK_NOT:
			case TK_LBRA: case TK_EQUAL: case TK_RBRA:
				{
					ASTreeVariable* sign = static_cast<ASTreeVariable*>(
						NewMemory<ASTreeVariable>(g_astManager)
						);
					sign->SetValueKind(g_token.kind);
					origin.push(sign);
					sign = nullptr;
					g_token = lexer.GetNextToken();
					break;
				}

			/*case TK_END:
				flag = true;
				break;
*/
			default:
				// throw ASTError(g_token, L"���ʽ���Ϸ�");
				flag = true;
				break;
			}
		} while (!flag);

		queue<ASTree*> tempStack;
		stack<ASTree*> opStack;

		while (origin.size())
		{
			ASTree* ast = origin.front();
			origin.pop();

			if (ast->GetKind() == AST_VALUE)
			{
				ASTreeVariable& astvar = static_cast<ASTreeVariable&>(*ast);
				switch (astvar.GetValueKind())
				{
				case TK_STRING: case TK_INTEGER: case TK_REAL: case TK_IDENTIFIER:
					tempStack.push(ast);
					break;

				case TK_LBRA:
					opStack.push(ast);
					break;

				case TK_RBRA:
					{
						flag = false;
						while (opStack.size() != 0)
						{
							ASTreeVariable* opvar = static_cast<ASTreeVariable*>(
								opStack.top()
								);
							opStack.pop();
							if (opvar->GetValueKind() == TK_LBRA)
							{
								flag = true;
								break;
							}
							tempStack.push(opvar);
						}

						if (!flag) 
							throw ASTError(g_token, L"���ʽ���Ų�ƥ��");

						break;
					}

				case TK_VERTICAL: case TK_AND: case TK_PLUS: case TK_MINUS: case TK_STAR:
				case TK_SLASH: case TK_PERCENT: case TK_NOT: case TK_EQUAL:
					{
						flag = false;
						while (opStack.size() != 0)
						{
							ASTreeVariable* opvar = static_cast<ASTreeVariable*>(
								opStack.top()
								);
							
							if (sign[opvar->GetValueKind()].lever 
								< sign[astvar.GetValueKind()].lever)
							{
								opStack.push(ast);
								flag = true;
								break;
							}
							else
							{
								tempStack.push(opvar);
								opStack.pop();
							}
						}

						if (!flag)
							opStack.push(ast);

						break;
					}
				}
			}
			else
			{
				tempStack.push(ast);
			}
		}

		while (opStack.size())
		{
			tempStack.push(opStack.top());
			opStack.pop();
		}

		// ��ʱ��tempStackӦ�����沨������

		stack<ASTree*> sloveStack;

		while (tempStack.size())
		{
			ASTree* ast = tempStack.front();
			tempStack.pop();

			if (ast->GetKind() == AST_VALUE)
			{
				ASTreeVariable& astvar = static_cast<ASTreeVariable&>(*ast);
				switch (astvar.GetValueKind())
				{
				case TK_STRING: case TK_INTEGER: case TK_REAL: case TK_IDENTIFIER:
					sloveStack.push(ast);
					break;

				case TK_NOT:
					{
						ASTreeUnary* unary = static_cast<ASTreeUnary*>(
							NewMemory<ASTreeUnary>(g_astManager)
							);
						
						if (!sloveStack.size())
							throw ASTError(g_token, L"������ʽ���󣬵�Ŀ�������û���ҵ�����");

						unary->SetOperator(TK_NOT);
						unary->SetExprssion(sloveStack.top());
						sloveStack.pop();
						sloveStack.push(unary);
						break;
					}

				case TK_VERTICAL: case TK_AND: case TK_PLUS: case TK_MINUS: 
				case TK_STAR: case TK_SLASH: case TK_PERCENT: case TK_EQUAL:
					{
						ASTreeBinary* binary = static_cast<ASTreeBinary*>(
							NewMemory<ASTreeBinary>(g_astManager)
							);
						
						if (sloveStack.size() < 2)
							throw ASTError(g_token, L"������ʽ����˫Ŀ�����ȱ���ҵ�����");

						// �˴�������ԭ�ȵ�˳��
						binary->SetOperator(astvar.GetValueKind());
						binary->SetRight(sloveStack.top());
						sloveStack.pop();
						binary->SetLeft(sloveStack.top());
						sloveStack.pop();
						sloveStack.push(binary);
						break;
					}
				}
			}
			else
			{
				sloveStack.push(ast);
			}
		}

		// ����˵����SloveStatck��Ӧ��ֻʣ��һ��Ԫ���ˣ�������ǣ�˵��������
		if (sloveStack.size() != 1)
			throw ASTError(g_token, L"�������ʽ������");

		expression->SetBinary(sloveStack.top());
		return expression;
	}

	// 
	// BreakOrReturnStatements 
	//		"return" [experssion] ";" | "break" ";"
	//
	ASTreeReturnStatement* BreakOrReturnStatements(Lexer& lexer, BaseProgram* base)
	{
		if (g_token.kind == TK_BREAK)
		{
			g_token = lexer.GetNextToken();
			Except(TK_END, L';');
			g_token = lexer.GetNextToken();
			return static_cast<ASTreeReturnStatement*>(
				NewMemory<ASTreeBreakStatement>(g_astManager)
				);
		}
		else
		{
			ASTreeReturnStatement* astree = static_cast<ASTreeReturnStatement*>(
				NewMemory<ASTreeReturnStatement>(g_astManager)
				);

			g_token = lexer.GetNextToken();
			astree->SetExprssion(Expression(lexer, base));
			Except(TK_END, L';');
			g_token = lexer.GetNextToken();

			return astree;
		}
	}

	//
	// WhileStatements
	//		"while" "(" expression ")" 
	//		"{" [statements] "}"
	//
	ASTreeWhileStatement* WhileStatements(Lexer& lexer, BaseProgram* base)
	{
		ASTreeWhileStatement* astree = static_cast<ASTreeWhileStatement*>(
			NewMemory<ASTreeWhileStatement>(g_astManager)
			);

		lexer.TakeNotes();
		g_token = lexer.GetNextToken();
		Except(TK_LBRA, L'(');
		lexer.Restore();

		astree->SetCondition(Expression(lexer, base));

		Except(TK_LBRACE, L'{');
		g_token = lexer.GetNextToken();

		astree->SetStatements(Statements(lexer, base));

		Except(TK_RBRACE, L'}');
		g_token = lexer.GetNextToken();

		return astree;
	}

	//
	// IfStatements
	//		"if" "(" expression ")"
	//		"{" [statemens] "}"
	//		["else" "{" [statements] "}" ]
	//
	ASTreeIfStatement* IfStatements(Lexer& lexer, BaseProgram* base)
	{
		ASTreeIfStatement* astree = static_cast<ASTreeIfStatement*>(
			NewMemory<ASTreeIfStatement>(g_astManager)
			);

		lexer.TakeNotes();
		g_token = lexer.GetNextToken();
		Except(TK_LBRA, L'(');
		lexer.Restore();

		astree->SetCondition(Expression(lexer, base));

		Except(TK_LBRACE, L'{');
		g_token = lexer.GetNextToken();

		astree->SetIfStatements(Statements(lexer, base));
		
		Except(TK_RBRACE, L'}');
		g_token = lexer.GetNextToken();
		if (g_token.kind == TK_ELSE)
		{
			g_token = lexer.GetNextToken();
			Except(TK_LBRACE, L'{');
			g_token = lexer.GetNextToken();

			astree->SetElseStatements(Statements(lexer, base));

			Except(TK_RBRACE, L'}');
			g_token = lexer.GetNextToken();
		}

		return astree;
	}

	//
	// VariableDefination
	//		"var" identifier [ "=" expression ] ";"  
	//
	ASTreeExpression* VariableDefination(Lexer& lexer, BaseProgram* base)
	{
		if (g_token.kind != TK_IDENTIFIER)
		{
			throw ASTError(g_token, L" var ��Ӧ�ý�����Ҫ�����ı�ʶ��");
		}
		else if (base->GetIdentifier().find(g_token.value) 
			!= base->GetIdentifier().end())
		{
			throw ASTError(g_token, L"��ʶ����" + g_token.value + L"�ض���");
		}

		wstring name = g_token.value;
		base->AddVariable(name);

		g_token = lexer.GetNextToken();
		if (g_token.kind == TK_EQUAL)
		{
			ASTreeExpression* expression = static_cast<ASTreeExpression*>(
				NewMemory<ASTreeExpression>(g_astManager)
				);
			ASTreeBinary* binary = static_cast<ASTreeBinary*>(
				NewMemory<ASTreeBinary>(g_astManager)
				);
			ASTreeVariable* var = static_cast<ASTreeVariable*>(
				NewMemory<ASTreeVariable>(g_astManager)
				);

			var->SetValueKind(TK_IDENTIFIER);
			var->SetValue(name);
			binary->SetOperator(TK_EQUAL);
			binary->SetLeft(var);

			g_token = lexer.GetNextToken();
			binary->SetRight(Expression(lexer, base));
			expression->SetBinary(binary);
			
			Except(TK_END, L';');
			g_token = lexer.GetNextToken();

			return expression;
		}
		else
		{
			Except(TK_END, L';');
			// ȷ��ָ����һ��λ��
			g_token = lexer.GetNextToken();
			return nullptr;
		}
	}
	//
	//	Statements
	//		VariableDefination | IfStatements | WhileStatements | 
	//		BreakStatements | ReturnStatements | Expression 
	//
	ASTreeStatements* Statements(Lexer& lexer, BaseProgram* base)
	{
		ASTreeStatements* state = static_cast<ASTreeStatements*>(
			NewMemory<ASTreeStatements>(g_astManager)
			);

		//g_token = lexer.Get();
		while (true)
		{
			if (g_token.kind == TK_VARIABLE)
			{
				g_token = lexer.GetNextToken();
				auto expression = VariableDefination(lexer, base);
				if (expression == nullptr)
					continue;
				state->InsertStatement(expression);
			}
			else if (g_token.kind == TK_IF)
			{
				state->InsertStatement(IfStatements(lexer, base));
			}
			else if (g_token.kind == TK_WHILE)
			{
				state->InsertStatement(WhileStatements(lexer, base));
			}
			else if (g_token.kind == TK_BREAK || g_token.kind == TK_RETURN)
			{
				state->InsertStatement(BreakOrReturnStatements(lexer, base));
			}
			else if (g_token.kind == TK_IDENTIFIER
				|| g_token.kind == TK_NOT)
			{
				state->InsertStatement(Expression(lexer, base));

				Except(TK_END, L';');
				g_token = lexer.GetNextToken();
			}
			else
			{
				break;
			}
		}

		return state;
	}

	//
	// Defination
	//		"function" identifier "(" [params] ")" 
	//		"{" statements "}"
	//
	Function* Defination(Lexer& lexer, BaseProgram* base)
	{
		// g_token ��Զָ����һ�� token

		Function* func = nullptr;

		g_token = lexer.GetNextToken();
		if (g_token.kind != TK_IDENTIFIER)
		{
			throw ASTError(g_token, L" function �ؼ��ֺ�Ӧ�����û������ʶ��");
		}
		else if (base->GetIdentifier().find(g_token.value) 
			!= base->GetIdentifier().end())
		{
			throw ASTError(g_token, L"��ʶ����" + g_token.value + L" �ض���");
		}

		func = NewMemory<Function>(g_funcManager);
		func->SetFunctionName(g_token.value);
		base->AddIdentifier(g_token.value);

		g_token = lexer.GetNextToken();
		Except(TK_LBRA, L'(');

		g_token = lexer.GetNextToken();
		while (g_token.kind == TK_IDENTIFIER)
		{
			if (!func->FindIndetifier(g_token.value))
			{
				func->AddParam(g_token.value);
				func->AddVariable(g_token.value);
				g_token = lexer.GetNextToken();

				if (g_token.kind == TK_COMMA)
					g_token = lexer.GetNextToken();
				else
					break;
			}
			else
			{
				throw ASTError(g_token, L"��������" + g_token.value + L" �ظ�����");
			}
		}

		Except(TK_RBRA, L')');
		g_token = lexer.GetNextToken();
		Except(TK_LBRACE, L'{');
		g_token = lexer.GetNextToken();

		func->SetStatements(Statements(lexer, func));

		Except(TK_RBRACE, L'}');
		g_token = lexer.GetNextToken();

		return func;
	}

	// 
	// Parser
	//		defination | statements
	//
	void Parser(wstring& file, Program& program)
	{
		Lexer lexer(file);

		g_token = lexer.GetNextToken();
		while (g_token.kind != TK_EOF)
		{
			if (g_token.kind == TK_FUNCTION)
				program.AddFunction(Defination(lexer, &program));
			else
				program.AddStatements(Statements(lexer, &program));
		}

		Check(&program, &program);

		for (auto i : program.GetFunctions())
		{
			Check(i.second, &program);
		}

		return ;
	}
}