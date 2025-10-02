/* Implementation of Interpreter
	for the Simple Ada-Like (SADAL) Language
 * parser.cpp
 * Programming Assignment 3
 * Spring 2025
*/
#include <iostream>
#include <vector>
#include <sstream>
#include "parserInterp.h"


map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> TempsResults; //Container of temporary locations of Value objects for results of expressions, variables values and constants 

vector<string> * Ids_List;

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
    
    cout << endl;
    cout << "(DONE)" << endl;
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
	else {
		if (defVar.count(tok.GetLexeme()) == 0) {
			ParseError(line, "Procedure name mismatch in closing end identifier.");
			return false;
		}
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
	Ids_List = new vector<string>;
	if (tok != IDENT) {
		ParseError(line, "Missing identifier at start of declaration");
		return false;
	}
	defVar[tok.GetLexeme()] = true;
	Ids_List->push_back(tok.GetLexeme());
	
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
				Ids_List->push_back(tok.GetLexeme());
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
	Token tokType = GetNextToken(in, line).GetToken();

	for (const string& id : *Ids_List) {
		if (SymTable.find(id) != SymTable.end()) {
			ParseError(line, "Variable Redefinition");
			return false;
		}
		SymTable[id] = tokType;
	}

	tok = GetNextToken(in, line);
	if (tok == ASSOP) {
		Value initVal;
		if (!Expr(in, line, initVal)) {
			ParseError(line, "Invalid initialization expression");
			return false;
		}
		for (const string& id : *Ids_List) {
            Token idType = SymTable[id];
            if ((idType == INT && !initVal.IsInt()) ||
                (idType == FLOAT && !initVal.IsReal()) ||
                (idType == BOOL && !initVal.IsBool()) ||
                (idType == STRING && !initVal.IsString()) ||
                (idType == CHAR && !initVal.IsChar())) {
                ParseError(line, "Incorrect type in initialization");
                return false;
            }
            TempsResults[id] = initVal;
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
	Parser::PushBackToken(tok);
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
	bool isPUTLN = (tok == PUTLN);

	tok = GetNextToken(in, line);
	if (tok != LPAREN) {
		ParseError(line, "Missing Left Parenthesis");
		ParseError(line, "Invalid put statement.");
		return false;
	}
	
	Value retVal;
	if (!Expr(in, line, retVal)) {
		ParseError(line, "Invalid expression in print statement");
		ParseError(line, "Invalid put statement.");
		return false;
	}

	cout << retVal;
	if (isPUTLN) {
		cout << endl;
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
	
	LexItem idtok;
	if (!Var(in, line, idtok)) {
		return false;
	}
	
	Token varType = SymTable[idtok.GetLexeme()];
    string input;
    cin >> input;

	try {
        switch(varType) {
            case INT:
                TempsResults[idtok.GetLexeme()] = Value(stoi(input));
                break;
            case FLOAT:
                TempsResults[idtok.GetLexeme()] = Value(stod(input));
                break;
            case BOOL:
                if (input == "true") {
					TempsResults[idtok.GetLexeme()] = Value(true);
				}
                else if (input == "false") {
					TempsResults[idtok.GetLexeme()] = Value(false);
				}
                else {
					ParseError(line, "Invalid boolean input in Get");
					return false;
				}
                break;
            case STRING:
				TempsResults[idtok.GetLexeme()] = Value(input);
                break;
            case CHAR:
                if (input.length() != 1) {
					ParseError(line, "Invalid character output in Get");
					return false;
				}
                TempsResults[idtok.GetLexeme()] = Value(input[0]);
                break;
            default:
                ParseError(line, "Invalid variable type in Get");
				return false;
        }
    } catch (...) {
        ParseError(line, "Invalid input for variable type");
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
	
	Value ifVal;
	if (!Expr(in, line, ifVal)) {
		ParseError(line, "Missing if statement condition");
		ParseError(line, "Invalid If statement");
		return false;
	}

	if (!ifVal.IsBool()) {
		ParseError(line, "If condition must be a boolean");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok != THEN) {
		ParseError(line, "Missing THEN in if statement");
		ParseError(line, "Invalid If statement");
		return false;
	}

	if (ifVal.GetBool()) {
		if (!StmtList(in, line)) {
			ParseError(line, "Invalid statement list in if statement");
			ParseError(line, "Invalid If statement");
			return false;
		}
		while (tok != END) {
            tok = Parser::GetNextToken(in, line);
        }
	}
	else {
		while (tok != ELSIF && tok != ELSE && tok != END) {
			tok = GetNextToken(in, line);
		}
	}

	while (tok == ELSIF) {
		if (!Expr(in, line, ifVal)) {
			ParseError(line, "Invalid ELSIF condition");
			return false;
		}

		if (!ifVal.IsBool()) {
			ParseError(line, "ELSIF condition must be a boolean");
			return false;
		}

		tok = GetNextToken(in, line);
		if (tok != THEN) {
			ParseError(line, "Missing THEN in ELSIF");
			return false;
		}

		if (ifVal.GetBool()) {
			if (!StmtList(in, line)) {
				ParseError(line, "Invalid statement list in elsif statement");
				return false;
			}
			while (tok != END) {
				tok = GetNextToken(in, line);
			}
			break;
		}
		else {
			while (tok != ELSIF && tok != ELSE && tok != END) {
				tok = GetNextToken(in, line);
			}
		}
	}

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
	LexItem idtok;
    if (!Var(in, line, idtok)) {
		return false;
	}
    
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok == LPAREN) {
        Value idx1, idx2;
        if (!Range(in, line, idx1, idx2)) {
            ParseError(line, "Invalid range in string indexing");
            return false;
        }
        
        tok = Parser::GetNextToken(in, line);
        if (tok != RPAREN) {
            ParseError(line, "Missing right parenthesis in string indexing");
            return false;
        }
        
        tok = Parser::GetNextToken(in, line);
    }
   
	if (tok != ASSOP) {
		cout << tok.GetLexeme();
		ParseError(line, "Missing Assignment Operator");
		ParseError(line, "Invalid assignment statement.");
		return false;
	}

	Value rhsVal;
	if (!Expr(in, line, rhsVal)) {
		ParseError(line, "Missing Expression in Assignment Statement.");
		ParseError(line, "Invalid assignment statement.");
		return false;
	}
       
	string idName = idtok.GetLexeme();
	Token idType = SymTable[idName];
	if ((idType == INT && !rhsVal.IsInt()) || (idType == FLOAT && !rhsVal.IsReal()) ||
        (idType == BOOL && !rhsVal.IsBool()) || (idType == STRING && !rhsVal.IsString()) ||
        (idType == CHAR && !rhsVal.IsChar())) {
        	ParseError(line, "Incorrect type in Assignment Statement");
            ParseError(line, "Missing Expression in Assignment Statement.");
		    ParseError(line, "Invalid assignment statement.");
        	return false;
    }

	TempsResults[idName] = rhsVal;

	tok = GetNextToken(in, line);
	if (tok != SEMICOL) {
		ParseError(line - 1, "Missing semicolon at end of statement");
		ParseError(line, "Invalid assignment statement.");
		return false;
	}

	return true;
}

bool Var(istream& in, int& line, LexItem& idtok) {
	LexItem tok = GetNextToken(in, line);
	if (defVar.count(tok.GetLexeme()) == 0) {
		ParseError(line, "Using Undefined Variable");
		return false;
	}
	if (tok != IDENT ) {
		return false;
	}
	idtok = tok;
	return true;
}

bool Expr(istream& in, int& line, Value& retVal) {
	LexItem tok;
	Value val1, val2;

	if (!Relation(in, line, val1)) {
		return false;
	}
    
	tok = GetNextToken(in, line);
	while (tok == AND || tok == OR) {
		if (!Relation(in, line, val2)) {
			return false;
		}
		if (tok == AND) {
			val1 = val1 && val2;
		}
		if (tok == OR) {
			val1 = val1 || val2;
		}
		tok = GetNextToken(in, line);
	}
	PushBackToken(tok);
	retVal = val1;

	return true;
}

bool Relation(istream& in, int& line, Value& retVal) {
	LexItem tok;
	Value val1, val2;
	if (!SimpleExpr(in, line, val1)) {
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok == EQ || tok == NEQ || tok == LTHAN || tok == LTE || tok == GTHAN || tok == GTE) {
		if (!SimpleExpr(in, line, val2)) {
			return false;
		}
		switch(tok.GetToken()) {
			case EQ:
				retVal = val1 == val2;
				break;

			case NEQ:
				retVal = val1 != val2;
				break;

			case LTHAN:
				retVal = val1 < val2;
				break;

			case LTE:
				retVal = val1 <= val2;
				break;

			case GTHAN:
				retVal = val1 > val2;
				break;

			case GTE:
				retVal = val1 >= val2;
				break;

			default:
				break;
		}	
	}
	else {
		PushBackToken(tok);
		retVal = val1;
	}

	return true;
}

bool SimpleExpr(istream& in, int& line, Value& retVal) {
	LexItem tok;
	Value val1, val2;
	if (!STerm(in, line, val1)) {
		return false;
	}

	tok = GetNextToken(in, line);
	while (tok == PLUS || tok == MINUS || tok == CONCAT) {
		if (!STerm(in, line, val2)) {
			return false;
		}
		switch(tok.GetToken()) {    
			case PLUS:
				val1 = val1 + val2;
				break;

			case MINUS:
				val1 = val1 - val2;
				break;

			case CONCAT:
				val1 = val1.Concat(val2);
				break;

			default:
				break;
		}
		tok = GetNextToken(in, line);
	}
	PushBackToken(tok);
	retVal = val1;

	return true;
}

bool STerm(istream& in, int& line, Value& retVal) {
	LexItem tok = GetNextToken(in, line);
	if (tok == PLUS) {
		return Term(in, line, 1, retVal);
	}
	else if (tok == MINUS) {
		return Term(in, line, -1, retVal);	
	}
	else {
		PushBackToken(tok);
		return Term(in, line, 1, retVal);
	}
}

bool Term(istream& in, int& line, int sign, Value& retVal) {
	LexItem tok;
	Value val1, val2;
	if (!Factor(in, line, sign, val1)) {
		return false;
	}

	tok = GetNextToken(in, line);
	while (tok == MULT || tok == DIV || tok == MOD) {
		if (!Factor(in, line, sign, val2)) {
			ParseError(line, "Missing operand after operator");
			return false;
		}

		switch(tok.GetToken()) {
			case MULT:
				val1 = val1 * val2;
				break;

			case DIV:
                try {
                    val1 = val1 / val2;
                    break;
                } catch (char const* e) {
                    ParseError(line, "Division by zero");
                    return false;
                }      

			case MOD:
				val1 = val1 % val2;
				break;

			default:
				break;
		}
		tok = GetNextToken(in, line);
	}
	PushBackToken(tok);
	retVal = val1;

	return true;
}

bool Factor(istream& in, int& line, int sign, Value& retVal) {
	LexItem tok = GetNextToken(in, line);
	Value val;
	if (tok == NOT) {
		if (!Primary(in, line, sign, val)) {
			ParseError(line, "Incorrect operand");
			return false;
		}
		if (!val.IsBool()) {
			ParseError(line, "NOT operator requires boolean operand");
			return false;
		}
		retVal = !val;
		return true;
	}

	PushBackToken(tok);
	if (!Primary(in, line, sign, retVal)) {
		ParseError(line, "Incorrect operand");
		return false;
	}

	tok = GetNextToken(in, line);
	if (tok == EXP) {
		Value exp;
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

		if (!Primary(in, line, sign2, exp)) {
			ParseError(line, "Incorrect operand");
			return false;
		}
		if (!retVal.IsReal() || !exp.IsReal()) {
			ParseError(line, "** operator can only be performed on floats");
			return false;
		}

		retVal = retVal.Exp(exp);
	}
	else {
		PushBackToken(tok);
	}

	return true;
}

bool Primary(istream& in, int& line, int sign, Value& retVal) {
    LexItem tok = GetNextToken(in, line);
    
    if (tok == ICONST) {
		retVal = Value(stoi(tok.GetLexeme()) * sign);
		return true;
	} 
	else if (tok == FCONST) {
		retVal = Value(stod(tok.GetLexeme()) * sign);
		return true;
	} 
	else if (tok == SCONST) {
		retVal = Value(tok.GetLexeme());
		return true;
	}
	else if (tok == BCONST) {
		retVal = Value(tok.GetLexeme() == "true");
		return true;
	}
	else if (tok == CCONST) {
		retVal = Value(tok.GetLexeme()[0]);
		return true;
    } 
	else if (tok == LPAREN) {
        if (!Expr(in, line, retVal)) {
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
        return Name(in, line, sign, retVal);
    } else {
        ParseError(line, "Invalid reference to a variable.");
        return false;
    }
}

bool Name(istream& in, int& line, int sign, Value& retVal) {
	LexItem tok = GetNextToken(in, line);

	if (defVar.count(tok.GetLexeme()) == 0) {
		ParseError(line, "Using Undefined Variable");
		ParseError(line, "Invalid reference to a variable.");
		return false;
	}
	if (TempsResults.find(tok.GetLexeme()) == TempsResults.end()) {
		ParseError(line, "Uninitiliazed variable");
		return false;
	}

	retVal = TempsResults[tok.GetLexeme()];
	if(retVal.IsInt()) {
		retVal = retVal * Value(sign);
	}
    else if (retVal.IsReal()) {
        retVal = retVal * Value(float(sign));   
    }
	tok = GetNextToken(in, line);
	if (tok == LPAREN) {
		Value val1, val2;
		if (!Range(in, line, val1, val2)) {
			ParseError(line, "Invalid range in Name");
			return false;
		}
		if (retVal.IsString()) {
            string str = retVal.GetString();
            int len = str.length();
            int start = val1.IsInt() ? val1.GetInt() : 0;
            int end = val2.IsInt() ? val2.GetInt() : len-1;
            
            if (start < 0 || start >= len || end < 0 || end >= len || start > end) {
                ParseError(line, "String index out of range");
                return false;
            }
            
            if (start == end) {
                retVal = Value(str[start]);
            } else {
                retVal = Value(str.substr(start, end-start+1));
            }
        } else {
            ParseError(line, "Indexing only supported for strings");
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

bool Range(istream& in, int& line, Value& retVal1, Value& retVal2) {
	LexItem tok;
	if (!SimpleExpr(in, line, retVal1)) {
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
		else if (!SimpleExpr(in, line, retVal2)) {
			ParseError(line, "Invalid simple expression in range.");
			return false;
		}
	}
	else {
		PushBackToken(tok);
		retVal2 = retVal1;
	}

	return true;
}