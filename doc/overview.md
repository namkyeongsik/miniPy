# miniPy 설계 개요

## 언어 개요 및 설계 의도
- C 베이스 miniC를 파이썬풍 문법(들여쓰기 기반, 세미콜론/중괄호 없음)으로 단순화하며, 포인터 개념을 추가한 언어.
- 동적 타입 정수 모델: 모든 값은 정수(long) 한 종류로 취급.
- 실행 방식: 코드 생성 없이 AST를 바로 해석하는 인터프리터(`src/interp.c`).
- 파이썬이 기본적으로 제공하지 않는 **포인터 개념**을 C식 `&`/`*` 표기로 실험해 보기 위해 “셀 핸들” 방식으로 단순화해 도입(실제 주소 대신 값 슬롯 참조).


## 문법(Grammar) 요약
EBNF 스타일 개요:
```
program   = stmt_list ;
stmt_list = stmt { stmt } ;
stmt      = simple_stmt NEWLINE | compound_stmt | NEWLINE ;

simple_stmt =
    IDENT "=" expr
  | "*" expr "=" expr
  | PRINT_KW "(" expr ")"
  | IDENT "=" INPUT_KW "(" ")"
  | BREAK_KW
  | CONTINUE_KW
  | RETURN_KW [expr]
  | expr ;

compound_stmt =
    IF_KW expr ":" suite
  | IF_KW expr ":" suite ELSE_KW ":" suite
  | WHILE_KW expr ":" suite
  | FOR_KW IDENT IN_KW RANGE_KW "(" expr "," expr ["," expr] ")" ":" suite ;

suite     = simple_stmt NEWLINE | NEWLINE INDENT stmt_list DEDENT ;

expr      =
    NUMBER | IDENT | "&" IDENT | "*" expr
  | expr ("+"|"-"|"*"|"/"|"%") expr
  | expr ("<"|">"|"<="|">="|"=="|"!=") expr
  | expr ("&&"|"||") expr
  | "!" expr
  | "(" expr ")" ;
```

토큰/렉싱:
- 키워드: `print`, `input`, `if`, `else`, `while`, `for`, `in`, `range`, `return`, `break`, `continue`
- 연산자: `+ - * / % < > <= >= == != && || ! &`
- 식별자/숫자, 주석 `# ...`
- 줄 단위 들여쓰기 계산으로 `NEWLINE`, `INDENT`, `DEDENT`를 생성하여 블록 구조 표현.

## 전체 구조 및 흐름
1) **lexer** (`parser/scanner.l`): 토큰화 + 들여쓰기 스택으로 INDENT/DEDENT 생성, 공백/주석 스킵.  
2) **parser** (`parser/parser.y`): Bison으로 AST 생성. `program` 액션에서 `interpret` 호출.  
3) **AST** (`include/ast.h`, `src/ast.c`): 이진 트리 + third 포인터로 조건/반복/블록/포인터/논리/return/break/continue 표현.  
4) **인터프리터** (`src/interp.c`): 블록 스코프 프레임 + 동적 값(Value)으로 AST 실행. 포인터는 Value의 ref 필드를 통해 역참조/대입.  
5) **드라이버/빌드** (`src/main.c`, `Makefile`): 입력 파일/STDIN → `yyparse()` → `interpret(AST)`. 코드 생성 단계 없음.

## 구현된 기능
- 파이썬식 들여쓰기 문법, 동적 정수 타입.
- 산술/비교/논리 연산: `+ - * / %`, `< > <= >= == !=`, `&& || !`.
- 제어: `if / if-else`, `while`, `for x in range(start, end[, step])`, `break`, `continue`, `return`.
- I/O: `print(expr)`, `input()` (대입과 함께 사용).
- 포인터: `&name`, `*expr`, `*expr = expr` (셀 핸들 모델).
- 블록 스코프 및 섀도잉.
- 테스트 스크립트 10개(`tests/`)와 통합 실행 스크립트 `run_tests.sh`.

## 미구현/제한 사항
- 함수/프로시저 정의·호출 미지원.
- 타입은 정수 한 종류만, 문자열/리스트 등 복합 타입 없음.
- 포인터는 단순 참조 핸들로만 동작(실제 주소 아님), 포인터 연산 제한적.
- 에러 메시지: 파서 기본 `parse error` 수준, 소스 위치/친절한 진단 부족.
- Bison 문법에 shift/reduce 경고 다수(우선순위/결합 정의 단순).  
- 테스트 기대값 기준: `test10_pointer_chain` 출력이 설계상 `6 5 6 5`로 제한적.
- 최적화/코드 생성 단계 없음(IR/타깃 코드 미지원).
