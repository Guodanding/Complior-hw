#include <stdio.h>
#include <stdlib.h>
#include "Tree.h"

/*
extern FILE* yyin;
extern int yyrestart(FILE* f);
extern int yyparse();
extern Node* root;
// extern int yydebug;
*/
extern int yyrestart(FILE* f);
extern int yyparse();
extern void printTree(Node* root, int depth);
extern void semanticAnalyse(Node *root);
extern Node* root;
extern int lexError;
extern int synError;

int main(int argc, char** argv) {

    // /*** 关于所使用的数据结构的单元测试 ***/

    // // 初始化符号表
    // initSymbolTable();

    // // 插入符号
    // Entry entry1 = (Entry)malloc(sizeof(struct _Entry));
    // strcpy(entry1->name, "variable1");
    // entry1->layerNext = NULL;
    // entry1->listNext = NULL;
    // insertSymbol(entry1);

    // Entry entry2 = (Entry)malloc(sizeof(struct _Entry));
    // strcpy(entry2->name, "variable2");
    // insertSymbol(entry2);

    // // 查找符号
    // Entry foundEntry1 = findSymbol("variable1");
    // if (foundEntry1 != NULL) {
    //     printf("Found symbol: %s\n", foundEntry1->name);
    // } else {
    //     printf("Symbol not found\n");
    // }

    // Entry foundEntry2 = findLayerSymbol("variable2");
    // if (foundEntry2 != NULL) {
    //     printf("Found layer symbol: %s\n", foundEntry2->name);
    // } else {
    //     printf("Layer symbol not found\n");
    // }

    // // 删除符号
    // delSymbol("variable1");

    // // 测试删除后的查找
    // Entry notFoundEntry = findSymbol("variable1");
    // if (notFoundEntry == NULL) {
    //     printf("Symbol deleted successfully\n");
    // } else {
    //     printf("Symbol not deleted\n");
    // }

    // // 插入和删除层次
    // pushLayer();
    // Entry layerEntry = (Entry)malloc(sizeof(struct _Entry));
    // strcpy(layerEntry->name, "layerVariable");
    // insertSymbol(layerEntry);
    // Entry foundLayerEntry = findSymbol("layerVariable");
    // if (foundLayerEntry != NULL) {
    //     printf("Found layer symbol: %s\n", foundLayerEntry->name);
    // } else {
    //     printf("Layer symbol not found\n");
    // }
    // popLayer();
    // Entry notFoundLayerEntry = findSymbol("layerVariable");
    // if (notFoundLayerEntry == NULL) {
    //     printf("Layer symbol deleted successfully\n");
    // } else {
    //     printf("Layer symbol not deleted\n");
    // }

    /***关于语义分析的单元测试***/

    if (argc <= 1)
        return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    if (root != NULL && lexError == 0 && synError == 0) {
        // printTree(root, 0);
        semanticAnalyse(root);
    }
    return 0;
}
