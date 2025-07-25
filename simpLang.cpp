#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
#include <unordered_map>
using namespace std;

// Token types
enum class TokenType {
    VAR, PRINT, IDENT, INT, STRING, SEMI, PLUS, MINUS, STAR, SLASH,
    LPAREN, RPAREN, EQUAL, END
};

// Token structure
struct Token {
    TokenType type;
    string value;
    Token(TokenType t, string v = "") : type(t), value(v) {}
};

// Lexer with comment handling
class Lexer {
    string source;
    size_t pos = 0;
    char current() { return (pos < source.size()) ? source[pos] : '\0'; }
    void advance() { pos++; }

public:
    Lexer(const string& src) : source(src) {}

    Token nextToken() {
        while (isspace(current())) advance();

        // Handle single-line comments
        if (current() == '/' && pos+1 < source.size() && source[pos+1] == '/') {
            advance(); advance();  // Skip '//'
            while (current() != '\n' && current() != '\0') advance();
            return nextToken();
        }

        if (current() == '\0') return Token(TokenType::END);

        // Identifiers/keywords
        if (isalpha(current())) {
            string ident;
            while (isalnum(current())) {
                ident += current();
                advance();
            }
            if (ident == "var") return Token(TokenType::VAR);
            if (ident == "print") return Token(TokenType::PRINT);
            return Token(TokenType::IDENT, ident);
        }

        // Integers
        if (isdigit(current())) {
            string num;
            while (isdigit(current())) {
                num += current();
                advance();
            }
            return Token(TokenType::INT, num);
        }

        // Strings
        if (current() == '"') {
            advance();
            string str;
            while (current() != '"' && current() != '\0') {
                str += current();
                advance();
            }
            if (current() == '"') advance();
            return Token(TokenType::STRING, str);
        }

        // Operators/symbols
        switch (current()) {
            case ';': advance(); return Token(TokenType::SEMI);
            case '=': advance(); return Token(TokenType::EQUAL);
            case '+': advance(); return Token(TokenType::PLUS);
            case '-': advance(); return Token(TokenType::MINUS);
            case '*': advance(); return Token(TokenType::STAR);
            case '/': advance(); return Token(TokenType::SLASH);
            case '(': advance(); return Token(TokenType::LPAREN);
            case ')': advance(); return Token(TokenType::RPAREN);
            default: throw runtime_error("Unexpected character: " + string(1, current()));
        }
    }
};

// Compiler
class Compiler {
    vector<Token> tokens;
    size_t pos = 0;
    Token current() { return (pos < tokens.size()) ? tokens[pos] : Token(TokenType::END); }
    void advance() { pos++; }

    string parseVar() {
        advance(); // Skip 'var'
        string name = current().value;
        advance(); // Skip identifier
        advance(); // Skip '='
        
        if (current().type == TokenType::INT) {
            string value = current().value;
            advance();
            return "int " + name + " = " + value + ";";
        }
        else if (current().type == TokenType::STRING) {
            string value = "\"" + current().value + "\"";
            advance();
            return "string " + name + " = " + value + ";";
        }
        throw runtime_error("Invalid variable initialization");
    }

    string parsePrint() {
        advance(); // Skip 'print'
        ostringstream expr;
        while (current().type != TokenType::SEMI) {
            if (current().type == TokenType::IDENT) expr << current().value;
            else if (current().type == TokenType::PLUS) expr << " + ";
            else if (current().type == TokenType::INT) expr << current().value;
            else if (current().type == TokenType::STRING) expr << "\"" << current().value << "\"";
            else if (current().type == TokenType::LPAREN) expr << "(";
            else if (current().type == TokenType::RPAREN) expr << ")";
            else if (current().type == TokenType::STAR) expr << "*";
            else if (current().type == TokenType::SLASH) expr << "/";
            else if (current().type == TokenType::MINUS) expr << "-";
            advance();
        }
        advance(); // Skip ';'
        return "cout << " + expr.str() + " << endl;";
    }

public:
    Compiler(const vector<Token>& t) : tokens(t) {}

    string compile() {
        ostringstream cpp;
        cpp << "#include <iostream>\n#include <string>\nusing namespace std;\n\nint main() {\n";
        
        while (current().type != TokenType::END) {
            if (current().type == TokenType::VAR) cpp << "    " << parseVar() << "\n";
            else if (current().type == TokenType::PRINT) cpp << "    " << parsePrint() << "\n";
            else advance();
        }
        
        cpp << "    return 0;\n}\n";
        return cpp.str();
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <source.sl>" << endl;
        return 1;
    }

    ifstream file(argv[1]);
    if (!file) {
        cerr << "Error opening file: " << argv[1] << endl;
        return 1;
    }
    stringstream buffer;
    buffer << file.rdbuf();
    string source = buffer.str();

    Lexer lexer(source);
    vector<Token> tokens;
    try {
        Token token = lexer.nextToken();
        while (token.type != TokenType::END) {
            tokens.push_back(token);
            token = lexer.nextToken();
        }
    } catch (const exception& e) {
        cerr << "Lexer Error: " << e.what() << endl;
        return 1;
    }

    Compiler compiler(tokens);
    string cppCode = compiler.compile();

    ofstream out("output.cpp");
    out << cppCode;
    out.close();

    system("g++ output.cpp -o output");
    cout << "Compilation successful. Run './output' to execute." << endl;
    return 0;
}
