#include "../Parser/lexer.h"
#include "syntax.h"

namespace ScriptCompile
{
	MemoryManager<ASTree> g_astManager;
	MemoryManager<Function> g_funcManager;
	Token g_token = { TK_NONE, 0, L"NONE" };

	map<int, Operator> sign = {
		{ TK_EQUAL, { OP_EQUAL, 0, 2 } },
		{ TK_VERTICAL, { OP_VERTICAL, 1, 2 } },
		{ TK_AND, { OP_AND, 2, 2 } },
		{ TK_LEFT_RIGHT, { OP_LEFT_RIGHT, 3, 2 } },
		{ TK_NOT_EQUAL, { OP_NOT_EQUAL, 3, 2 } },
		{ TK_LEFT_ARROW, { OP_LEFT_ARROW, 3, 2 } },
		{ TK_LEFT_ARROW_EQUAL, { OP_LEFT_ARROW_EQUAL, 3, 2 } },
		{ TK_RIGHT_ARROW, { OP_RIGHT_ARROW, 3, 2 } },
		{ TK_RIGHT_ARROW_EQUAL, { OP_RIGHT_ARROW_EQUAL, 3, 2 } },
		{ TK_PLUS, { OP_PLUS, 4, 2 } },
		{ TK_MINUS, { OP_MINUS, 4, 2 } },
		{ TK_STAR, { OP_STAR, 5, 2 } },
		{ TK_SLASH, { OP_SLASH, 5, 2 } },
		{ TK_PERCENT, { OP_PERCENT, 5, 2 } },
		{ TK_NOT, { OP_NOT, 6, 1 } },
	};
	////////////////////////////////////////////////////////////////////////////////

	template<class AIM, class T>
	T* NewMemory(MemoryManager<T>& manager)
	{
		AIM* temp = new AIM;
		manager.Insert(temp);

		return temp;
	}

	template<class TYPE>
	inline TYPE *AllocateMemory()
	{
		return static_cast<TYPE*>(NewMemory<TYPE>(g_astManager));
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

	//////////////////////////////////////////////////////////////////////////////////////

	ASTreeExpression* Parser::Expression()
	{
		ASTreeExpression* expression = AllocateMemory<ASTreeExpression>();

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
						ASTreeCall* call = AllocateMemory<ASTreeCall>();
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
						program->GetFuntionUse().insert({ name, index });
						Except(TK_RBRA, L')');
						origin.push(call);
						g_token = lexer.GetNextToken();
						break;
					}
					else
					{
						ASTreeVariable* variable = AllocateMemory<ASTreeVariable>();
						program->GetIdentifierUse().insert(name);
						variable->SetValueKind(TK_IDENTIFIER);
						variable->SetValue(name);
						origin.push(variable);
						break;
					}
				}

			case TK_STRING: case TK_INTEGER: case TK_REAL:
				{
					ASTreeVariable* var = AllocateMemory<ASTreeVariable>();
					var->SetValueKind(g_token.kind);
					var->SetValue(g_token.value);
					origin.push(var);
					g_token = lexer.GetNextToken();
					break;
				}

			case TK_VERTICAL: case TK_AND: case TK_PLUS: case TK_MINUS:
			case TK_STAR: case TK_SLASH: case TK_PERCENT: case TK_NOT:
			case TK_LBRA: case TK_EQUAL: case TK_RBRA: case TK_LEFT_ARROW:
			case TK_RIGHT_ARROW: case TK_LEFT_ARROW_EQUAL: case TK_NOT_EQUAL:
			case TK_RIGHT_ARROW_EQUAL: case TK_LEFT_RIGHT:
				{
					ASTreeVariable* sign = AllocateMemory<ASTreeVariable>();
					sign->SetValueKind(g_token.kind);
					origin.push(sign);
					g_token = lexer.GetNextToken();
					break;
				}

			default:
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

				case TK_VERTICAL: case TK_AND: case TK_PLUS: case TK_MINUS:
				case TK_STAR: case TK_SLASH: case TK_PERCENT: case TK_NOT:
				case TK_EQUAL: case TK_LEFT_ARROW: case TK_RIGHT_ARROW:
				case TK_LEFT_ARROW_EQUAL: case TK_NOT_EQUAL:
				case TK_RIGHT_ARROW_EQUAL: case TK_LEFT_RIGHT:
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
						ASTreeUnary* unary = AllocateMemory<ASTreeUnary>();

						if (!sloveStack.size())
							throw ASTError(g_token, L"������ʽ���󣬵�Ŀ�������û���ҵ�����");

						unary->SetOperator(sign[TK_NOT]);
						unary->SetExprssion(sloveStack.top());
						sloveStack.pop();
						sloveStack.push(unary);
						break;
					}

				case TK_VERTICAL: case TK_AND: case TK_PLUS: case TK_MINUS:
				case TK_STAR: case TK_SLASH: case TK_PERCENT: case TK_EQUAL:
				case TK_LEFT_ARROW: case TK_RIGHT_ARROW: case TK_LEFT_ARROW_EQUAL:
				case TK_NOT_EQUAL: case TK_RIGHT_ARROW_EQUAL: case TK_LEFT_RIGHT:
					{
						ASTreeBinary* binary = AllocateMemory<ASTreeBinary>();

						if (sloveStack.size() < 2)
							throw ASTError(g_token, L"������ʽ����˫Ŀ�����ȱ���ҵ�����");

						// �˴�������ԭ�ȵ�˳��
						binary->SetOperator(sign[astvar.GetValueKind()]);
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
	// BreakStatements
	//		"break"
	//
	ASTreeBreakStatement* Parser::BreakStatements()
	{
		g_token = lexer.GetNextToken();
		Except(TK_END, L';');
		g_token = lexer.GetNextToken();
		return AllocateMemory<ASTreeBreakStatement>();
	}

	// 
	// BreakOrReturnStatements 
	//		"return" [experssion] ";" | "break" ";"
	//
	ASTreeReturnStatement* Parser::ReturnStatements()
	{
		ASTreeReturnStatement* astree = AllocateMemory<ASTreeReturnStatement>();

		g_token = lexer.GetNextToken();
		astree->SetExprssion(Expression());
		Except(TK_END, L';');
		g_token = lexer.GetNextToken();

		return astree;
	}

	//
	// WhileStatements
	//		"while" "(" expression ")" 
	//		"{" [statements] "}"
	//
	ASTreeWhileStatement* Parser::WhileStatements()
	{
		ASTreeWhileStatement* astree = AllocateMemory<ASTreeWhileStatement>();

		lexer.TakeNotes();
		g_token = lexer.GetNextToken();
		Except(TK_LBRA, L'(');
		lexer.Restore();

		astree->SetCondition(Expression());

		Except(TK_LBRACE, L'{');
		g_token = lexer.GetNextToken();

		astree->SetStatements(Statements());

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
	ASTreeIfStatement* Parser::IfStatements()
	{
		ASTreeIfStatement* astree = AllocateMemory<ASTreeIfStatement>();

		lexer.TakeNotes();
		g_token = lexer.GetNextToken();
		Except(TK_LBRA, L'(');
		lexer.Restore();

		astree->SetCondition(Expression());

		Except(TK_LBRACE, L'{');
		g_token = lexer.GetNextToken();

		astree->SetIfStatements(Statements());

		Except(TK_RBRACE, L'}');
		g_token = lexer.GetNextToken();
		if (g_token.kind == TK_ELSE)
		{
			g_token = lexer.GetNextToken();
			Except(TK_LBRACE, L'{');
			g_token = lexer.GetNextToken();

			astree->SetElseStatements(Statements());

			Except(TK_RBRACE, L'}');
			g_token = lexer.GetNextToken();
		}

		return astree;
	}

	//
	// VariableDefination
	//		"var" identifier [ "=" expression ] ";"  
	//
	ASTreeExpression* Parser::VariableDefination()
	{
		if (g_token.kind != TK_IDENTIFIER)
			throw ASTError(g_token, L" var ��Ӧ�ý�����Ҫ�����ı�ʶ��");
		else if (program->GetIdentifier().find(g_token.value) 
			!= program->GetIdentifier().end())
			throw ASTError(g_token, L"��ʶ����" + g_token.value + L"�ض���");

		wstring name = g_token.value;
		program->AddVariable(name);

		g_token = lexer.GetNextToken();
		if (g_token.kind == TK_EQUAL)
		{
			ASTreeExpression* expression = AllocateMemory<ASTreeExpression>();
			ASTreeBinary* binary = AllocateMemory<ASTreeBinary>();
			ASTreeVariable* var = AllocateMemory<ASTreeVariable>();

			var->SetValueKind(TK_IDENTIFIER);
			var->SetValue(name);
			binary->SetOperator(sign[TK_EQUAL]);
			binary->SetLeft(var);

			g_token = lexer.GetNextToken();
			binary->SetRight(Expression());
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
	ASTreeStatements* Parser::Statements()
	{
		ASTreeStatements* state = AllocateMemory<ASTreeStatements>();

		//g_token = lexer.Get();
		while (true)
		{
			if (g_token.kind == TK_VARIABLE)
			{
				g_token = lexer.GetNextToken();
				auto expression = VariableDefination();
				if (expression == nullptr)
					continue;
				state->InsertStatement(expression);
			}
			else if (g_token.kind == TK_IF)
			{
				state->InsertStatement(IfStatements());
			}
			else if (g_token.kind == TK_WHILE)
			{
				state->InsertStatement(WhileStatements());
			}
			else if (g_token.kind == TK_BREAK)
			{
				state->InsertStatement(BreakStatements());
			}
			else if (g_token.kind == TK_RETURN)
			{
				state->InsertStatement(ReturnStatements());
			}
			else if (g_token.kind == TK_IDENTIFIER
				|| g_token.kind == TK_NOT)
			{
				state->InsertStatement(Expression());

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

	void Parser::ParserStatements()
	{
		if (program->GetKind() == PRO_FUNCTION)
		{
			auto function = static_cast<Function*>(program);
			function->SetStatements(Statements());
		}
		else
		{
			auto pro = static_cast<Program*>(program);
			pro->AddStatements(Statements());
		}
	}

	//
	// Defination
	//		"function" identifier "(" [params] ")" 
	//		"{" statements "}"
	//
	Function* Defination(Parser &parser, BaseProgram* base)
	{
		// g_token ��Զָ����һ�� token
		Lexer &lexer = parser.GetLexer();
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

		parser.Bind(func);
		parser.ParserStatements();
		//func->SetStatements(Statements(lexer, func));

		Except(TK_RBRACE, L'}');
		g_token = lexer.GetNextToken();

		return func;
	}

	// 
	// Parser
	//		defination | statements
	//
	void ParserASTreeUnit(wstring& file, Program& program)
	{
		Parser parser(file);
		Lexer &lexer = parser.GetLexer();

		g_token = lexer.GetNextToken();
		while (g_token.kind != TK_EOF)
		{
			if (g_token.kind == TK_FUNCTION)
			{ 
				program.AddFunction(Defination(parser, &program));	
				parser.Bind(&program);
			}
			else
			{
				parser.Bind(&program);
				parser.ParserStatements();
			}
		}

		program.CheckAll();

		return ;
	}
}