#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_TOKEN_LENGTH 100
#define NUM_KEYWORDS 20

// Tipos de Tokens
typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_KEYWORD,
    TOKEN_STRING,
    TOKEN_UNKNOWN,
    TOKEN_EOF
} TokenType;

// Estrutura de Token
typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LENGTH];
    int line;
} Token;

// Lista de Palavras Reservadas de PERL
const char *keywords[NUM_KEYWORDS] = {
    "my", "sub", "use", "strict", "if", "elsif", "else",
    "for", "foreach", "while", "do", "next", "last",
    "redo", "print", "return", "package", "require", "bless", "eval"
};

// Variáveis Globais
int current_line = 1;
Token current_token;

// Prototipação de Funções
int is_keyword(const char *word);
Token get_next_token(FILE *file);
void error(const char *message);
void parse_program(FILE *file);
void parse_statement(FILE *file);
void parse_variable_declaration(FILE *file);
void parse_print_statement(FILE *file);
void parse_assignment(FILE *file);
void parse_if_statement(FILE *file);

// Função para verificar se uma palavra é reservada
int is_keyword(const char *word) {
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Função para gerar erro
void error(const char *message) {
    printf("Erro de sintaxe na linha %d: %s\n", current_line, message);
    exit(1);
}

// Função para Obter Próximo Token
Token get_next_token(FILE *file) {
    Token token;
    int ch;

    // Ignorar espaços em branco e contagem de linhas
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') current_line++;
        if (!isspace(ch)) break;
    }

    // Verificar Fim do Arquivo
    if (ch == EOF) {
        token.type = TOKEN_EOF;
        strcpy(token.value, "EOF");
        token.line = current_line;
        return token;
    }

    // Identificar Números
    if (isdigit(ch)) {
        int i = 0;
        token.value[i++] = ch;
        while ((ch = fgetc(file)) != EOF && isdigit(ch)) {
            token.value[i++] = ch;
        }
        token.value[i] = '\0';
        if (ch != EOF) ungetc(ch, file);
        token.type = TOKEN_NUMBER;
        token.line = current_line;
        return token;
    }

    // Identificar Variáveis (Iniciam com $)
    if (ch == '$') {
        int i = 0;
        token.value[i++] = ch;
        while ((ch = fgetc(file)) != EOF && (isalnum(ch) || ch == '_')) {
            token.value[i++] = ch;
        }
        token.value[i] = '\0';
        if (ch != EOF) ungetc(ch, file);
        token.type = TOKEN_IDENTIFIER;
        token.line = current_line;
        return token;
    }

    // Identificar Identificadores e Palavras Reservadas
    if (isalpha(ch) || ch == '_') {
        int i = 0;
        token.value[i++] = ch;
        while ((ch = fgetc(file)) != EOF && (isalnum(ch) || ch == '_')) {
            token.value[i++] = ch;
        }
        token.value[i] = '\0';
        if (ch != EOF) ungetc(ch, file);

        if (is_keyword(token.value)) {
            token.type = TOKEN_KEYWORD;
        } else {
            token.type = TOKEN_IDENTIFIER;
        }
        token.line = current_line;
        return token;
    }

    // Identificar Strings
    if (ch == '"') {
        int i = 0;
        while ((ch = fgetc(file)) != EOF && ch != '"') {
            token.value[i++] = ch;
        }
        token.value[i] = '\0';
        if (ch == EOF) {
            token.type = TOKEN_UNKNOWN;
        } else {
            token.type = TOKEN_STRING;
        }
        token.line = current_line;
        return token;
    }

    // Identificar Operadores Simples
    if (strchr("+-*/=;(){}", ch)) {
        token.type = TOKEN_OPERATOR;
        token.value[0] = ch;
        token.value[1] = '\0';
        token.line = current_line;
        return token;
    }

    // Qualquer outra coisa é um token desconhecido
    token.type = TOKEN_UNKNOWN;
    token.value[0] = ch;
    token.value[1] = '\0';
    token.line = current_line;
    return token;
}

// Função para Analisar um Programa
void parse_program(FILE *file) {
    current_token = get_next_token(file);
    while (current_token.type != TOKEN_EOF) {
        parse_statement(file);
    }
}

// Função para Analisar uma Declaração
void parse_statement(FILE *file) {
    if (strcmp(current_token.value, "my") == 0) {
        parse_variable_declaration(file);
    } else if (strcmp(current_token.value, "print") == 0) {
        parse_print_statement(file);
    } else if (strcmp(current_token.value, "if") == 0) {
        parse_if_statement(file);
    } else if (current_token.type == TOKEN_IDENTIFIER) {
        parse_assignment(file);
    } else {
        error("Declaração inválida");
    }
}

// Função para Analisar Declarações de Variáveis
void parse_variable_declaration(FILE *file) {
    current_token = get_next_token(file); // Espera um identificador após 'my'
    if (current_token.type != TOKEN_IDENTIFIER) {
        error("Esperado um identificador após 'my'");
    }

    current_token = get_next_token(file); // Espera '='
    if (strcmp(current_token.value, "=") != 0) {
        error("Esperado '=' após o identificador");
    }

    current_token = get_next_token(file); // Espera um número
    if (current_token.type != TOKEN_NUMBER) {
        error("Esperado um número após '='");
    }

    current_token = get_next_token(file); // Espera ';'
    if (strcmp(current_token.value, ";") != 0) {
        error("Esperado ';' no final da declaração");
    }

    printf("Declaração de variável reconhecida.\n");
    current_token = get_next_token(file);  // Atualização do token após processar a declaração
}

// Função para Analisar Comandos 'print'
void parse_print_statement(FILE *file) {
    current_token = get_next_token(file); // Espera uma string
    if (current_token.type != TOKEN_STRING) {
        error("Esperado uma string após 'print'");
    }

    current_token = get_next_token(file); // Espera ';'
    if (strcmp(current_token.value, ";") != 0) {
        error("Esperado ';' no final do comando 'print'");
    }

    printf("Comando 'print' reconhecido.\n");
    current_token = get_next_token(file);
}

// Função para Analisar Atribuições
void parse_assignment(FILE *file) {
    current_token = get_next_token(file); // Espera '='
    if (strcmp(current_token.value, "=") != 0) {
        error("Esperado '=' após o identificador");
    }

    current_token = get_next_token(file); // Espera número ou identificador
    if (current_token.type != TOKEN_NUMBER && current_token.type != TOKEN_IDENTIFIER) {
        error("Esperado um número ou identificador após '='");
    }

    current_token = get_next_token(file); // Espera ';'
    if (strcmp(current_token.value, ";") != 0) {
        error("Esperado ';' no final da atribuição");
    }

    printf("Atribuição reconhecida.\n");
    current_token = get_next_token(file);
}

// Função para Analisar Estruturas 'if' e 'else'
void parse_if_statement(FILE *file) {
    current_token = get_next_token(file); // Espera '('
    if (strcmp(current_token.value, "(") != 0) {
        error("Esperado '(' após 'if'");
    }

    current_token = get_next_token(file); // Espera condição
    if (current_token.type != TOKEN_IDENTIFIER && current_token.type != TOKEN_NUMBER) {
        error("Esperado uma condição válida após '('");
    }

    current_token = get_next_token(file); // Espera ')'
    if (strcmp(current_token.value, ")") != 0) {
        error("Esperado ')' após condição");
    }

    current_token = get_next_token(file); // Espera '{'
    if (strcmp(current_token.value, "{") != 0) {
        error("Esperado '{' após ')'");
    }

    // Processar o bloco do 'if'
    while (strcmp(current_token.value, "}") != 0) {
        parse_statement(file);
    }

    current_token = get_next_token(file); // Atualização após '}'

    // Verifica 'else'
    if (strcmp(current_token.value, "else") == 0) {
        current_token = get_next_token(file); // Espera '{'
        if (strcmp(current_token.value, "{") != 0) {
            error("Esperado '{' após 'else'");
        }

        // Processar o bloco do 'else'
        while (strcmp(current_token.value, "}") != 0) {
            parse_statement(file);
        }

        current_token = get_next_token(file); // Atualização após '}'
    }
}

int main() {
    FILE *file = fopen("input.pl", "r");
    if (!file) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    parse_program(file);

    fclose(file);
    return 0;
}
