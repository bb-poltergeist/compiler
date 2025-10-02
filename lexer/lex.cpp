#include "lex.h"
#include <string>
#include <map>
#include <cctype>
#include <algorithm>

using std::string;
using std::map;
using std::endl;

map<Token, string> stringTokens {
    {PROCEDURE, "PROCEDURE"}, {STRING, "STRING"}, {ELSE, "ELSE"}, {IF, "IF"},
    {INT, "INT"}, {FLOAT, "FLOAT"}, {CHAR, "CHAR"}, {PUT, "PUT"},
    {PUTLN, "PUTLN"}, {GET, "GET"}, {BOOL, "BOOL"}, {TRUE, "TRUE"},
    {FALSE, "FALSE"}, {ELSIF, "ELSIF"}, {IS, "IS"}, {END, "END"},
    {BEGIN, "BEGIN"}, {THEN, "THEN"}, {CONST, "CONST"}, {MOD, "MOD"},
    {AND, "AND"}, {OR, "OR"}, {NOT, "NOT"},

    {PLUS, "PLUS"}, {MINUS, "MINUS"}, {MULT, "MULT"}, {DIV, "DIV"},
    {ASSOP, "ASSOP"}, {EQ, "EQ"}, {NEQ, "NEQ"}, {LTHAN, "LTHAN"},
    {GTHAN, "GTHAN"}, {LTE, "LTE"}, {GTE, "GTE"}, {CONCAT, "CONCAT"},
    {EXP, "EXP"},

    {COMMA, "COMMA"}, {SEMICOL, "SEMICOL"}, {LPAREN, "LPAREN"},
    {RPAREN, "RPAREN"}, {COLON, "COLON"}, {DOT, "DOT"},

    {ICONST, "ICONST"}, {FCONST, "FCONST"}, {SCONST, "SCONST"},
    {CCONST, "CCONST"}, {BCONST, "BCONST"}, {IDENT, "IDENT"},

    {ERR, "ERR"}, {DONE, "DONE"}
};

map<string, Token> keywords = {
    {"procedure", PROCEDURE}, {"string", STRING}, {"else", ELSE}, {"if", IF}, {"integer", INT}, {"float", FLOAT},
    {"character", CHAR}, {"put", PUT}, {"putline", PUTLN}, {"get", GET}, {"boolean", BOOL}, {"true", TRUE},
    {"false", FALSE}, {"elsif", ELSIF}, {"is", IS}, {"end", END}, {"begin", BEGIN}, {"then", THEN}, {"constant", CONST},
    {"mod", MOD}, {"and", AND}, {"or", OR}, {"not", NOT}
};

ostream& operator<<(ostream& out, const LexItem& tok) {
    switch (tok.GetToken()) {
        case ICONST:
            out << "ICONST: (" << tok.GetLexeme() << ")" << endl;
            break;
        case FCONST:
            out << "FCONST: (" << tok.GetLexeme() << ")" << endl;
            break;
        case BCONST:
            out << "BCONST: (" << tok.GetLexeme() << ")" << endl;
            break;
        case IDENT:
            out << "IDENT: <" << tok.GetLexeme() << ">" << endl;
            break;
        case SCONST:
            out << "SCONST: \"" << tok.GetLexeme() << "\"" << endl;
            break;
        case CCONST:
            out << "CCONST: '" << tok.GetLexeme() << "'" << endl;
            break;
        case ERR:
            out << "ERR: Unrecognized Lexeme {" << tok.GetLexeme() << "} in line " << tok.GetLinenum() << endl;
            break;
        default:
            if (tok.GetToken() != DONE) {
                out << stringTokens[tok.GetToken()] << endl;
            }
    }
    return out;
}

LexItem id_or_kw(const string& lexeme, int linenum) {
    string lower = lexeme;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (keywords.count(lower) && (keywords[lower] == TRUE || keywords[lower] == FALSE)) {
        return LexItem(BCONST, lexeme, linenum);
    }
    else if (keywords.count(lower)) {
        return LexItem(keywords[lower], lexeme, linenum);
    }
    else {
        return LexItem(IDENT, lexeme, linenum);
    }
}

LexItem getNextToken (istream& in, int& linenumber) {
    enum State {START, INID, ININT, INSTRING, INCHAR, INCOMMENT, ERROR, DONE};
    State lexstate = START;
    string lexeme;
    char ch;

    map<char, Token> singleOperators = {
        {'+', PLUS}, {'-', MINUS}, {'*', MULT}, {'/', DIV}, {'=', EQ}, 
        {'<', LTHAN}, {'>', GTHAN}, {'&', CONCAT}
    };
    
    map<string, Token> multiOperators = {
        {":=", ASSOP}, {"/=", NEQ}, {"<=", LTE}, {">=", GTE}, {"**", EXP}
    };

    map<char, Token> delimiters = {
        {',', COMMA}, {';', SEMICOL}, {'(', LPAREN}, {')', RPAREN}, {':', COLON}, {'.', DOT}
    };

    while (in.get(ch)) {
        switch (lexstate) {
            case START:
            {
                if (isspace(ch)) {
                    if (ch == '\n') {
                        linenumber++;
                    }
                    continue;
                }
                else if (isalpha(ch)) {
                    lexeme = tolower(ch);
                    lexstate = INID;
                    continue;
                }
                else if (isdigit(ch)) {
                    lexeme = ch;
                    lexstate = ININT;
                    continue;
                }
                else if (ch == '-' && in.peek() == '-') {
                    in.get();
                    lexstate = INCOMMENT;
                    continue;
                }
                else if (ch == '"') {
                    lexeme = "";
                    lexstate = INSTRING;
                    continue;
                }
                else if (ch == '\'') {
                    lexeme = "";
                    lexstate = INCHAR;
                    continue;
                }

                lexeme = ch;
                char nextChar = in.peek();
                string multiOp = lexeme + nextChar;
                if (multiOperators.count(multiOp)) {
                    in.get();
                    return LexItem(multiOperators[multiOp], lexeme, linenumber);
                }
                else if (singleOperators.count(ch)) {
                    return LexItem(singleOperators[ch], lexeme, linenumber);
                }
                else if (delimiters.count(ch)) {
                    return LexItem(delimiters[ch], lexeme, linenumber);
                }
                else {
                    lexstate = ERROR; // only returns if no terminal is matched
                }
                break;
            }

            case INID:
                if (isalnum(ch) || ch == '_') {
                    char lowerChar = tolower(ch);
                    if (ch == '_' && in.peek() == '_') {
                        lexeme += ch;
                        return id_or_kw(lexeme, linenumber);
                    }
                    else if (ch == '_' && !lexeme.empty() && lexeme.back() == '_') {
                        return LexItem(ERR, lexeme + lowerChar, linenumber);
                    }
                    else {
                        lexeme += lowerChar;
                    }
                }
                else {
                    in.putback(ch);
                    return id_or_kw(lexeme, linenumber);
                }
                break;
                            
            case ININT:
                if (isdigit(ch) || ch == '.') {
                    if (ch == '.' && !isdigit(in.peek())) {
                        in.putback(ch);
                        if (lexeme.find('.') != string::npos) {
                            return LexItem(FCONST, lexeme, linenumber);
                        }
                        else {
                            return LexItem(ICONST, lexeme, linenumber);
                        }
                    }
                    lexeme += ch;
                    if (count(lexeme.begin(), lexeme.end(), '.') > 1) {
                        return LexItem(ERR, lexeme, linenumber);
                    }
                }
                else if (tolower(ch) == 'e' && (in.peek() == '+' || in.peek() == '-' || isdigit(in.peek()))) {
                    lexeme += ch;
                    in.get(ch);
                    lexeme += ch;

                    while (isdigit(in.peek())) {
                        in.get(ch);
                        lexeme += ch;
                    }
                }
                else {
                    char temp = ch;
                    in.putback(ch);
                    if (lexeme.find('.') != string::npos) {
                        return LexItem(FCONST, lexeme, linenumber);
                    }
                    else {
                        return LexItem(ICONST, lexeme, linenumber);
                    }
                    if (temp == 'e') {
                        return LexItem(IDENT, "e", linenumber);
                    }
                }
                break;

            case INCOMMENT:
                while (ch != '\n') {
                    in.get(ch);
                }
                linenumber++;
                lexstate = START;
                break;
            
            case INSTRING:
                if (ch == '"') {
                    return LexItem(SCONST, lexeme, linenumber);
                }
                else if (ch == '\'' || ch == '\n') {
                    if (ch == '\'')  {
                        lexeme += ch;
                    }
                    cout << "ERR: In line " << linenumber << ", Error Message { Invalid string constant \"" << lexeme << "}" << endl;
                    exit(1);
                }
                else {
                    lexeme += ch;
                }
                break;

            case INCHAR:
                if (ch == '\n') {
                    cout << "ERR: In line " << linenumber << ", Error Message {New line is an invalid character constant.}" << endl;
                    exit(1);
                }
                else if (ch == 'e') {
                    cout << "ERR: In line " << linenumber << ", Error Message { Invalid character constant \'" << lexeme << "\'}" << endl;
                    exit(1);
                }
                else if (ch == '\'') {
                    return LexItem(CCONST, lexeme, linenumber);
                }
                else {
                    lexeme += ch;
                }
                break;

            default:
                lexstate = ERROR;
        }
    }
    if (lexstate == ERROR) {
        return LexItem(ERR, lexeme, linenumber);
    }
    
    return LexItem(Token::DONE, "", linenumber);
}