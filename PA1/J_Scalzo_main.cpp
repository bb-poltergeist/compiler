#include "lex.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include <set>
#include <vector>

using std::cout;
using std::string;
using std::endl;
using std::map;

int main(int argc, char* argv[]) {
    if (argc == 1) {
        cout << "No specified input file." << endl;
        exit(1);
    }
    
    string fileName = argv[1];
    std::ifstream file(fileName);
    if (!file) {
        cout << "CANNOT OPEN THE FILE " << fileName << endl;
        exit(1);
    }

    map <string, bool> flags = {
        {"-all", false}, {"-num", false}, {"-str", false}, {"-id", false}, {"-kw", false}
    };

    for (int i = 2; i < argc; i++) {
       string str = argv[i];
       if (flags.count(str)) {
           flags[str] = true;
       }
       else if (str[0] == '-') {
            cout << "Unrecognized flag {" << str << "}" << endl;
            exit(1);
        }
       else {
           cout << "Only one file name is allowed." << endl;
           exit(1);
           }
       }

    if (file.peek() == EOF) {
        cout << "Empty file." << endl;
        exit(1);
    }

    int linenum = 1;
    LexItem tok;
    vector<Token> tokens; 
    vector<Token> reservedToks = {IF, ELSE, ELSIF, PUT, PUTLN, GET, INT, FLOAT, CHAR, STRING, BOOL, PROCEDURE, TRUE, FALSE, END, IS, BEGIN, THEN, CONST, MOD, AND, OR, NOT};
    map<Token, string> keywords;
    set<string> stringsAndChars, identifiers;
    set<double> numerics;

    while ((tok = getNextToken(file, linenum)).GetToken() != DONE) {
        if (tok.GetToken() == ERR) {
            cout << "ERR: In line " << linenum << ", Error Message {" << tok.GetLexeme() << "}" << endl;
            exit(1);
        }

        if (flags["-all"]) {
            cout << tok;
        }

        tokens.push_back(tok.GetToken());
        switch (tok.GetToken()) {
            case IDENT:
                identifiers.insert(tok.GetLexeme());
                break;
            
            case SCONST: case CCONST:
                stringsAndChars.insert(tok.GetLexeme());
                break;
            
            case ICONST: case FCONST:
                numerics.insert(stod(tok.GetLexeme()));
                break;

            default:
                if (count(reservedToks.begin(), reservedToks.end(), tok.GetToken()) > 0) { // uses id_or_kw 
                    keywords[tok.GetToken()] = tok.GetLexeme();
                }
        }
    }
    if (tok.GetToken() == DONE) {
        linenum--;
    }

    cout << "\nLines: " << linenum << endl;
    cout << "Total Tokens: " << tokens.size() << endl;
    cout << "Numerals: " << numerics.size() << endl;
    cout << "Characters and Strings : " << stringsAndChars.size() << endl;
    cout << "Identifiers: " << identifiers.size() << endl;
    cout << "Keywords: " << keywords.size() << endl;

    if (flags["-num"] && numerics.size() > 0) {
        cout << "NUMERIC CONSTANTS:" << endl;
        size_t numIndex = 0;
        for (const auto& num : numerics) {
            cout << num;
            if (numIndex < numerics.size() - 1) {
                cout << ", ";
            }
            numIndex++;
        }
        cout << endl;
    }

    if (flags["-str"] && stringsAndChars.size() > 0) {
        cout << "CHARACTERS AND STRINGS:" << endl;
        size_t strIndex = 0;
        for (const auto& str : stringsAndChars) {
            cout << "\"" << str << "\"";
            if (strIndex < stringsAndChars.size() - 1) {
                cout << ", ";
            }
            strIndex++;
        }
        cout << endl;
    }

    if (flags["-id"] && identifiers.size() > 0) {
        cout << "IDENTIFIERS:" << endl;
        size_t idIndex = 0;
        for (const auto& id : identifiers) {
            cout << id;
            if (idIndex < identifiers.size() - 1) {
                cout << ", ";
            }
            idIndex++;
        }
        cout << endl;
    }

    if (flags["-kw"] && keywords.size() > 0) {
        cout << "KEYWORDS:" << endl;
        for (auto it = keywords.begin(); it != keywords.end(); it++) {
            if (it != keywords.begin()) {
                cout << ", ";
            }
            cout << it->second;
        }
        cout << endl;
    }
}