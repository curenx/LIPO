#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cctype>

using namespace std;

int main() {
    ifstream in("program.txt");
    if (!in) {
        cout << "Error (оши,ка)" << endl;
        return 1;
    }

    map<string, string> lexemT{
        {"Begin", "keyword"},
        {"End", "keyword"},
        {"Var", "keyword"},

        {"+", "1"},
        {"-", "2"},
        {"*", "3"},
        {"/", "4"},
        {"(", "5_1"},
        {")", "5_2"},
        {"=", "6"},
        {",", "7"},
        {".", "8"}

    };

    string strLexem;
    char ch;
    int nstr = 1;

    cout << "LEXEMA\t\tTYPE/ID\t\tSTROKA V BLOKNOTE" << endl;
    cout << "-----------------------------------------------" << endl;

    while (in.get(ch)) {
        if (isspace(ch)) {
            if (ch == '\n') nstr++;
            continue;
        }
        if (isalpha(ch)) {
            strLexem = ch;
            bool hasDigit = false;

            while (in.get(ch) && isalnum(ch)) {
                strLexem += ch;
                if (isdigit(ch)) hasDigit = true;
            }
            in.unget();

            if (lexemT.count(strLexem)) {
                cout << strLexem << "\t\t" << lexemT[strLexem] << "\t\t" << nstr << endl;
            }
            else if (hasDigit) {
                cout << strLexem << "\t\twrong\t\t" << nstr << endl;
            }
            else {
                cout << strLexem << "\t\tidentificator\t" << nstr << endl;
            }
            strLexem.clear();
        }
        else if (isdigit(ch)) {
            strLexem = ch;
            bool hasAlpha = false;

            while (in.get(ch) && isalnum(ch)) {
                strLexem += ch;
                if (isalpha(ch)) hasAlpha = true;
            }
            in.unget();

            if (hasAlpha) {
                cout << strLexem << "\t\twrong\t\t" << nstr << endl;
            }
            else {
                cout << strLexem << "\t\tconst\t\t" << nstr << endl;
            }
            strLexem.clear();
        }

        else {
            string oneChar(1, ch);
            if (lexemT.count(oneChar)) {
                cout << oneChar << "\t\t" << lexemT[oneChar] << "\t\t" << nstr << endl;
            }
            else {
                cout << oneChar << "\t\twrong\t\t" << nstr << endl;
            }
        }
    }

    in.close();
    return 0;
}