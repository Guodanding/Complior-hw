#include "semantic.h"
#include <string.h>

/* 整体看下来，要是分析的程序很大的话可能会很浪费空间，毕竟都没有回收 */

// 对要求2.2 变量的定义可以受嵌套作用域的影响思考：
// 1. 需要根据当前 symbol 找到当前 layer
// 2. layer 间可以快捷比较
// 3. 可以认为 findSymbol 中会找到许多同样的 symbol，且越前层次越高？
// 4. find 和 pop 都需要修改，但是 del 不需要？因为认为第一个找到的肯定是最高层次，而当前要 pop 的也是最高层次？ -> 不一定，不要忘记函数？

// 用 hash table 实现符号表
Entry symbolTable[TABLE_SIZE];
// 用栈来存储层次，层次的‘头’们，且该头不存具体的符号
// listNext 存的是具体的栈层次，layerNext 存的是同一层次的链表，千万不要搞混
// 把这个栈理解为优先级链表好像更好，同样是在头部插入
// 层次的‘头’们 的头 ！！！
Entry layerHeads;

// 初始化符号表
void initSymbolTable() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        // 散列表的每个槽位
        symbolTable[i] = NULL;
    }
    // 为栈头分配内存
    layerHeads = (Entry)malloc(sizeof(struct _Entry));
    // 初始化，加入第一层次
    Entry firstLayer = (Entry)malloc(sizeof(struct _Entry));
    firstLayer->layerNext = NULL;
    firstLayer->listNext = NULL;
    layerHeads->listNext = firstLayer;
    layerHeads->layerNext = NULL;
}

// 哈希函数
unsigned int hashFunc(char *name) {
    unsigned int val = 0, i;
    for (; *name; ++name) {
        val = (val << 2) + *name;
        if (i = val & ~TABLE_SIZE)
            val = (val ^ (i >> 12)) & TABLE_SIZE;
    }
    return val;
}

// 向符号表插入符号
void insertSymbol(Entry symbol) {
    // 哈希值
    unsigned int hash = hashFunc(symbol->name);
    // 插入对应槽位
    // 插入头部
    Entry cur = symbolTable[hash];  // 槽位当前符号
    symbolTable[hash] = symbol;
    symbol->listNext = cur;         // 向后移

    // 插入对应层次的链表
    // 插入头部
    Entry currentLayer = layerHeads->listNext; // 当前层次的头
    cur = currentLayer->layerNext;        // 当前层次的链表的第一个元素
    currentLayer->layerNext = symbol;
    symbol->layerNext = cur;            // 向后移
}

// 从符号表中查找所有同名符号
// 错误类型3：变量出现重复定义，或变量与前面定义过的结构体名字重复
// 错误类型4：同样的函数名出现了不止一次定义
// 不能出现 1. 两个 int a  2. int a 和 struct a  3. 两个 func a
// 但是可以出现 func a 和 int a
Entry findSymbol(char *name) {
    unsigned int hash = hashFunc(name);
    // 找到目标链表
    Entry target = symbolTable[hash];  

    /* 目前只是找到第一个 */

    // 根据链表查找目标
    while (target != NULL) {
        if (strcmp(target->name, name) == 0)
            break;
        target = target->listNext;
    }
    return target;
}

Entry findNextSymbol(Entry src, char *name) {
    Entry target = src;
    // 根据链表查找目标
    while (target != NULL) {
        if (strcmp(target->name, name) == 0)
            break;
        target = target->listNext;
    }
    return target;
}

// 查找函数
Entry findSymbolFunc(char* name) {
    unsigned int hash = hashFunc(name);
    Entry tmp = symbolTable[hash];
    while (tmp != NULL) {
        if (strcmp(tmp->name, name) == 0 && tmp->type->kind == FUNC) {
            break;
        }
        tmp = tmp->listNext;
    }
    return tmp;
}

/* 目前只是查找当前最上层次的符号 */

// 查找同一层次的符号
Entry findLayerSymbol(char *name) {
    Entry currentLayer = layerHeads->listNext;
    Entry target = currentLayer->layerNext;

    /* 目前只是找到第一个 */

    while (target != NULL) {
        if (strcmp(target->name, name) == 0)
            break;
        target = target->layerNext;
    }
    return target;
}

// 从符号表中删除符号
void delSymbol(char *name) {
    unsigned int hash = hashFunc(name);
    // 前一个节点
    Entry prev = NULL;
    // 目标节点
    Entry target = symbolTable[hash];

    /* 目前只是删除第一个 */

    while (target != NULL) {
        if (strcmp(target->name, name) == 0)
            break;
        prev = target;
        target = target->layerNext;
    }

    // 链表头节点
    if (target != NULL && prev == NULL)
        symbolTable[hash] = target->listNext;
    // 正常节点
    else if (target != NULL && prev != NULL)
        prev->listNext = target->listNext;
    // 空节点
}

// 插入一个层次，一般是遇到 '{'
void pushLayer() {
    Entry layer = (Entry)malloc(sizeof(struct _Entry));
    layer->listNext = NULL;
    layer->layerNext = NULL;
    // 插入栈头
    Entry cur = layerHeads->listNext;   // 当前最高层次的栈
    layerHeads->listNext = layer;       // 更新
    layer->listNext = cur;              // 往后移 
}

// 弹出一个层次，一般是遇到 '}'，同时也可以删除该层次的所有符号，也可以说是从层次栈删除符号
void popLayer() {
    Entry targetLayer = layerHeads->listNext;
    Entry targetSymbol = targetLayer->layerNext;
    while (targetSymbol != NULL) {

        /* 目前只是删除第一个 */
        // 也可以认为就是删除了最上层的 symbol
        delSymbol(targetSymbol->name);
        targetSymbol = targetSymbol->layerNext;
    }
}

// 类型等价判断函数
int typeEqual(Type a, Type b) {
    if (a == NULL && b == NULL)
        return 1;
    else if (a == NULL || b == NULL)
        return 0;
    else if (a->kind != b->kind)
        return 0;
    else if (a->kind == BASIC) {
        return a->basic == b->basic;
    }
    // 数组等价——基类型和维数相同
    else if (a->kind == ARRAY) {
        return typeEqual(a->array.elem, b->array.elem);
    }
    // 结构体类型结构等价
    else if (a->kind == STRUCT) {
        FieldList aFields = a->structure->head;
        FieldList bFields = b->structure->head;
        int res = 1;
        while (aFields != NULL && bFields != NULL) {
            if (!typeEqual(aFields->type, bFields->type)) {
                res = 0;
                break;
            }
            aFields = aFields->next;
            bFields = bFields->next;
        }
        if (aFields != NULL || bFields != NULL)
            res = 0;
        return res;
    }
    // 函数类型等价——返回类型、参数个数和参数类型等价
    else if (a->kind == FUNC) {
        if (!typeEqual(a->func->returnType, b->func->returnType))
            return 0;
        if (a->func->parmNum != b->func->parmNum)
            return 0;
        FieldList aFields = a->func->head;
        FieldList bFields = b->func->head;
        int res = 1;
        while (aFields != NULL && bFields != NULL) {
            if (!typeEqual(aFields->type, bFields->type)) {
                res = 0;
                break;
            }
            aFields = aFields->next;
            bFields = bFields->next;
        }
        if (aFields != NULL || bFields != NULL)
            res = 0;
        return res;
    }
    else
        return 0;
}

// 语义分析
void semanticAnalyse(Node *root) {
    initSymbolTable();
    Program(root);
    check();
}

// 整个程序的起始符号，表示一个完整的程序
void Program(Node *root) {
    // printf("1 Program %s in line %d\n", root->name, root->lineno);
    ExtDefList(root->children[0]);
}

void check() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (symbolTable[i] != NULL) {
            Entry entry = symbolTable[i];
            while (entry != NULL) {
                if (entry->type->kind == FUNC && entry->type->func->hasDefined == 0) {
                    printf("Error type 18 at line %d: Undefined function \"%s\".\n", entry->type->func->lineno, 
                    entry->name);
                }
                entry = entry->listNext;
            }
        }
    }
}

// 外部定义的列表，用来表示程序中的一系列外部定义。
void ExtDefList(Node *root) {
    // printf("2 ExtDefList %s in line %d\n", root->name, root->lineno);
    if (root->childNum != 0) {
        ExtDef(root->children[0]);
        ExtDefList(root->children[1]);
    }
}

// 外部定义，描述了变量声明、函数定义等。
void ExtDef(Node *root) {
    // printf("3 ExtDef %s in line %d\n", root->name, root->lineno);
    Type type = Specifier(root->children[0]);

    // 类型错误（结构体重复定义）
    if (type == NULL)
        return;

    // 非结构体
    if (type->kind == BASIC) {

        // Specifier ExtDecList SEMI
        if (strcmp(root->children[1]->name, "ExtDefList") == 0) {
            // printf("6%s\n", root->children[1]->name);
            ExtDecList(root->children[1], type);
        }

        // Specifier FunDec CompSt 
        else if (strcmp(root->children[1]->name, "FunDec") == 0) {
            // printf("6%s\n", root->children[1]->name);
            // TODO
            Function func = FunDec(root->children[1]);
            func->returnType = type;
            func->hasDefined = 0;
            Type newType = (Type)malloc(sizeof(struct _Type));
            newType->kind = FUNC;
            newType->func = func;
            Entry sym = findSymbolFunc(func->name);
            // 存在同名函数声明/定义
            if (sym != NULL) {
                // 是函数定义
                if (sym->type->func->hasDefined == 1) {
                    // 错误类型4：函数出现重复定义
                    if (strcmp(root->children[2]->name, "CompSt") == 0)
                        printf("Error type 4 at line %d: Redefined function \"%s\".\n", root->lineno, sym->name);
                    // 错误类型19：函数的多次声明相互冲突
                    else if (strcmp(root->children[2]->name, "SEMI") == 0 && !typeEqual(newType, sym->type))
                        printf("Error type 19 at line %d: Inconsistent declaration of function \"%s\".\n", root->lineno, sym->name);
                    return;
                }
                // 是函数声明
                if (sym->type->func->hasDefined == 0) {
                    if (strcmp(root->children[2]->name, "CompSt") == 0) {
                        // 错误类型19：函数的多次声明相互冲突
                        if (!typeEqual(newType, sym->type)) {
                            printf("Error type 19 at line %d: Inconsistent declaration of function \"%s\".\n", root->lineno, sym->name);
                            return;
                        }
                        // 为已声明的函数添加定义
                        else {
                            sym->type->func->hasDefined = 1;
                            return;
                        }
                    }
                    else {
                        // 错误类型19：函数的多次声明相互冲突
                        if (!typeEqual(newType, sym->type))
                            printf("Error type 19 at line %d: Inconsistent declaration of function \"%s\".\n", root->lineno, sym->name);
                        return;
                    }
                }
            }
            // 是首次出现的函数声明/定义
            if (strcmp(root->children[2]->name, "SEMI") == 0) {
                Entry res = (Entry)malloc(sizeof(struct _Entry));
                strcpy(res->name, func->name);
                res->type = newType;
                insertSymbol(res);
            }
            else {
                Entry res = (Entry)malloc(sizeof(struct _Entry));
                func->hasDefined = 1;
                strcpy(res->name, func->name);
                res->type = newType;
                insertSymbol(res);
                pushLayer();
                CompSt(root->children[2], func->name, func->returnType);
                popLayer();
            }
            return;
        }

    }
    
    // 结构体, !!!!!!! 为什么要用 STRUCT 而不是 STRUCT_DEF
    if (type->kind == STRUCT) {
        // printf("6%s\n", root->children[1]->name);
        Entry res = (Entry)malloc(sizeof(struct _Entry));
        strcpy(res->name, type->structure->name);
        res->type = (Type)malloc(sizeof(struct _Type));
        res->type->structure = type->structure;
        res->type->kind = STRUCT_DEF;
        insertSymbol(res);
    }

    return;
}

// 类型说明符，可以是基本类型（int, float等）或结构体类型。
Type Specifier(Node *root) {
    // printf("4 Specifier %s in line %d\n", root->name, root->lineno);
    // printf("5%s\n", root->children[0]->strVal);

    Node *child = root->children[0];
    // int float
    if (strcmp(child->name, "TYPE") == 0) {
        Type type = (Type)malloc(sizeof(struct _Type));
        type->kind = BASIC;
        if (strcmp(child->strVal, "int") == 0)
            type->basic = INT_TYPE;
        else if (strcmp(child->strVal, "float") == 0)
            type->basic = FLOAT_TYPE;

        return type;
    }
    // struct specifier
    else if (strcmp(child->name, "StructSpecifier") == 0) {
        return StructSpecifier(child);
    }
}

// 结构体类型说明符，可以定义结构体或引用已有的结构体
Type StructSpecifier(Node *root) {
    Type res = (Type)malloc(sizeof(struct _Type));
    res->kind = STRUCT;
    res->structure = (Structure)malloc(sizeof(struct _Structure));

    // STRUCT OptTag LC DefList RC
    if (root->childNum == 5) {
        // OptTag
        Node *child = root->children[1];
        // 空(empty)
        if (child->childNum == 0)   
            strcpy(res->structure->name, "");
        // 定义(ID)
        else {
            Entry sym = findSymbol(child->children[0]->strVal);
            // 如果找到，因为当前肯定是在最低层次，所以结构体的名字与前面定义过的结构体或变量的名字重复
            // 错误类型16：结构体的名字与前面定义过的结构体或变量的名字重复(注意，函数可以)
            /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
            if (sym != NULL && sym->type->kind != FUNC) {
                printf("Error type 16 at line %d: Duplicated name \"%s\".\n", child->lineno, child->children[0]->strVal);
                return NULL;
            }
            /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
            strcpy(res->structure->name, child->children[0]->strVal);
        }

        // DefList
        child = root->children[3];
        // 进入结构体定义，压入层次
        pushLayer();
        res->structure->head = DefList(child, FIELD);
        // 退出结构体定义，弹出层次
        popLayer();
        return res;
    }

    // STRUCT Tag
    else {
        // 是要去使用一个已经定义的结构体类型
        // Tag
        Node *child = root->children[1];
        Entry sym = findSymbol(child->children[0]->strVal);
        /*!!!!!!!!!!!!!!!!!!!!!!*/
        // 有没有可能，先定义了 struct a, 然后定义了 func a, 这时候再找表的话找的是最上层的也就是 func
        if (sym != NULL && sym->type->kind == FUNC)
            sym = findNextSymbol(sym, sym->name);
        /*!!!!!!!!!!!!!!!!!!!!!!*/
        // 不可能有两个 func a, 不可能同时有 struct a 和 int a
        // 错误类型17：直接使用未定义过的结构体来定义变量
        if (sym == NULL || sym->type->kind != STRUCT_DEF) {
            printf("Error type 17 at line %d: Undefined struct \"%s\".\n", child->lineno, child->children[0]->strVal);
            return NULL;
        }
        res->structure = sym->type->structure;
        return res;
    }
}

// 函数声明，包括函数名和参数列表
Function FunDec(Node *root) {
    Function res = (Function)malloc(sizeof(struct _Function));
    Node *ID = root->children[0];
    // 名字
    strcpy(res->name, ID->strVal);
    res->parmNum = 0;
    res->lineno = root->lineno;
    // ID LP RP
    if (root->childNum == 3) {
        res->head = NULL;
    }
    // ID LP VarList RP
    else {
        Node *varList = root->children[2];
        // 函数定义，插入一个层次
        pushLayer();
        res->head = VarList(varList);
        // 定义结束，弹出一个层次
        popLayer();
        // 计算参数的个数
        FieldList tmp = res->head;
        while (tmp != NULL) {
            res->parmNum++;
            tmp = tmp->next;
        }
    }
    return res;
}

// 参数列表中的参数声明，可以有多个参数
FieldList VarList(Node *root) {
    FieldList res = (FieldList)malloc(sizeof(struct _FieldList));
    // ParamDec
    if (root->childNum == 1) {
        res = ParamDec(root->children[0]);
    }
    // ParamDec COMMA VarList 
    else {
        res = ParamDec(root->children[0]);
        res->next = VarList(root->children[2]);
    }
    return res;
}

// 函数参数
FieldList ParamDec(Node *root) {
    Type type = Specifier(root->children[0]);
    return VarDec(root->children[1], type, FIELD);
}

// 复合语句，表示一段复杂的语句块。
void CompSt(Node *root, char *funcName, Type reType) {
    // 先把函数参数存进符号表
    Type type = reType;
    if (funcName != NULL) {
        Entry sym = findSymbolFunc(funcName);
        FieldList parms = sym->type->func->head;
        while (parms != NULL) {
            Entry parm = (Entry)malloc(sizeof(struct _Entry));
            strcpy(parm->name, parms->name);
            parm->type = parms->type;
            insertSymbol(parm);
            parms = parms->next;
        }
        type = sym->type->func->returnType;
    }
    DefList(root->children[1], VAR);
    StmtList(root->children[2], type);
    return;
}

// 全局变量名称列表
void ExtDecList(Node *root, Type type) {
    // VarDec
    if (root->childNum == 1) {
        VarDec(root->children[0], type, VAR);
    }
    // VarDec COMMA ExtDecList
    else {
        VarDec(root->children[0], type, VAR);
        ExtDecList(root->children[2], type);
    }
}

// 变量声明，包括变量名、数组类型、维度等
FieldList VarDec(Node *root, Type type, __IdType class) {
    // ID
    if (root->childNum == 1) {
        Entry sym = findLayerSymbol(root->children[0]->strVal);
        Entry symA = findSymbol(root->children[0]->strVal);
        // 域名/变量名重复定义或与结构体定义重复
        if (sym != NULL || (symA != NULL && symA->type->kind == STRUCT_DEF)) {
            // 错误类型15：结构体中域名重复定义
            if (class == FIELD)
                printf("Error type 15 at line %d: Redefined field \"%s\".\n", root->lineno, root->children[0]->strVal);
            // 错误类型3：变量出现重复定义，或变量与前面定义过的结构体的名字重复
            if (class == VAR)
                printf("Error type 3 at line %d: Redefined variable \"%s\".\n", root->lineno, root->children[0]->strVal);
            return NULL;
        }
        FieldList res = (FieldList)malloc(sizeof(struct _FieldList));
        strcpy(res->name, root->children[0]->strVal);
        res->type = type;
        // 域也要加符号表
        Entry tmp = (Entry)malloc(sizeof(struct _Entry));
        strcpy(tmp->name, root->children[0]->strVal);
        tmp->type = type;
        insertSymbol(tmp); 
        return res;
    }
    // VarDec LB INT RB (数组)
    else {
        Type newType = (Type)malloc(sizeof(struct _Type));
        newType->kind = ARRAY;
        newType->array.elem = type;
        // INT
        newType->array.size = root->children[2]->intVal;
        return VarDec(root->children[0], newType, class);
    }
}

// 局部定义列表，表示局部变量或常量的定义
FieldList DefList(Node *root, __IdType class) {
    // 空
    if (root->childNum == 0)
        return NULL;

    // Def DefList  
    FieldList res = Def(root->children[0], class);
    if (res == NULL)
        res = DefList(root->children[1], class);
    else {
        // 需要挂在最后面
        FieldList tmp = res;
        while (tmp->next != NULL) 
            tmp = tmp->next;
        tmp->next = DefList(root->children[1], class);
    }
    return res;
}

// 语句列表
void StmtList(Node *root, Type reType) {
    // Stmt StmtList
    if (root->childNum == 2) {
        Stmt(root->children[0], reType);
        StmtList(root->children[1], reType);
    }
    return;
}

// 局部定义，包括类型说明符和变量定义
FieldList Def(Node *root, __IdType class) {
    // Specifier DecList SEMI
    Type type = Specifier(root->children[0]);
    FieldList res = DecList(root->children[1], type, class);
    return res;
}

// TODO
void Stmt(Node *root, Type reType) {
    // Exp SEMI 
    if (strcmp(root->children[0]->name, "Exp") == 0) {
        Exp(root->children[0]);
    }

    // CompSt
    else if (strcmp(root->children[0]->name, "CompSt") == 0) {
        pushLayer();
        CompSt(root->children[0], NULL, reType);
        popLayer();
    }

    // RETURN Exp SEMI
    else if (strcmp(root->children[0]->name, "RETURN") == 0) {
        Type type = Exp(root->children[1]);
        // 错误类型8：return语句的返回类型与函数定义的返回类型不匹配
        if (!typeEqual(reType, type))
            printf("Error type 8 at line %d: Type mismatched for return.\n", root->lineno);
    }

    // IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
    // IF LP Exp RP Stmt ELSE Stmt
    else if (strcmp(root->children[0]->name, "IF") == 0) {
        // IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
        if (root->childNum == 5) {
            Exp(root->children[2]);
            Stmt(root->children[4], reType);
        }
        // IF LP Exp RP Stmt ELSE Stmt
        else {
            Exp(root->children[2]);
            Stmt(root->children[4], reType);
            Stmt(root->children[6], reType);
        }
    }

    // WHILE
    else if (strcmp(root->children[0]->name, "WHILE") == 0) {
        // WHILE LP Exp RP Stmt
        if (root->children[0]->childNum == 5) {
            Exp(root->children[2]);
            Stmt(root->children[4], reType);
        }
    }
    return;
}

// 表达式，可以是各种表达式，如赋值、逻辑、关系、算术等
Type Exp(Node *root) {
    // Exp
    if (strcmp(root->children[0]->name, "Exp") == 0) {

        // Exp DOT ID
        // 对结构体使用.操作符
        if (strcmp(root->children[1]->name, "DOT") == 0) {
            Type res = Exp(root->children[0]);
            if (res != NULL) {
                // 对非结构体变量使用"."操作符
                if (res->kind != STRUCT) {
                    printf("Error type 13 at line %d: Illegal use of \".\".\n", root->lineno);
                    return NULL;                    
                }
                char field[32];
                strcpy(field, root->children[2]->strVal);
                // 检测域名是否有效
                FieldList head = res->structure->head;
                Type ans = NULL;
                while (head != NULL) {
                    if (strcmp(field, head->name) == 0) {
                        ans = head->type;
                        break;
                    }
                    head = head->next;
                }
                // 错误类型14：访问结构体中未定义过的域
                if (ans == NULL) {
                    printf("Error type 14 at line %d: Non-existed field \"%s\".\n", root->lineno, field);
                    return NULL;
                }
                return ans;
            }
            return res;                
        }

        // Exp LB Exp RB
        // 数组取地址操作
        else if (strcmp(root->children[1]->name, "LB") == 0) {
            Type pre = Exp(root->children[0]);
            if (pre != NULL) {
                // 错误类型10：对非数组型变量使用"[...]"(数组访问)操作符
                if (pre->kind != ARRAY) {
                    printf("Error type 10 at line %d: Expect an array before [...].\n", root->lineno);
                    return NULL;
                }
                Type index = Exp(root->children[2]);
                // 错误类型12：数组访问操作符"[...]"中出现非整数
                if (index == NULL || index->kind != BASIC || index->basic != INT_TYPE) {
                    printf("Error type 12 at line %d: Expect an integer in [...].\n", root->lineno);
				    return NULL;
                }
                return pre->array.elem;
            }
            return pre;            
        }

        // Exp ASSIGNOP Exp
        // 赋值操作
        else if (strcmp(root->children[1]->name, "ASSIGNOP") == 0) {
            // 左值只有三种情况
            // 1、变量
            // 2、域
            // 3、数组元素
            Node* left = root->children[0];
            Node* right = root->children[2];
            Type leftType = NULL;
            Type rightType = Exp(right);
            if ((left->childNum == 1 && strcmp(left->children[0]->name, "ID") == 0) ||
				(left->childNum == 4 && strcmp(left->children[1]->name, "LB") == 0) ||
			    (left->childNum == 3 && strcmp(left->children[1]->name, "DOT") == 0))
				leftType = Exp(left);
            // 错误类型6：赋值号左边出现一个只有右值的表达式
			else {
				printf("Error type 6 at line %d: The left-hand side of an assignment must be a variable.\n", root->lineno);
                return NULL;
            }
            // 错误类型5：赋值号两边的表达式类型不匹配
            if (leftType != NULL && rightType != NULL && !typeEqual(leftType, rightType)) {
                printf("Error type 5 at line %d: Type mismatched for assignment.\n", root->lineno);
                return NULL;
            }
            return leftType;            
        }

        // Exp [AND OR RELOP PLUS MINUS STAR DIV] Exp
        // 普通二元运算操作
        else {
            Type pre = Exp(root->children[0]);
            Type aft = Exp(root->children[2]);
            if (pre == NULL || aft == NULL)
                return NULL;
            // 错误类型7：操作数类型不匹配或操作数类型与操作符不匹配
            if (!typeEqual(pre, aft)) {
                printf("Error type 7 at line %d: Type mismatched for operands.\n", root->lineno);
				return NULL;
            }
            if (strcmp(root->children[1]->name, "RELOP") == 0) {
                Type res = (Type)malloc(sizeof(struct _Type));
                res->kind = BASIC;
                res->basic = INT_TYPE;
                return res;  
            }
            return pre;
        }
    }

    // LP Exp RP 
    else if (strcmp(root->children[0]->name, "LP") == 0) {
        Type res = Exp(root->children[1]);
        return res;
    }

    // MINUS Exp
    else if (strcmp(root->children[0]->name, "MINUS") == 0) {
        Type res = Exp(root->children[1]);
        // 如果res为NULL应该是Exp有错，这里就不再报连锁错误
        if (res != NULL)
            if (res->kind != BASIC) {
                printf("Error type 7 at line %d: Operands type mismatched.\n", root->lineno);
                return NULL;
            }
        return res;
    }

    // NOT Exp
    else if (strcmp(root->children[0]->name, "NOT") == 0) {
        Type res = Exp(root->children[1]);
        if (res != NULL)
            if (res->kind != BASIC || res->basic != INT_TYPE) {
                printf("Error type 7 at line %d: Operands type mismatched.\n", root->lineno);
                return NULL;
            }
        return res;
    }

    // ID
    else if (strcmp(root->children[0]->name, "ID") == 0) {
        // ID是一个变量
        if (root->childNum == 1) {
            Entry sym = findSymbol(root->children[0]->strVal);
            // 使用不存在的变量
            if (sym == NULL) {
                printf("Error type 1 at line %d: Undefined variable \"%s\".\n", root->lineno, root->children[0]->strVal);
                return NULL;
            }
            return sym->type;
        }
        // ID是一个函数名
        else {
            Entry sym = findSymbolFunc(root->children[0]->strVal);
            if (sym == NULL) {
                sym = findSymbol(root->children[0]->strVal);
                // 错误类型11：对普通变量使用()操作符
                if (sym != NULL) {
                    printf("Error type 11 at line %d: \"%s\" is not a function.\n", root->lineno, sym->name);
                    return NULL;
                }
                // 错误类型2：函数在调用时未经定义
                else {
                    printf("Error type 2 at line %d: Undefined function \"%s\".\n", root->lineno, root->children[0]->strVal);
                    return NULL;
                }
            }
            // 使用未定义的函数
            // 错误类型2：函数在调用时未经定义
            if (sym->type->func->hasDefined == 0) {
                printf("Error type 2 at line %d: Undefined function \"%s\".\n", root->lineno, sym->name);
                return NULL;
            }
            // 提供的参数
            FieldList args = NULL;
            FieldList args_ = NULL;
            // 函数应有的参数
            FieldList realArgs = sym->type->func->head;
            // 函数有参数
            if (root->childNum == 4) {
                args = Args(root->children[2]);
                args_ = args;
            }
            int flag = 1;
            while (args != NULL && realArgs != NULL) {
                if (!typeEqual(args->type, realArgs->type)) {
                    flag = 0;
                    break;
                }
                args = args->next;
                realArgs = realArgs->next;
            }
            if (args != NULL || realArgs != NULL)
                flag = 0;
            if (flag == 0) {
                printf("Error type 9 at line %d: The method \"%s(", root->lineno, sym->name);
				printArgs(sym->type->func->head);
				printf(")\" is not applicable for the arguments \"(");
                printArgs(args_);
                printf(")\".\n");
            }
            return sym->type->func->returnType;
        }
    }

    // INT
    else if (strcmp(root->children[0]->name, "INT") == 0) {
        Type res = (Type)malloc(sizeof(struct _Type));
        res->kind = BASIC;
        res->basic = INT_TYPE;
        return res;
    }

    // FLOAT
    else if (strcmp(root->children[0]->name, "FLOAT") == 0) {
        Type res = (Type)malloc(sizeof(struct _Type));
        res->kind = BASIC;
        res->basic = FLOAT_TYPE;
        return res;
    }
}

// 变量定义列表，表示多个变量的定义
FieldList DecList(Node *root, Type type, __IdType class) {
    // Dec
    FieldList res = Dec(root->children[0], type, class);
    // Dec COMMA DecList
    if (root->childNum == 3) {
        if (res == NULL)
            res = DecList(root->children[2], type, class);
        else {
            FieldList tmp = res;
            while (tmp->next != NULL)
                tmp = tmp->next;
            tmp->next = DecList(root->children[2], type, class);
        }
    }
    return res;
}

// 变量定义，可以包括初始化赋值
FieldList Dec(Node *root, Type type, __IdType class) {
    FieldList res = VarDec(root->children[0], type, class);
    // 错误类型15：在定义是对域进行初始化
    if (class == FIELD && res != NULL && root->childNum == 3) {
        printf("Error type 15 at line %d: Initialized field \"%s\".\n", root->lineno, res->name);
        return NULL;        
    }
    if (class == VAR && res != NULL && root->childNum == 3) {
        Type right = Exp(root->children[2]);
        // 错误类型5：赋值号两边的表达式类型不匹配
        if (right != NULL && !typeEqual(right, type)) {
            printf("Error type 5 at line %d: Type mismatched.\n", root->lineno);
            return NULL;
        }
    }
    return res;
}

// 函数调用的参数列表，可以包括多个参数
FieldList Args(Node *root) {
    FieldList res = (FieldList)malloc(sizeof(struct _FieldList));
    // Exp
    res->type = Exp(root->children[0]);
    // Exp COMMA Args
    if (root->childNum == 3)
        res->next = Args(root->children[2]);
    return res;
}

// 打印参数类型列表
void printArgs(FieldList head) {
    if (head == NULL)
        return;
    if (head->next == NULL) {
        printType(head->type);
        return;
    }
    printType(head->type);
    printf(", ");
    printArgs(head->next);
}

// 打印类型
void printType(Type type) {
	if (type->kind == BASIC) {
        if (type->basic == INT_TYPE)
            printf("int");
        else
            printf("float");
    }
	else if (type->kind == STRUCT)
		printf("struct %s", type->structure->name);
	else if (type->kind == ARRAY) {
		printType(type->array.elem);
		printf("[]");
	}
    else if (type->kind = FUNC)
        printf("func %s", type->func->name);
    else if (type->kind == STRUCT_DEF)
        printf("struct definition %s", type->structure->name);
}