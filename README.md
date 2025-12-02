# miniPy

## 의존성
- gcc, flex, bison

## 빌드
```
make
```
빌드 결과 실행 파일: `minipy`

## 실행 방법
- 파일 실행: `./minipy path/to/code.py`
- 표준 입력/대화형: 코드 입력 후 EOF(터미널에서 Ctrl+D)로 종료
```
./minipy <<'EOF'
x = 2
y = 3
print(x + y)
EOF
```
- 입력 사용 예:
```
printf '5\n' | ./minipy - <<'EOF'
n = input()
print(n)
EOF
```

## 언어 개요
- 파이썬식 들여쓰기(NEWLINE/INDENT/DEDENT), 동적 정수 타입
- 산술 `+ - * / %`, 비교 `< > <= >= == !=`, 논리 `&& || !`
- 제어: `if / if-else`, `while`, `for x in range(start, end[, step])`, `break`, `continue`, `return`
- I/O: `print(expr)`, `input()` (대입과 함께 사용)
- 포인터: `&name`, `*expr`, `*expr = expr` (셀 핸들 방식)

## 테스트(예제 코드)
- 개별 실행: `./minipy tests/test01_assign.py` 등
- 전체 실행: `./run_tests.sh` (빌드 후)
  - 주요 기대 출력:  
    - test01_assign → 3  
    - test02_logic_if → 1  
    - test03_if_else → 1  
    - test04_while_sum → 12  
    - test05_for_range → 10  
    - test06_for_step → 9  
    - test07_break → 3  
    - test08_continue_for → 9  
    - test09_pointer → 15, 15  
    - test10_pointer_chain → 6, 5, 6, 5

## 플랫폼 메모
- macOS(Apple clang, flex, bison) 환경에서 작성·빌드 확인.  
- 리눅스도 동일 의존성(gcc/clang, flex, bison)만 갖추면 동작.  
- Windows는 WSL/MSYS2 등 유닉스 호환 환경에서 빌드 권장.

## 문서
- `doc/overview.md`: 언어 설계 개요, 문법 요약, 구조, 구현/제한 사항
