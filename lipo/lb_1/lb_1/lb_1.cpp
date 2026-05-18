#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <vector>

void S();
void A();
void B();
void C();
void F();
void H();
void G();
void O();
void P();

struct Token {
    int line;
    std::string value;
    std::string type;
};

struct Idents {
    std::string name;
    int val;
};

std::vector<Token> tokens;
std::vector<Idents> idents;
int pos = 0;
int errorCount = 0;

Token currentToken() {
    if (pos < tokens.size()) return tokens[pos];
    return { -1, "EOF", "Конец файла" };
}

void error(std::string message) {
    errorCount++;
    std::cout << "[ОШИБКА] Строка " << currentToken().line << ": " << message
        << " (найдено: '" << currentToken().value << "')" << std::endl;
}

bool match(std::string expectedValue) {
    if (currentToken().value == expectedValue) {
        pos++;
        return true;
    }
    error("Ожидалось '" + expectedValue + "'");
    if (pos < tokens.size()) pos++;
    return false;
}


void readFile(const std::string file_name) {
    std::ifstream file1(file_name);
    if (!file1.is_open()) {
        std::cout << "[ОШИБКА] Не удалось открыть файл " << file_name << std::endl;
        std::cout << "Убедитесь, что файл лежит в папке с решением!" << std::endl;
        return;
    }
    char ch;
    int line = 1;
    while (file1.get(ch)) {
        if (ch == '\n') { line++; continue; }
        if (isspace(ch)) continue;

        if (isalpha(ch)) {
            std::string word;
            word += ch;
            while (file1.get(ch) && isalnum(ch)) word += ch;
            if (!file1.eof()) file1.putback(ch);

            if (word == "Var" || word == "Begin" || word == "End")
                tokens.push_back({ line, word, "Ключевое слово" });
            else
                tokens.push_back({ line, word, "Идентификатор" });
        }
        else if (isdigit(ch)) {
            std::string num;
            num += ch;
            while (file1.get(ch) && isdigit(ch)) num += ch;
            if (!file1.eof()) file1.putback(ch);
            tokens.push_back({ line, num, "Число" });
        }
        else if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
            tokens.push_back({ line, std::string(1, ch), "Операторы действий" });
        }
        else if (ch == ',' || ch == '(' || ch == ')' || ch == '=') {
            tokens.push_back({ line, std::string(1, ch), "Разделитель" });
        }
        else {
            
            tokens.push_back({ line, std::string(1, ch), "Не распознано" });
        }
    }
    file1.close();
}

bool isDeclared(std::string name) {
    for (const auto& id : idents) if (id.name == name) return true;
    return false;
}



void O() { 
    Token t = currentToken();
    if (t.type == "Идентификатор") {
        if (!isDeclared(t.value)) error("Семантическая ошибка: некомпилируемая переменная '" + t.value + "' не объявлена");
        pos++;
    }
    else if (t.type == "Число") {
        pos++;
    }
    else {
        error("Ожидалось число или переменная");
        if (pos < tokens.size()) pos++;
    }
}

void P() { 
    if (currentToken().type == "Операторы действий") {
        pos++;
    }
    else {
        error("Ожидался знак операции (+, -, *, /)");
        if (pos < tokens.size()) pos++;
    }
}

void G() { 
    if (currentToken().value == "(") {
        match("(");
        H();
        match(")");
    }
    else O();

    while (currentToken().type == "Операторы действий") {
        P();
        if (currentToken().value == "(") {
            match("("); H(); match(")");
        }
        else O();
    }
}

void H() { 
    if (currentToken().value == "-") match("-");
    G();
}

void F() { 
    if (currentToken().type == "Идентификатор") {
        std::string name = currentToken().value;
        if (!isDeclared(name)) error("Семантическая ошибка: переменная '" + name + "' не объявлена");
        pos++;

        if (currentToken().value == "=") {
            match("=");
            H();
        }
        else {
            error("Ожидался знак присваивания '='");
            
            while (pos < tokens.size() && currentToken().type != "Идентификатор" && currentToken().value != "End") pos++;
        }
    }
    else {
        error("Ожидалось имя переменной в начале выражения");
        while (pos < tokens.size() && currentToken().type != "Идентификатор" && currentToken().value != "End") pos++;
    }
}

void C() { 
    if (currentToken().type == "Идентификатор") {
        idents.push_back({ currentToken().value, 0 });
        pos++;
        while (currentToken().value == ",") {
            pos++;
            if (currentToken().type == "Идентификатор") {
                idents.push_back({ currentToken().value, 0 });
                pos++;
            }
            else {
                error("Ожидалось имя переменной после ','");
                if (pos < tokens.size()) pos++;
            }
        }
    }
    else {
        error("Ожидалось имя переменной после 'Var'");
        while (pos < tokens.size() && currentToken().value != "Begin" && currentToken().type != "Идентификатор") pos++;
    }
}

void A() { 
    if (currentToken().value == "Var") {
        match("Var");
        C();
    }
    else {
        error("Ожидалось ключевое слово 'Var'");
    }
}

void B() { 
    if (currentToken().value == "Begin") {
        match("Begin");
        while (currentToken().value != "End" && currentToken().value != "EOF") {
            F();
        }
        match("End");
    }
    else {
        error("Ожидалось ключевое слово 'Begin'");
    }
}

void S() { 
    std::cout << "\n=== ЗАПУСК СИНТАКСИЧЕСКОГО АНАЛИЗА ===" << std::endl;
    A();
    B();
    std::cout << "=== АНАЛИЗ ЗАВЕРШЕН ===" << std::endl;
    std::cout << "Всего ошибок найдено: " << errorCount << std::endl;
}

int main() {
    setlocale(LC_ALL, "Russian");

    
    std::string filename = "program_errors.txt";

    readFile(filename);

    if (tokens.empty()) {
        std::cout << "\n[ВНИМАНИЕ] Список токенов пуст. Программа завершена." << std::endl;
        std::cout << "Нажмите Enter для выхода...";
        std::cin.get();
        return 1;
    }

    std::cout << "СТРОКА\tЛЕКСЕМА\t\tТИП" << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;
    for (int i = 0; i < tokens.size(); i++) {
        std::cout << tokens[i].line << "\t" << tokens[i].value << "\t\t" << tokens[i].type << std::endl;
    }
    std::cout << "--------------------------------------------------------" << std::endl;

    S();

    std::cout << "\nНажмите Enter для выхода...";
    std::cin.get();
    return 0;
}