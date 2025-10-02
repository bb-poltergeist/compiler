#include <queue>
#include "parser.h"


map<string, bool> defVar;

namespace Parser {
	bool pushed_back = false;
	LexItem	pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		if( pushed_back ) {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem & t) {
		if( pushed_back ) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;	
	}

}

using Parser::GetNextToken;
using Parser::PushBackToken;

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
	++error_count;
	cout << line << ": " << msg << endl;
}

bool Prog(istream& in, int& line) {
	LexItem tok = GetNextToken(in, line);
	if (tok != PROCEDURE) {
		ParseError(line, "Incorrect compilation file.");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok != IDENT) {
		ParseError(line, "Missing Procedure Name.");
		return false;
	}
	else {
		defVar[tok.GetLexeme()] = true;
	}

	tok = GetNextToken(in, line);
	if (tok != IS) {
		ParseError(line, "Missing IS");
		return false;
	}

	if (!ProcBody(in, line)) {
		ParseError(line, "Incorrect Procedure Definition.");
		return false;
	}

	cout << "Declared Variables:" << endl;
    bool first = true;
    for (const auto& var : defVar) {
        if (!first) cout << ", ";
        cout << var.first;
        first = false;
    }
    cout << endl << "\n(DONE)" << endl;

	return true;
}

bool ProcBody(istream& in, int& line) {
	if (!DeclPart(in, line)) {
		ParseError(line, "Incorrect procedure body");
		return false;
	}

	LexItem tok = GetNextToken(in, line);
	if (tok != BEGIN) {
		ParseError(line, "Incorrect procedure body");
		return false;
	}

	if (!StmtList(in, line)) {
		ParseError(line, "Incorrect procedure body");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok != END) {
		ParseError(line, "Incorrect procedure body");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok != IDENT) {
		ParseError(line, "Incorrect procedure body");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok != SEMICOL) {
		ParseError(line - 1, "Incorrect procedure body");
		return false;
	}

	return true;
}

bool DeclPart(istream& in, int& line) {
	bool status = false;
	LexItem tok;
	//cout << "in DeclPart" << endl;
	status = DeclStmt(in, line);
	if(status)
	{
		tok = GetNextToken(in, line);
		if(tok == BEGIN )
		{
			PushBackToken(tok);
			return true;
		}
		else 
		{
			PushBackToken(tok);
			status = DeclPart(in, line);
		}
	}
	else
	{
		ParseError(line, "Non-recognizable Declaration Part.");
		return false;
	}
	return status;
}

bool DeclStmt(istream& in, int& line) {
	LexItem tok = GetNextToken(in, line);
	if (tok != IDENT) {
		ParseError(line, "Missing identifier at start of declaration");
		return false;
	}
	defVar[tok.GetLexeme()] = true;
	
	while (true) {
		tok = GetNextToken(in, line);
		if (tok == COMMA) {
			tok = GetNextToken(in, line);
			if (tok != IDENT) {
				ParseError(line, "Missing IDENT after comma");
				ParseError(line, "Incorrect identifiers list in Declaration Statement");
				return false;
			}
			else if (defVar.count(tok.GetLexeme()) == 1) {
				ParseError(line, "Variable Redefinition");
				ParseError(line, "Incorrect identifiers list in Declaration Statement");
				return false;
			}
			else {
				defVar[tok.GetLexeme()] = true;
			}
		} else if (tok == IDENT) {
			ParseError(line, "Missing comma in declaration statement.");
			ParseError(line, "Incorrect identifiers list in Declaration Statement");
			return false;
		} else {
			PushBackToken(tok);
			break;
		}
	}

	tok = GetNextToken(in, line);
	if (tok != COLON) {
		ParseError(line, string("Invalid name for an Identifier:\n(") + tok.GetLexeme() + ")");
		ParseError(line, "Incorrect identifiers list in Declaration Statement");
		return false;
	}

	bool isConst = false;
	tok = GetNextToken(in, line);
	if (tok == CONST) {
		isConst = true;
		tok = GetNextToken(in, line);
	} 
	else {
		PushBackToken(tok);
	} 

	if (isConst == true) {
		PushBackToken(tok);
	}

	if (!Type(in, line)) {
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok == LPAREN) {
		if (!Range(in, line)) {
			ParseError(line, "Missing Range after Left Parenthesis.");
			return false;
		}
		tok = GetNextToken(in, line);
		if (tok != RPAREN) {
			ParseError(line, "Missing Right Parenthesis");
			return false;
		}
		tok = GetNextToken(in, line);
	}

	if (tok == ASSOP) {
		if (!Expr(in, line)) {
			ParseError(line, "Missing Expression after Assignment Operator");
			return false;
		}
		tok = GetNextToken(in, line);
	}

	if (tok != SEMICOL) {
		ParseError(line - 1, "Missing semicolon at end of statement");
		return false;
	}
	
	return true;
}

bool Type(istream& in, int& line) {
	LexItem tok = GetNextToken(in, line);
	if (tok != INT && tok != FLOAT && tok != BOOL && tok != STRING && tok != CHAR) {
		ParseError(line, "Incorrect Declaration Type.");
		return false;
	}
	return true;
}

bool StmtList(istream& in, int& line)
{
	bool status;
	LexItem tok;
	//cout << "in StmtList" << endl;
	status = Stmt(in, line);
	tok = Parser::GetNextToken(in, line);
	while(status && (tok != END && tok != ELSIF && tok != ELSE))
	{
		Parser::PushBackToken(tok);
		status = Stmt(in, line);
		tok = Parser::GetNextToken(in, line);
	}
	if(!status)
	{
		ParseError(line, "Syntactic error in statement list.");
		return false;
	}
	Parser::PushBackToken(tok); //push back the END token
	return true;
}

bool Stmt(istream& in, int& line) {
	LexItem tok = GetNextToken(in, line);

	if (tok == IDENT) {
		PushBackToken(tok);
		return AssignStmt(in, line);
	} 
	else if (tok == PUTLN || tok == PUT) {
		PushBackToken(tok);
		return PrintStmts(in, line);
	}
	else if (tok == GET) {
		PushBackToken(tok);
		return GetStmt(in, line);
	}
	else if (tok == IF) {
		PushBackToken(tok);
		return IfStmt(in, line);
	}
	else {
		ParseError(line, "Invalid statement.");
		return false;
	}
}

bool PrintStmts(istream& in, int& line) {
	LexItem tok = GetNextToken(in, line);
	if (tok != PUT && tok != PUTLN) {
		ParseError(line, "Invalid put statement.");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok != LPAREN) {
		ParseError(line, "Missing Left Parenthesis");
		ParseError(line, "Invalid put statement.");
		return false;
	}
	
	if (!Expr(in, line)) {
		ParseError(line, "Invalid expression in print statement");
		ParseError(line, "Invalid put statement.");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok != RPAREN) {
		ParseError(line, "Missing Right Parenthesis");
		ParseError(line, "Invalid put statement.");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok != SEMICOL) {
		ParseError(line - 1, "Missing semicolon at end of statement");
		ParseError(line - 1, "Invalid put statement.");
		return false;
	}

	return true;
}

bool GetStmt(istream& in, int& line) {
	LexItem tok = GetNextToken(in, line);
	if (tok != GET) {
		ParseError(line, "Missing GET");
		return false;
	}
	
	tok = GetNextToken(in, line);
	if (tok != LPAREN) {
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}
	
	if (!Var(in, line)) {
		ParseError(line, "Invalid variable");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok != RPAREN) {
		ParseError(line, "Missing Right Parenthesis");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok != SEMICOL) {
		ParseError(line - 1, "Missing semicolon at end of statement");
		return false;
	}

	return true;
}

bool IfStmt(istream& in, int& line) {
	LexItem tok = GetNextToken(in, line);
	if (tok != IF) {
		ParseError(line, "Missing IF in if statement");
		ParseError(line, "Invalid If statement");
		return false;
	}
	
	if (!Expr(in, line)) {
		ParseError(line, "Missing if statement condition");
		ParseError(line, "Invalid If statement");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok != THEN) {
		ParseError(line, "Missing THEN in if statement");
		ParseError(line, "Invalid If statement");
		return false;
	}

	if (!StmtList(in, line)) {
		ParseError(line, "Invalid statement list in if statement");
		ParseError(line, "Invalid If statement");
		return false;
	}

	while (true) {
		tok = GetNextToken(in, line);
		if (tok == ELSIF) {
			if (!Expr(in, line)) {
				ParseError(line, "Missing expression after ELSIF");
				return false;
			}
			tok = GetNextToken(in, line);
			if (tok != THEN) {
				ParseError(line, "Missing THEN in ELSIF");
				return false;
			}
			if (!StmtList(in, line)) {
				ParseError(line, "Invalid statement list in elsif statement");
				return false;
			}
		}
		else {
			PushBackToken(tok);
			break;
		}
	}

	tok = GetNextToken(in, line);
	if (tok == ELSE) {
		if (!StmtList(in, line)) {
			ParseError(line, "Invalid statement list in else statement.");
			return false;
		}
		tok = GetNextToken(in, line);
	}

	if (tok != END) {
		ParseError(line, "Missing END in if statement.");
		ParseError(line, "Invalid If statement");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok != IF) {
		ParseError(line, "Missing IF at end of if statement.");
		ParseError(line, "Invalid If statement");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok != SEMICOL) {
		ParseError(line - 1, "Missing semicolon at end of statement");
		ParseError(line, "Invalid If statement");
		return false;
	}

	return true;
}

bool AssignStmt(istream& in, int& line) {
	LexItem tok;
	if (!Var(in, line)) {
		ParseError(line, "Invalid variable");
		ParseError(line, "Invalid assignment statement.");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok != ASSOP) {
		ParseError(line, "Missing Assignment Operator");
		ParseError(line, "Invalid assignment statement.");
		return false;
	}

	if (!Expr(in, line)) {
		ParseError(line, "Missing Expression in Assignment Statement.");
		ParseError(line, "Invalid assignment statement.");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok != SEMICOL) {
		ParseError(line - 1, "Missing semicolon at end of statement");
		ParseError(line, "Invalid assignment statement.");
		return false;
	}

	return true;
}

bool Var(istream& in, int& line) {
	LexItem tok = GetNextToken(in, line);
	if (defVar.count(tok.GetLexeme()) == 0) {
		ParseError(line, "Using Undefined Variable");
		return false;
	}
	if (tok != IDENT ) {
		return false;
	}
	return true;
}

bool Expr(istream& in, int& line) {
	LexItem tok;
	if (!Relation(in, line)) {
		return false;
	}

	tok = GetNextToken(in, line);
	while (tok == AND || tok == OR) {
		if (!Relation(in, line)) {
			return false;
		}
		tok = GetNextToken(in, line);
	}
	PushBackToken(tok);

	return true;
}

bool Relation(istream& in, int& line) {
	LexItem tok;
	if (!SimpleExpr(in, line)) {
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok == EQ || tok == NEQ || tok == LTHAN || tok == LTE || tok == GTHAN || tok == GTE) {
		if (!SimpleExpr(in, line)) {
			return false;
		}
	}
	else {
		PushBackToken(tok);
	}

	return true;
}

bool SimpleExpr(istream& in, int& line) {
	LexItem tok;
	if (!STerm(in, line)) {
		return false;
	}

	tok = GetNextToken(in, line);
	while (tok == PLUS || tok == MINUS || tok == CONCAT) {
		if (!STerm(in, line)) {
			return false;
		}
		tok = GetNextToken(in, line);
	}
	PushBackToken(tok);

	return true;
}

bool STerm(istream& in, int& line) {
	LexItem tok = GetNextToken(in, line);
	if (tok == PLUS) {
		return Term(in, line, 1);
	}
	else if (tok == MINUS) {
		return Term(in, line, -1);	
	}
	else {
		PushBackToken(tok);
		return Term(in, line, 1);
	}
}

bool Term(istream& in, int& line, int sign) {
	LexItem tok;
	if (!Factor(in, line, sign)) {
		return false;
	}

	tok = GetNextToken(in, line);
	while (tok == MULT || tok == DIV || tok == MOD) {
		if (!Factor(in, line, sign)) {
			ParseError(line, "Missing operand after operator");
			return false;
		}
		tok = GetNextToken(in, line);
	}
	PushBackToken(tok);

	return true;
}

bool Factor(istream& in, int& line, int sign) {
	LexItem tok = GetNextToken(in, line);
	if (tok == NOT) {
		if (!Primary(in, line, sign)) {
			ParseError(line, "Incorrect operand");
			return false;
		}
		return true;
	}

	PushBackToken(tok);
	if (!Primary(in, line, sign)) {
		ParseError(line, "Incorrect operand");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok == EXP) {
		tok = GetNextToken(in, line);
		int sign2 = 1;
		if (tok == PLUS) {
			sign2 = 1;
		}
		else if (tok == MINUS) {
			sign2 = -1;
		}
		else {
			PushBackToken(tok);
		}

		if (!Primary(in, line, sign2)) {
			ParseError(line, "Incorrect operand");
			return false;
		}
	}
	else {
		PushBackToken(tok);
	}

	return true;
}

bool Primary(istream& in, int& line, int sign) {
    LexItem tok = GetNextToken(in, line);
    
    if (tok == ICONST || tok == FCONST || tok == SCONST || tok == BCONST || tok == CCONST) {
        return true;
    } else if (tok == LPAREN) {
        if (!Expr(in, line)) {
			ParseError(line, "Invalid expression in primary");
            return false;
        }
        tok = GetNextToken(in, line);
        if (tok != RPAREN) {
            ParseError(line, "Missing right parenthesis after expression");
            return false;
        }
        return true;
    } else if (tok == IDENT) {
        PushBackToken(tok);
        return Name(in, line);
    } else {
        ParseError(line, "Invalid reference to a variable.");
        return false;
    }
}

bool Name(istream& in, int& line) {
	LexItem tok = GetNextToken(in, line);
	if (defVar.count(tok.GetLexeme()) == 0) {
		ParseError(line, "Using Undefined Variable");
		ParseError(line, "Invalid reference to a variable.");
		return false;
	}
	if (tok != IDENT ) {
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok == LPAREN) {
		if (!Range(in, line)) {
			ParseError(line, "Invalid range in Name");
			return false;
		}
		tok = GetNextToken(in, line);
		if (tok != RPAREN) {
			ParseError(line, "Missing Right Parenthesis");
			return false;
		}
	}
	else {
		PushBackToken(tok);
	}

	return true;
}

bool Range(istream& in, int& line) {
	LexItem tok;
	if (!SimpleExpr(in, line)) {
		ParseError(line, "Missing simple expression in range");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok == DOT) {
		tok = GetNextToken(in, line);
		if (tok != DOT) {
			ParseError(line, "Missing second dot");
			return false;
		}
		else if (!SimpleExpr(in, line)) {
			ParseError(line, "Invalid simple expression in range.");
			return false;
		}
	}
	else {
		PushBackToken(tok);
	}

	return true;
}