#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <vector>
#include <stack>

// ─────────────────────────────────────────────
//  Структуры данных
// ─────────────────────────────────────────────

struct Token {
    int         line;
    std::string value;
    std::string type;
};

struct Ident {
    std::string name;
};

std::vector<Token> tokens;
std::vector<Ident> idents;
int pos = 0;
int errorCount = 0;
std::ofstream outFile; // Поток для записи результата в файл

// ─────────────────────────────────────────────
//  Алгоритм ОПЗ (Обратная польская запись)
// ─────────────────────────────────────────────

// Функция для определения приоритета операции (взято из методички)
int priority(char op) {
    if (op == '(' || op == ')') return 0;
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '~') return 3; // Специальный высокий приоритет для УНАРНОГО минуса
    return -1;
}

// Построение постфиксной записи для выделенного участка токенов
std::vector<std::string> buildPostfix(int start_idx, int end_idx) {
    std::vector<std::string> postfix;
    std::stack<char> opstk;

    for (int i = start_idx; i < end_idx; i++) {
        Token symb = tokens[i];

        if (symb.type == "Число" || symb.type == "Идентификатор") {
            postfix.push_back(symb.value);
        }
        else if (symb.value == "(") {
            opstk.push('(');
        }
        else if (symb.value == ")") {
            while (!opstk.empty() && opstk.top() != '(') {
                // Заменяем внутренний маркер '~' обратно на '-' при выводе
                char op = opstk.top();
                postfix.push_back(op == '~' ? "-" : std::string(1, op));
                opstk.pop();
            }
            if (!opstk.empty()) opstk.pop(); // Выбрасываем '('
        }
        else if (symb.type == "Бин.оп.") {
            char currentOp = symb.value[0];

            // Математически точная обработка унарного минуса
            if (currentOp == '-') {
                bool isUnary = false;
                if (i == start_idx) isUnary = true;
                else if (tokens[i - 1].value == "(") isUnary = true;
                else if (tokens[i - 1].type == "Бин.оп.") isUnary = true;

                if (isUnary) {
                    postfix.push_back("0"); // Добавляем 0 для эмуляции (0 - x)
                    currentOp = '~';        // Меняем оператор на унарный (высший приоритет)
                }
            }

            // Выталкиваем из стека операции с бóльшим или равным приоритетом
            while (!opstk.empty() && opstk.top() != '(' &&
                priority(opstk.top()) >= priority(currentOp)) {
                char op = opstk.top();
                postfix.push_back(op == '~' ? "-" : std::string(1, op));
                opstk.pop();
            }
            opstk.push(currentOp);
        }
    }

    // Дописываем оставшиеся в стеке операции
    while (!opstk.empty()) {
        char op = opstk.top();
        postfix.push_back(op == '~' ? "-" : std::string(1, op));
        opstk.pop();
    }

    return postfix;
}

// ─────────────────────────────────────────────
//  Лексический и синтаксический анализатор
// ─────────────────────────────────────────────

Token currentToken() {
    if (pos < (int)tokens.size()) return tokens[pos];
    return { -1, "EOF", "Конец файла" };
}

void error(const std::string& msg) {
    errorCount++;
    std::cout << "[ОШИБКА] Строка " << currentToken().line
        << ": " << msg
        << " (найдено: '" << currentToken().value << "')" << std::endl;
}

bool match(const std::string& expected) {
    if (currentToken().value == expected) {
        pos++;
        return true;
    }
    error("Ожидалось '" + expected + "'");
    if (pos < (int)tokens.size()) pos++;
    return false;
}

bool isDeclared(const std::string& name) {
    for (const auto& id : idents)
        if (id.name == name) return true;
    return false;
}

void readFile(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cout << "[ОШИБКА] Не удалось открыть файл: " << fileName << std::endl;
        return;
    }

    char ch;
    int  line = 1;

    while (file.get(ch)) {
        if (ch == '\n') { line++; continue; }
        if (isspace(ch)) continue;

        if (isalpha(ch)) {
            std::string word;
            word += ch;
            while (file.get(ch) && isalnum(ch)) word += ch;
            if (!file.eof()) file.putback(ch);

            if (word == "end") word = "End";

            if (word == "Var" || word == "Begin" || word == "End")
                tokens.push_back({ line, word, "Ключевое слово" });
            else
                tokens.push_back({ line, word, "Идентификатор" });
        }
        else if (isdigit(ch)) {
            std::string num;
            num += ch;
            while (file.get(ch) && isdigit(ch)) num += ch;
            if (!file.eof()) file.putback(ch);
            tokens.push_back({ line, num, "Число" });
        }
        else if (ch == '+' || ch == '*' || ch == '/') {
            tokens.push_back({ line, std::string(1, ch), "Бин.оп." });
        }
        else if (ch == '-') {
            tokens.push_back({ line, "-", "Бин.оп." });
        }
        else if (ch == '=') {
            tokens.push_back({ line, "=", "Оператор присваивания" });
        }
        else if (ch == ',') {
            tokens.push_back({ line, ",", "Разделитель" });
        }
        else if (ch == '(' || ch == ')') {
            tokens.push_back({ line, std::string(1, ch), "Разделитель" });
        }
        else if (ch == '.') {
            tokens.push_back({ line, ".", "Конец программы" });
        }
        else {
            errorCount++;
            std::cout << "[ОШИБКА] Строка " << line << ": Неизвестный символ '" << ch << "'" << std::endl;
        }
    }
    file.close();
}

// Предварительные объявления рекурсивных функций
void S(); void A(); void C(); void C_prime();
void B(); void D(); void Z(); void E_expr();
void F(); void G(); void G_prime(); void H(); void O();

void O() {
    Token t = currentToken();
    if (t.type == "Идентификатор") {
        if (!isDeclared(t.value))
            error("Семантическая ошибка: переменная '" + t.value + "' не объявлена");
        pos++;
    }
    else if (t.type == "Число") {
        pos++;
    }
    else {
        error("Ожидался операнд (идентификатор или число)");
        if (pos < (int)tokens.size()) pos++;
    }
}

void H() {
    if (currentToken().type == "Бин.оп.") pos++;
    else error("Ожидался бинарный оператор (+, -, *, /)");
}

void G_prime() {
    if (currentToken().type == "Бин.оп.") {
        H();
        G();
        G_prime();
    }
}

void G() {
    if (currentToken().value == "(") {
        pos++;
        F();
        match(")");
        G_prime();
    }
    else {
        O();
        G_prime();
    }
}

void F() {
    if (currentToken().value == "-") {
        pos++; // Унарный минус
        G();
    }
    else {
        G();
    }
}

// Обработка присваивания и формирование постфиксной записи
void E_expr() {
    Token t = currentToken();

    if (t.type != "Идентификатор") {
        error("Ожидался идентификатор в левой части присваивания");
        while (pos < (int)tokens.size() && currentToken().type != "Идентификатор" && currentToken().value != "End") pos++;
        return;
    }

    if (!isDeclared(t.value))
        error("Семантическая ошибка: переменная '" + t.value + "' не объявлена");
    pos++;

    if (!match("=")) {
        while (pos < (int)tokens.size() && currentToken().type != "Идентификатор" && currentToken().value != "End") pos++;
        return;
    }

    int expr_start = pos;
    int errors_before = errorCount; // Фиксируем ошибки до начала выражения

    F(); // Парсим выражение

    int expr_end = pos;

    // Генерируем ОПЗ только если КОНКРЕТНО В ЭТОМ выражении не было синтаксических ошибок
    if (errorCount == errors_before) {
        std::vector<std::string> postfix = buildPostfix(expr_start, expr_end);

        // Вывод на экран
        std::cout << "Постфиксная запись для '" << t.value << "' : ";
        for (const auto& p : postfix) std::cout << p << " ";
        std::cout << std::endl;

        // Запись в файл (Требование 1.3)
        if (outFile.is_open()) {
            outFile << t.value << " = ";
            for (const auto& p : postfix) outFile << p << " ";
            outFile << "\n";
        }
    }
}

void Z() {
    if (currentToken().type == "Идентификатор") D();
}

void D() {
    E_expr();
    Z();
}

void B() {
    if (!match("Begin")) return;

    if (currentToken().type == "Идентификатор") {
        D();
    }
    else if (currentToken().value != "End") {
        error("Ожидался оператор присваивания или End");
    }

    match("End");
}

void C_prime() {
    if (currentToken().value == ",") {
        pos++;
        C();
    }
}

void C() {
    Token t = currentToken();
    if (t.type == "Идентификатор") {
        idents.push_back({ t.value });
        pos++;
        C_prime();
    }
    else {
        error("Ожидался идентификатор в списке переменных");
        while (pos < (int)tokens.size() && currentToken().type != "Идентификатор" && currentToken().value != "Begin") pos++;
        if (currentToken().type == "Идентификатор") C();
    }
}

void A() {
    if (match("Var")) C();
}

void S() {
    std::cout << "\n=== ЗАПУСК СИНТАКСИЧЕСКОГО АНАЛИЗА ===" << std::endl;
    A();
    B();
    match(".");
    std::cout << "=== АНАЛИЗ ЗАВЕРШЁН ===" << std::endl;
    std::cout << "Всего ошибок найдено: " << errorCount << std::endl;
}

int main() {
    setlocale(LC_ALL, "Russian");

    std::string fileName = "input.txt"; // Убедись, что файл с кодом называется так же

    // Открываем файл для записи результатов
    outFile.open("postfix_out.txt");
    if (!outFile.is_open()) {
        std::cout << "Предупреждение: Не удалось открыть/создать файл postfix_out.txt" << std::endl;
    }

    readFile(fileName);

    if (tokens.empty()) {
        std::cout << "[ВНИМАНИЕ] Список токенов пуст. Проверьте файл " << fileName << std::endl;
        return 1;
    }

    S();

    if (outFile.is_open()) {
        outFile.close();
        std::cout << "\n[УСПЕХ] Постфиксная запись сохранена в файл 'postfix_out.txt'." << std::endl;
    }

    return 0;
}