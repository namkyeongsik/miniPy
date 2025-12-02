#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp.h"

typedef struct Value {
    int is_ptr;
    long num;
    struct Value* ref; // pointer target if is_ptr
} Value;

static Value* value_int(long n) {
    Value* v = (Value*)malloc(sizeof(Value));
    if (!v) return NULL;
    v->is_ptr = 0;
    v->num = n;
    v->ref = NULL;
    return v;
}

static Value* value_ptr(Value* target) {
    Value* v = (Value*)malloc(sizeof(Value));
    if (!v) return NULL;
    v->is_ptr = 1;
    v->num = 0;
    v->ref = target;
    return v;
}

/* --- 환경(블록 스코프) --------------------------------------------- */

typedef struct {
    char* names[256];
    Value* vals[256];
    int count;
} Frame;

static Frame frames[64];
static int frame_top = 0;

typedef struct {
    char* name;
    AST* params; // AST_STMT_LIST of AST_VAR
    AST* body;   // suite AST
} FuncDef;

static FuncDef funcs[128];
static int func_count = 0;

static void push_frame(void) {
    if (frame_top < 64) {
        frames[frame_top].count = 0;
        frame_top++;
    }
}

static void pop_frame(void) {
    if (frame_top > 0) {
        Frame* f = &frames[frame_top - 1];
        for (int i = 0; i < f->count; ++i) {
            free(f->names[i]);
            free(f->vals[i]);
        }
        frame_top--;
    }
}

static Value* lookup(const char* name) {
    for (int i = frame_top - 1; i >= 0; --i) {
        Frame* f = &frames[i];
        for (int j = 0; j < f->count; ++j) {
            if (strcmp(f->names[j], name) == 0) {
                return f->vals[j];
            }
        }
    }
    return NULL;
}

static Value* assign_var(const char* name, Value* v) {
    for (int i = frame_top - 1; i >= 0; --i) {
        Frame* f = &frames[i];
        for (int j = 0; j < f->count; ++j) {
            if (strcmp(f->names[j], name) == 0) {
                f->vals[j]->is_ptr = v->is_ptr;
                f->vals[j]->num    = v->num;
                f->vals[j]->ref    = v->ref;
                return f->vals[j];
            }
        }
    }

    if (frame_top == 0) return NULL;
    Frame* f = &frames[frame_top - 1];
    if (f->count >= 256) return NULL;
    f->names[f->count] = strdup(name);
    f->vals[f->count]  = value_int(0);
    Value* slot = f->vals[f->count];
    slot->is_ptr = v->is_ptr;
    slot->num = v->num;
    slot->ref = v->ref;
    f->count++;
    return slot;
}

/* --- 실행 결과 플래그 ---------------------------------------------- */

typedef enum { RES_NORMAL, RES_BREAK, RES_CONTINUE, RES_RETURN } ResType;

typedef struct {
    ResType type;
    Value* val;
} ExecResult;

static ExecResult res_normal(void) { return (ExecResult){RES_NORMAL, NULL}; }
static ExecResult res_break(void)  { return (ExecResult){RES_BREAK,  NULL}; }
static ExecResult res_continue(void){ return (ExecResult){RES_CONTINUE,NULL}; }
static ExecResult res_return(Value* v){ return (ExecResult){RES_RETURN, v}; }

/* --- 평가 함수 ----------------------------------------------------- */

static Value* eval_expr(AST* node);
static ExecResult exec_stmt(AST* node);
static ExecResult exec_func(FuncDef* f, AST* args);

static ExecResult exec_func(FuncDef* f, AST* args) {
    push_frame();

    AST* p = f->params;
    AST* a = args;
    while (p || a) {
        if (!p) break;

        char* pname = NULL;
        AST* pnext = NULL;
        if (p->type == AST_VAR) {
            pname = p->name;
            pnext = NULL;
        } else if (p->type == AST_STMT_LIST && p->left && p->left->type == AST_VAR) {
            pname = p->left->name;
            pnext = p->right;
        }

        AST* arg_expr = NULL;
        AST* anext = NULL;
        if (a) {
            if (a->type == AST_STMT_LIST) {
                arg_expr = a->left;
                anext = a->right;
            } else {
                arg_expr = a;
                anext = NULL;
            }
        }

        if (pname) {
            Value* v = arg_expr ? eval_expr(arg_expr) : value_int(0);
            assign_var(pname, v);
        }

        p = pnext;
        a = anext;
    }

    ExecResult r = exec_stmt(f->body);
    pop_frame();
    return r;
}

static Value* eval_expr(AST* node) {
    if (!node) return value_int(0);

    switch (node->type) {
    case AST_INT:
        return value_int(node->value);

    case AST_VAR: {
        Value* v = lookup(node->name);
        if (!v) {
            fprintf(stderr, "NameError: %s not defined\n", node->name);
            return value_int(0);
        }
        if (v->is_ptr) {
            Value* p = value_ptr(v->ref);
            if (p) p->num = v->num;
            return p ? p : value_int(0);
        }
        return value_int(v->num);
    }

    case AST_ADDR: {
        if (!node->left || node->left->type != AST_VAR) {
            fprintf(stderr, "address-of requires variable\n");
            return value_int(0);
        }
        Value* target = lookup(node->left->name);
        if (!target) target = assign_var(node->left->name, value_int(0));
        return value_ptr(target);
    }

    case AST_DEREF: {
        Value* ptr = eval_expr(node->left);
        if (!ptr->is_ptr || !ptr->ref) {
            fprintf(stderr, "TypeError: cannot dereference non-pointer\n");
            return value_int(0);
        }
        return value_int(ptr->ref->num);
    }

    case AST_ADD: case AST_SUB: case AST_MUL: case AST_DIV:
    case AST_LT: case AST_GT: case AST_LE: case AST_GE:
    case AST_EQ: case AST_NE: case AST_MOD: {
        Value* l = eval_expr(node->left);
        Value* r = eval_expr(node->right);
        long a = l ? l->num : 0;
        long b = r ? r->num : 0;
        switch (node->type) {
        case AST_ADD: return value_int(a + b);
        case AST_SUB: return value_int(a - b);
        case AST_MUL: return value_int(a * b);
        case AST_DIV: return value_int(b ? a / b : 0);
        case AST_MOD: return value_int(b ? a % b : 0);
        case AST_LT:  return value_int(a < b);
        case AST_GT:  return value_int(a > b);
        case AST_LE:  return value_int(a <= b);
        case AST_GE:  return value_int(a >= b);
        case AST_EQ:  return value_int(a == b);
        case AST_NE:  return value_int(a != b);
        default: break;
        }
        return value_int(0);
    }

    case AST_AND: {
        Value* l = eval_expr(node->left);
        if (l->num == 0) return value_int(0);
        Value* r = eval_expr(node->right);
        return value_int(r->num != 0);
    }

    case AST_OR: {
        Value* l = eval_expr(node->left);
        if (l->num != 0) return value_int(1);
        Value* r = eval_expr(node->right);
        return value_int(r->num != 0);
    }

    case AST_NOT: {
        Value* v = eval_expr(node->left);
        return value_int(v->num == 0);
    }

    case AST_ASSIGN: {
        Value* rhs = eval_expr(node->right);
        if (node->left->type == AST_VAR) {
            assign_var(node->left->name, rhs);
            return value_int(rhs->num);
        } else if (node->left->type == AST_DEREF) {
            Value* ptr = eval_expr(node->left->left);
            if (!ptr->is_ptr || !ptr->ref) {
                fprintf(stderr, "TypeError: cannot assign through non-pointer\n");
                return value_int(0);
            }
            ptr->ref->num = rhs->num;
            return value_int(rhs->num);
        } else {
            fprintf(stderr, "invalid assignment target\n");
            return value_int(0);
        }
    }

    case AST_PRINTF: {
        Value* v = eval_expr(node->left);
        printf("%ld\n", v->num);
        return value_int(v->num);
    }

    case AST_SCANF: {
        long n = 0;
        if (scanf("%ld", &n) != 1) n = 0;
        assign_var(node->name, value_int(n));
        return value_int(n);
    }

    case AST_CALL: {
        FuncDef* f = NULL;
        for (int i = 0; i < func_count; ++i) {
            if (strcmp(funcs[i].name, node->name) == 0) {
                f = &funcs[i];
                break;
            }
        }
        if (!f) {
            fprintf(stderr, "NameError: function %s not defined\n", node->name);
            return value_int(0);
        }
        ExecResult r = exec_func(f, node->left);
        if (r.type == RES_RETURN && r.val) return r.val;
        return value_int(0);
    }

    default:
        return value_int(0);
    }
}

static ExecResult exec_stmt(AST* node) {
    if (!node) return res_normal();

    switch (node->type) {
    case AST_FUNC_DEF:
        if (func_count >= 128) {
            fprintf(stderr, "function table overflow\n");
            return res_normal();
        }
        funcs[func_count].name   = node->name;
        funcs[func_count].params = node->left;
        funcs[func_count].body   = node->right;
        func_count++;
        return res_normal();

    case AST_STMT_LIST:
        {
            ExecResult r = exec_stmt(node->left);
            if (r.type != RES_NORMAL) return r;
            return exec_stmt(node->right);
        }

    case AST_BLOCK:
        push_frame();
        {
            ExecResult r = exec_stmt(node->left);
            pop_frame();
            return r;
        }

    case AST_VAR_DECL: {
        if (node->left) {
            Value* v = eval_expr(node->left);
            assign_var(node->name, v);
        } else {
            assign_var(node->name, value_int(0));
        }
        return res_normal();
    }

    case AST_ASSIGN:
    case AST_PRINTF:
    case AST_SCANF:
        eval_expr(node);
        return res_normal();

    case AST_IF: {
        Value* cond = eval_expr(node->left);
        if (cond->num) {
            return exec_stmt(node->right);
        } else {
            return exec_stmt(node->third);
        }
    }

    case AST_WHILE: {
        while (1) {
            Value* cond = eval_expr(node->left);
            if (!cond->num) break;
            ExecResult r = exec_stmt(node->right);
            if (r.type == RES_BREAK) break;
            if (r.type == RES_CONTINUE) {
                /* fall through to step */
            } else if (r.type == RES_RETURN) {
                return r;
            }
            if (node->third) {
                ExecResult s = exec_stmt(node->third);
                if (s.type == RES_RETURN) return s;
            }
        }
        return res_normal();
    }

    case AST_BREAK:
        return res_break();
    case AST_CONTINUE:
        return res_continue();
    case AST_RETURN:
        return res_return(node->left ? eval_expr(node->left) : value_int(0));

    default:
        eval_expr(node);
        return res_normal();
    }
}

int interpret(AST* root) {
    func_count = 0;
    push_frame();
    ExecResult r = exec_stmt(root);
    int exit_code = 0;
    if (r.type == RES_RETURN && r.val) exit_code = (int)r.val->num;
    pop_frame();
    return exit_code;
}
