#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdio.h>
#include <stdlib.h>
#include "Tree.h"

// 两个基本类型
#define INT_TYPE 0
#define FLOAT_TYPE 1

// 符号表大小
#define TABLE_SIZE 1024

// 命名别名以及构建指针
typedef struct _Type * Type;
typedef struct _FieldList * FieldList;
typedef struct _Structure * Structure;
typedef struct _Function * Function;
typedef struct _Entry * Entry;

typedef enum {
    BASIC,      // int float
    ARRAY,      // 数组
    STRUCT,     // struct 
    STRUCT_DEF, // 定义 struct
    FUNC        // 函数    
} __Kind; // 种属

typedef enum {
    VAR,        // 变量
    FIELD       // 作用域
} __IdType;

// 结构体域链表节点
struct _FieldList {
    // 域的名称，或者说结构体成员的名称
    char name[32];
    // 类型
    Type type;
    // 指向下一个域的指针
    FieldList next;
};

// 结构体类型
struct _Structure {
    // 结构体名称
    char name[32];
    // 结构体域链表表头
    FieldList head;
};

// 函数类型
struct _Function {
    // 函数名称
    char name[32];
    // 返回值类型
    Type returnType;
    // 参数个数
    int parmNum;
    // 参数链表头节点指针
    FieldList head;
    // ?
    // 是否已经定义
    int hasDefined;
    // 所在行数
    int lineno;
};

struct _Type {
    // 种属
    __Kind kind;
    // 根据 type 选择子成员
    union {
        // int = INT_TYPE 0,  float = FLOAT_TYPE 1
        int basic;
        // 数组类型包括元素类型与数组大小
        struct {
            Type elem;
            int size;
        } array;
        // 结构体类型是一个域链表，根据结构体成员决定
        // 比如说                                   
        // struct a {int x; float y;}
        // 那么就是指向 int 的域连接指向 float 的域
        Structure structure;
        // 函数
        Function func;
    };  
};

// 符号表条目类型
struct _Entry {
    // 该终结符号名称 ???????
    char name[32];
    // 条目类型
    Type type;
    // 指向同一槽位的下一个条目，也即哈希碰撞使用的close addressing的链表
    Entry listNext;
    // 指向同一层次的下一个条目
    Entry layerNext;
};

// #define list_entry(ptr, type_ptr, field) \
//     ((type_ptr)((void *)(ptr) - (unsigned)(&(((type_ptr)(0))->field))))
// 

void semanticAnalyse(Node* root);
void Program(Node* root);
void ExtDefList(Node* root);
void ExtDef(Node* root);
Type Specifier(Node* root);
Type StructSpecifier(Node *root);
Function FunDec(Node *root);
FieldList VarList(Node *root);
void CompSt(Node *root, char *funcName, Type reType);
FieldList VarList(Node *root);
FieldList ParamDec(Node *root);
void ExtDecList(Node *root, Type type);
FieldList DefList(Node *root, __IdType class);
void StmtList(Node *root, Type reType);
FieldList Def(Node *root, __IdType class);
void Stmt(Node *root, Type reType);
Type Exp(Node *root);
FieldList VarDec(Node *root, Type type, __IdType class);
FieldList DecList(Node *root, Type type, __IdType class);
FieldList Dec(Node *root, Type type, __IdType class);
FieldList Args(Node *root);
void printType(Type type);
void printArgs(FieldList head);
void check();

#endif
