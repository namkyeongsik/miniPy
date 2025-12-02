#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

int yyparse(void);
extern FILE* yyin;

int main(int argc, char** argv) {
    if (argc >= 2) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            perror("입력 파일을 열 수 없습니다");
            return EXIT_FAILURE;
        }
    } else {
        yyin = stdin;
    }

    int res = yyparse();
    if (yyin != stdin) fclose(yyin);
    return res ? EXIT_FAILURE : EXIT_SUCCESS;
}
