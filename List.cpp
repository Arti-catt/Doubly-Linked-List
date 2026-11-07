#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "colors.h"

#define LIST_DEBUG

#define GRAPH_FILE_NAME "graph.dot"
#define POISON   -666

FILE *graph = fopen(GRAPH_FILE_NAME, "w");

#ifdef LIST_DEBUG
    #define DEBUG_DUMP(listik) Dump(listik);
#else
    #define DEBUG_DUMP(...)
#endif

#define GONDON(condition)                                                    \
    if (!(condition)) {                                                      \
        fprintf(stderr, "File: %s:%d, Function: %s\nGONDON %sHELP%s YOU\n",  \
            __FILE__, __LINE__, __FUNCTION__, RED, RES);                     \
        abort();                                                             \
    }
#define ERR_TEXT(...)                                                                   \
    fprintf(stderr, "Function: %s, file: %s:%d\n", __FUNCTION__, __FILE__, __LINE__);   \
    fprintf(stderr, __VA_ARGS__);
    
#define LIST_VERIFY GONDON(ListVerify(listik) != FATAL_ERR)
#define LIST listik->list

enum Errors {
    NON_ERR   = 0,
    FATAL_ERR = 1,
};

struct Elem {
    int value;
    ssize_t next;
    ssize_t prev;
};

struct ListData {
    Elem *list;
    size_t capacity;
    size_t size;

    ssize_t tail;
    ssize_t head;

    ssize_t free;
    int err;
};

void ListInit(ListData *listik, size_t capacity);

void Insert(ListData *listik, ssize_t index, int number);     // index of element before
void Erase(ListData *listik, ssize_t index);                  // index of element for erase

bool CorrectIndex(ListData *listik, ssize_t index);

void PrintList(ListData *listik);
void ListDestroy(ListData *listik);
int ListVerify(ListData *listik);

void DumpPicture(ListData *listik);
void Dump(ListData *listik);

int main()
{
    ListData listik = {};
    ListInit(&listik, 4);

    Insert(&listik, 0, 10);
    Insert(&listik, 1, 20);
    Insert(&listik, 2, 30);
    Erase(&listik, 3);
    // Dump();
    PrintList(&listik);

    ListDestroy(&listik);
    return 0;
}

void ListInit (ListData *listik, size_t capacity)
{
    GONDON(listik)

    listik->list = (Elem *) calloc(capacity + 1, sizeof(Elem));
    GONDON(listik->list)

    listik->capacity = capacity + 1;

    for (ssize_t i = 1; i < listik->capacity; i++) {
        LIST[i].value = POISON;
        LIST[i].next = i + 1;
        LIST[i].prev = -1;
    }

    LIST[listik->capacity - 1].next = -1;
    listik->free = 1;

    LIST[0].value = POISON;         // Fiction element with index [0]
    listik->tail = 0;
    listik->head = 0;
}

void Insert(ListData *listik, ssize_t index, int number)
{
    GONDON(listik)
    GONDON(listik)

    LIST_VERIFY

    if (!CorrectIndex(listik, index))
        return;
    if (listik->free == -1) {
        ERR_TEXT("LIST IS %sFULL%s\n", RED, RES)
        DEBUG_DUMP(listik);
        return;
    }
    ssize_t new_free = LIST[listik->free].next;

    if (index == listik->head)
        listik->head = listik->free;
    if (index == 0)
        listik->tail = listik->free;

    LIST[listik->free].value = number;
    LIST[listik->free].next = LIST[index].next;
    LIST[index].next = listik->free;
    LIST[listik->free].prev = index;
    LIST[LIST[listik->free].next].prev = listik->free;

    LIST[0].next = listik->tail;
    LIST[0].prev = listik->head;
    LIST[listik->head].next = 0;

    listik->free = new_free;
    listik->size++;
    LIST_VERIFY
}

void Erase(ListData *listik, ssize_t index)
{
    GONDON(listik)
    GONDON(listik)

    LIST_VERIFY

    if (!CorrectIndex(listik, index))
        return;
    if (index == 0) {
        ERR_TEXT("CAN'T %sERASE%s FICTION [0] ELEMENT\n", RED, RES)
        DEBUG_DUMP(listik);
        return;
    }
    if (index == listik->head)
        listik->head = LIST[index].prev;
    if (index == listik->tail)
        listik->tail = LIST[index].next;

    LIST[LIST[index].prev].next = LIST[index].next;
    LIST[LIST[index].next].prev = LIST[index].prev;
    
    LIST[index].value = POISON;
    LIST[index].prev = -1;
    LIST[listik->head].next = 0;

    LIST[index].next = listik->free;
    listik->free = index;

    listik->size--;
    LIST_VERIFY
}

bool CorrectIndex(ListData *listik, ssize_t index)
{
    GONDON(listik)
    GONDON(listik)

    bool answer = true;
    if (index < 0 && index >= listik->capacity) {
        ERR_TEXT("INDEX <%zd> %sOUT OF RANGE%s\n", index, RED, RES)
        answer = false;
    }
    else if (LIST[index].prev == -1) {
        ERR_TEXT("ELEMENT WITH INDEX <%zd> %sDOESN'T EXIST%s\n", index, RED, RES)
        answer = false;
    }

    if (!answer) {
        DEBUG_DUMP(listik);
    }
    return answer;
}

void PrintList(ListData *listik) // base for verificator
{
    GONDON(listik)
    GONDON(listik)

    LIST_VERIFY

    ssize_t index = listik->tail;
    while (index != 0) {
        printf("%d\t", LIST[index].value);
        index = LIST[index].next;
    }
    printf("\n");

    DumpPicture(listik);
}

void ListDestroy(ListData *listik)
{
    GONDON(listik)

    free(listik->list);
    char *cleaner = (char *) listik;
    for (int i = 0; i < (sizeof(ListData) / sizeof(char)); i++) {
        *(cleaner++) = 0;
    }
    listik = NULL;
}

int ListVerify(ListData *listik)
{
    GONDON(listik)
    GONDON(listik)

    int err = NON_ERR;
    ssize_t index = listik->tail;

    for(size_t i = 0; i < listik->size; i++) {
        if (index != 0) {
            index = LIST[index].next;
            continue;
        }
        break;
    }
    if (index != 0) {
        DEBUG_DUMP(listik);
        ERR_TEXT("%sERROR%s OF VERIFICATION\n", RED, RES)
        err = FATAL_ERR;
    }

    return err;
}

void DumpPicture(ListData *listik)
{
    GONDON(listik)
    GONDON(listik)

    static FILE *graph_file = fopen(GRAPH_FILE_NAME, "w");
    GONDON(graph_file)

    fprintf(graph_file,
    "digraph G {\n"
    "\trankdir = LR;\n"
    "\tnode [shape = Mrecord, color = red, style = filled, fillcolor = pink];\n"
    "\tedge [color = darkred];\n");
    fprintf(graph_file, "\n");

    ssize_t index = 0;
    for (size_t i = 0; i < listik->size + 1; i++) {
        fprintf(graph_file, "\tnode%zd [label = \"{%d} | {index | %zd} | {next | %zd} | {prev | %zd}\", fillcolor = pink];\n",
                index, LIST[index].value, index, LIST[index].next, LIST[index].prev);
        index = LIST[index].next;
    }
    fprintf(graph_file, "\n");
    index = 0;
    for (size_t i = 0; i < listik->size; i++) {
        fprintf(graph_file, "\tnode%zu -> node%zu;\n", index, LIST[index].next);
        index = LIST[index].next;
    }
    fprintf(graph_file, "\tnode%zd:s -> node%d:s;\n", LIST[0].prev, 0);
    fprintf(graph_file, "}");
}

void Dump(ListData *listik)
{
    GONDON(listik)
    GONDON(listik)

    printf("Struct List {\nTAIL = %zd;\nHEAD = %zd;\nFREE = %zd;\n}\n____LISTDUMP____\n\n",
            listik->tail, listik->head, listik->free);

    printf("%sINDEX%s:\n", MASLO, RES);
    for (ssize_t i = 0; i < listik->capacity; i++) {
        if (i == listik->tail || i == listik->head)
            printf("%s%6zd%s\t", ORANGE_MASLO, i, RES);
        else
            printf("%s%6zd%s\t", MASLO, i, RES);
    }
    printf("\n");
    for (ssize_t i = 0; i < listik->capacity; i++) {
        char end_ind[2] = {' ', ' '};
        if (i == listik->tail && i == listik->head) {
            end_ind[0] = 'T';
            end_ind[1] = 'H';
        }
        else if (i == listik->head)
            end_ind[1] = 'H';
        else if (i == listik->tail)
            end_ind[1] = 'T';
        printf("%s%6s%s\t", ORANGE_MASLO, end_ind, RES);
    }
    printf("\n");

    printf("%sVALUE%s:\n", BLUE_F, RES);
    for (ssize_t i = 0; i < listik->capacity; i++) {
        printf("%s%6d%s\t", BLUE, LIST[i].value, RES);
    }
    printf("\n\n");

    printf("%sNEXT%s:\n", GREEN_F, RES);
    for (ssize_t i = 0; i < listik->capacity; i++) {
        printf("%s%6zd%s\t", GREEN, LIST[i].next, RES);
    }
    printf("\n\n");

    printf("%sPREV%s:\n", RED_F, RES);
    for (ssize_t i = 0; i < listik->capacity; i++) {
        printf("%s%6zd%s\t", RED, LIST[i].prev, RES);
    }
    printf("\n\n");
}