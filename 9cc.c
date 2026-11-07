#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    TK_RESERVED, // 記号
    TK_NUM,      // 数字
    TK_EOF,     // 終端
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int value;
    char *str;
};

// 現在のトークン
Token *cur_token;

// エラーメッセージを出力する
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが期待している記号のときには、トークンを１つ読み進めて、真を返す。それ以外の場合には偽を返す。
bool consume(char op) {
    if (cur_token->kind != TK_RESERVED || cur_token->str[0] != op) {
        return false;
    }
    cur_token = cur_token->next;
    return true;
}

// 次のトークンが期待している記号のときには、トークンを１つ読み進める。それ以外の場合にはエラーを報告する。
void expect(char op) {
    if (cur_token->kind != TK_RESERVED || cur_token->str[0] != op) {
        error("expected '%c'", op);
    }
    cur_token = cur_token->next;
}

// 次のトークンが数字の場合、トークンを１つ読み進めて、その数値を返す。それ以外の場合にはエラーを報告する。
int expect_number() {
    if (cur_token->kind != TK_NUM) {
        error("expected number");
    }
    int val = cur_token->value;
    cur_token = cur_token->next;
    return val;
}

bool at_eof() {
    return cur_token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        // 空白文字をスキップする
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->value = strtol(p, &p, 10);
            continue;
        }

        error("invalid character: '%c'", *p);
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input>\n", argv[0]);
        return 1;
    }

    cur_token = tokenize(argv[1]);

    // アセンブリの前半部分を出力する
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 式の最初は数でなければならないので、それをチェックして、最初のmov命令を出力する
    printf("    mov rax, %d\n", expect_number());

    // `+ <数>`あるいは`- <数>`の形のトークンが続く限り、アセンブリを出力する
    while(!at_eof()) {
        if (consume('+')) {
            printf("    add rax, %d\n", expect_number());
        }

        expect('-');
        printf("    sub rax, %d\n", expect_number());
    }

    printf("    ret\n");
    return 0;
}