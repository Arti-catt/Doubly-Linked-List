#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "colors.h"

#define LIST_DEBUG

#define FOLDER_NAME     "DumpPictures"
#define DUMP_FILE       "dump.html"
#define GRAPH_FILE_NAME "graph.dot"
 
#define POISON -666

#ifdef LIST_DEBUG
    #define DEBUG_PARS , const char *function, const char *file, int line
    #define DEBUG_ARGS , __FUNCTION__, __FILE__, __LINE__
    FILE *dump_file = fopen(DUMP_FILE, "wa");
    #define DEBUG(...) __VA_ARGS__
#else
    #define DEBUG_PARS
    #define DEBUG_ARGS
    #define DEBUG_DUMP(...)
    #define DEBUG(...)
#endif

#define GONDON(condition)                                                               \
    if (!(condition)) {                                                                 \
        fprintf(stderr, "Function: %s, file: %s:%d\nGONDON %sHELP%s YOU\n",             \
            __FUNCTION__, __FILE__, __LINE__, RED, RES);                                \
        DEBUG(fprintf(dump_file, "Function: %s, file: %s:%d\nGONDON HELP YOU\n",        \
            __FUNCTION__, __FILE__, __LINE__);)                                         \
        DEBUG(Dump(lipstik, BASE DEBUG_ARGS);)                                          \
        DEBUG(fprintf(dump_file, "</body>"); fclose(dump_file);)                        \
        abort();                                                                        \
    }

#define LIST_VERIFY(mode)                                                               \
    if (ListVerify(lipstik, mode DEBUG_ARGS) != NON_ERR) {                              \
        fprintf(stderr, "Function: %s, file: %s:%d\nVERIFICATION %sERROR%s\n",          \
            __FUNCTION__, __FILE__, __LINE__, RED, RES);                                \
        return;                                                                         \
    }

#define LIST lipstik->list
#define LEN_COMMAND 100

#define NUMBER_ERRORS 11

enum Errors {
    NON_ERR          =   0b0000000000,
    NON_CONNECT      =   0b0000000001,
    INCORRECT_ENDS   =   0b0000000010,
    NEGATIVE_TAIL    =   0b0000000100,
    NEGATIVE_HEAD    =   0b0000001000,
    FICTION_ELEM_ERR =   0b0000010000,
    INCORRECT_INDEX  =   0b0000100000,
    NON_VALID_INDEX  =   0b0001000000,
    LIST_IS_FULL     =   0b0010000000,
    FATAL_ERR        =   0b0100000000,
    SYSTEM_ERR       =   0b1000000000,
};

enum Parametres {
    INSERT = 0,
    ERASE  = 1,
    BEFORE = 2,
    AFTER  = 3,
    BASE   = 4,
};

struct ErrMessages {
    int number;
    const char *message;
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
    ErrMessages *err_list;
    int err;

    #ifdef LIST_DEBUG
        int numerator_of_pictures;
        char command[LEN_COMMAND];
        char current_graph_name[LEN_COMMAND];
    #endif
};

void ListInit(ListData *lipstik, size_t capacity);

void Insert(ListData *lipstik, ssize_t index, int number);     // index of element before
void Erase(ListData *lipstik, ssize_t index);                  // index of element for erase

int CorrectParametres(ListData *lipstik, ssize_t index, int mode);

void PrintList(ListData *lipstik);
void ListDestroy(ListData *lipstik);
int ListVerify(ListData *lipstik, int mode DEBUG_PARS);

ErrMessages *StructsErrCreator();
void ErrorShredder(ListData *lipstik);

#ifdef LIST_DEBUG

void CommandsCreator(ListData *lipstik);

void Dump(ListData *lipstik, int mode DEBUG_PARS);
void DumpParametres(ListData *lipstik, int mode);

void DumpStructure(ListData *lipstik);
void DumpTable(ListData *lipstik);

void DumpPicture(ListData *lipstik);
void WriteGeneralPart(ListData *lipstik, FILE *graph_file);
void NodesCreator(ListData *lipstik, FILE *graph_file);
void ConnectionCreator(ListData *lipstik, FILE *graph_file);

void AddBackgroundPicture(ListData *lipstik, FILE *file_html);

void CommandsPreparation(ListData *lipstik);
void ClearNameEnd(char *graph_name, int symbol);

#endif

int main()
{
    ListData lipstik = {};
    ListInit(&lipstik, 10);

    Insert(&lipstik, 0, 10);
    Insert(&lipstik, 1, 20);
    Insert(&lipstik, 2, 30);
    lipstik.list[0]. next = 3;
    Erase(&lipstik, 3);
    Erase(&lipstik, 1);

    PrintList(&lipstik);
    ListDestroy(&lipstik);
    return 0;
}

void ListInit (ListData *lipstik, size_t capacity)
{
    GONDON(lipstik)

    lipstik->list = (Elem *) calloc(capacity + 1, sizeof(Elem));
    GONDON(lipstik->list)

    lipstik->capacity = capacity + 1;       // + fiction element

    for (ssize_t i = 1; i < lipstik->capacity; i++) {
        LIST[i].value = POISON;
        LIST[i].next = i + 1;
        LIST[i].prev = -1;
    }

    LIST[lipstik->capacity - 1].next = -1;
    lipstik->free = 1;

    LIST[0].value = POISON;         // Fiction element with index [0]
    lipstik->tail = 0;
    lipstik->head = 0;

    lipstik->err = 0;
    lipstik->err_list = StructsErrCreator();

    #ifdef LIST_DEBUG
        CommandsCreator(lipstik);
    #endif
}

ErrMessages *StructsErrCreator()
{
    static ErrMessages err_list[NUMBER_ERRORS] = {
        {NON_ERR,           "Errcode = 0, non errors in program"                 },
        {NON_CONNECT,       "Incorrect connection between elements"              },
        {INCORRECT_ENDS,    "Incorrect ENDS of list"              },
        {NEGATIVE_TAIL,     "Negative tail"                                      },
        {NEGATIVE_HEAD,     "Negative head"                                      },
        {FICTION_ELEM_ERR,  "Value of the fiction element is incorrect"          },
        {INCORRECT_INDEX,   "Index as a parameter of the function is incorrect"  },
        {NON_VALID_INDEX,   "Element with index is incorrect for operation"      },
        {LIST_IS_FULL,      "Size of the list is critical"                       },
        {FATAL_ERR,         "Exotic error like problem in switch"                },
        {SYSTEM_ERR,        "Function system worked incorrect"                   },
    };
    return err_list;
}

void Insert(ListData *lipstik, ssize_t index, int number)
{
    GONDON(lipstik)
    if (lipstik->err != NON_ERR)
        return;
    if (CorrectParametres(lipstik, index, INSERT) != NON_ERR) {
        DEBUG(Dump(lipstik, BEFORE DEBUG_ARGS);)
        return;
    }
    LIST_VERIFY(BEFORE)

    if (lipstik->err != NON_ERR)
        return;

    ssize_t new_free = LIST[lipstik->free].next;

    if (index == lipstik->head)
        lipstik->head = lipstik->free;
    if (index == 0)
        lipstik->tail = lipstik->free;

    LIST[lipstik->free].value = number;
    LIST[lipstik->free].next = LIST[index].next;
    LIST[index].next = lipstik->free;
    LIST[lipstik->free].prev = index;
    LIST[LIST[lipstik->free].next].prev = lipstik->free;

    LIST[0].next = lipstik->tail;
    LIST[0].prev = lipstik->head;
    LIST[lipstik->head].next = 0;

    lipstik->free = new_free;
    lipstik->size++;
    LIST_VERIFY(AFTER)
}

void Erase(ListData *lipstik, ssize_t index)
{
    GONDON(lipstik)
    if (lipstik->err != NON_ERR)
        return;
    if (CorrectParametres(lipstik, index, ERASE) != NON_ERR) {
        DEBUG(Dump(lipstik, BEFORE DEBUG_ARGS);)
        return;
    }
    LIST_VERIFY(BEFORE)

    if (index == lipstik->head)
        lipstik->head = LIST[index].prev;
    if (index == lipstik->tail)
        lipstik->tail = LIST[index].next;

    LIST[LIST[index].prev].next = LIST[index].next;
    LIST[LIST[index].next].prev = LIST[index].prev;
    
    LIST[index].value = POISON;
    LIST[index].prev = -1;
    LIST[lipstik->head].next = 0;

    LIST[index].next = lipstik->free;
    lipstik->free = index;

    lipstik->size--;
    LIST_VERIFY(AFTER)
}

int CorrectParametres(ListData *lipstik, ssize_t index, int mode)
{
    GONDON(lipstik)

    if (mode == INSERT && lipstik->free == -1) {
        // ERR_TEXT("LIST IS %sFULL%s\n", RED, RES)
        lipstik->err += LIST_IS_FULL;
    }
    if (mode == ERASE && index == 0) {
        // ERR_TEXT("CAN'T %sERASE%s FICTION [0] ELEMENT\n", RED, RES)
        lipstik-> err += NON_VALID_INDEX;
    }
    if (index < 0 && index >= lipstik->capacity) {
        // ERR_TEXT("INDEX <%zd> %sOUT OF RANGE%s\n", index, RED, RES)
        lipstik->err += INCORRECT_INDEX;
    }
    else if (LIST[index].prev == -1 && (lipstik->err & NON_VALID_INDEX) == 0) {
        // ERR_TEXT("ELEMENT WITH INDEX <%zd> %sDOESN'T EXIST%s\n", index, RED, RES)
        lipstik->err += NON_VALID_INDEX;
    }
    return lipstik->err;
}

void PrintList(ListData *lipstik) // base for verificator
{
    GONDON(lipstik)
    if (lipstik->err != NON_ERR)
        return;
    LIST_VERIFY(BASE)

    ssize_t index = lipstik->tail;
    while (index != 0) {
        printf("%d\t", LIST[index].value);
        index = LIST[index].next;
    }
    printf("\n");
}

void ListDestroy(ListData *lipstik) // Cleaner of the structure
{
    GONDON(lipstik)
    LIST_VERIFY(BASE)

    free(lipstik->list);
    char *cleaner = (char *) lipstik;
    for (int i = 0; i < (sizeof(ListData) / sizeof(char)); i++) {
        *(cleaner++) = 0;
    }
    lipstik = NULL;

    #ifdef LIST_DEBUG
        fprintf(dump_file, "</body>");
        fclose(dump_file);
    #endif
}

int ListVerify(ListData *lipstik, int mode DEBUG_PARS)
{
    GONDON(lipstik)
    ssize_t index = lipstik->tail;

    for(size_t i = 0; i < lipstik->size; i++) {
        if (LIST[LIST[index].prev].next == index) {
            index = LIST[index].next;
            continue;
        }
        lipstik->err += NON_CONNECT;
        break;
    }
    if (index != 0)
        lipstik->err += INCORRECT_ENDS;
    if (lipstik->tail < 0)
        lipstik->err += NEGATIVE_TAIL;
    if (lipstik->head < 0)
        lipstik->err += NEGATIVE_HEAD;
    if (LIST[0].value != POISON) {
        lipstik->err += FICTION_ELEM_ERR;
    }
    #ifdef LIST_DEBUG
        Dump(lipstik, mode, function, file, line);
    #else
        ErrorShredder(lipstik);
    #endif
    return lipstik->err;
}

void ErrorShredder(ListData *lipstik)
{
    GONDON(lipstik)

    if (lipstik->err == NON_ERR) {
        #ifdef LIST_DEBUG
            fprintf(dump_file, "<span style=\"color: lime;\">%s</span>\n", lipstik->err_list[0].message);
        #endif
        return;
    }
    for (int i = 0; i < NUMBER_ERRORS; i++) {
        if (lipstik->err & lipstik->err_list[i].number) {
            #ifdef LIST_DEBUG
                fprintf(dump_file, "<span style=\"color: red;\">%s</span>\n", lipstik->err_list[i].message);
            #else
                fprintf(stderr, "%s%s%s\n", RED, lipstik->err_list[i].message, RES);
            #endif
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////

#ifdef LIST_DEBUG

void Dump(ListData *lipstik, int mode DEBUG_PARS)     // Work with HTML file
{
    GONDON(lipstik)

    AddBackgroundPicture(lipstik, dump_file);

    fprintf(dump_file,
        "<h1>Dumping<img src=\"https://magnotek.ru/wp-content/uploads/2018/07/g0R5.gif\" width=\"20\"></h1>\n"
        "<p>"
        "<pre style=\"font-size: 24px;\">"
        "<span style=\"color: gold;\">File: %s, line: %d\n",
        file, line
    );
    fprintf(dump_file, "Function: <span style=\"font-weight: bold;\">%s</span>", function);
    DumpParametres(lipstik, mode);

    DumpPicture(lipstik);
    ErrorShredder(lipstik);

    DumpStructure(lipstik);
    DumpTable(lipstik);

    fprintf(dump_file,
        "\n"
        "<span style=\"font-weight: bold;\">__GRAPHIC_DUMP__</span>\n"
        "</pre>"
        "</p>\n"
        "<img src=\"/home/solo/Documents/GitHub/Doubly-Linked-List/%s/%s\">\n",
        FOLDER_NAME, lipstik->current_graph_name
    );
    fprintf(dump_file, "\n");

}

void AddBackgroundPicture(ListData *lipstik, FILE *file_html)
{
    GONDON(file_html)

    static int non_first = false;
    if (non_first)
        return;
    fprintf(file_html,
        "<body style=\""
        "background-image: url(\'/home/solo/Documents/GitHub/Doubly-Linked-List/kaneki_bubble_small.jpg\'); "
        "background-size: cover; "
        "background-repeat: no-repeat;"
        "background-position: center; "
        "background-attachment: fixed; "
        "color: white;\">\n"
    );
    non_first = true;
}

void CommandsCreator(ListData *lipstik)
{
    GONDON(lipstik)

    strncpy(lipstik->command, "/usr/bin/dot -Tpng ", strlen("/usr/bin/dot -Tpng ") + 1);
    strncat(lipstik->command, GRAPH_FILE_NAME, strlen(GRAPH_FILE_NAME) + 1);
    strncat(lipstik->command, " -o ", 5);
    strncat(lipstik->command, FOLDER_NAME, strlen(FOLDER_NAME) + 1);
    strncat(lipstik->command, "/", 2);
}

void DumpParametres(ListData *lipstik, int mode)
{
    GONDON(lipstik)

    switch(mode) {
        case BEFORE:
            fprintf(dump_file, "(BEFORE)");
            break;
        case AFTER:
            fprintf(dump_file, "(AFTER)");
            break;
        default:
            break;
    }
    fprintf(dump_file, "</span>\n");
}

void DumpPicture(ListData *lipstik)     // Work with DOT file
{
    GONDON(lipstik)

    static int open_check = mkdir(FOLDER_NAME, 0777);

    FILE *graph_file = fopen(GRAPH_FILE_NAME, "w");
    GONDON(graph_file)
    // Write in the html file
    WriteGeneralPart(lipstik, graph_file);
    NodesCreator(lipstik, graph_file);
    ConnectionCreator(lipstik, graph_file);

    fclose(graph_file);

    CommandsPreparation(lipstik);

    int status = system(lipstik->command);
    if (status != 0)
        lipstik->err += SYSTEM_ERR;
}

void WriteGeneralPart(ListData *lipstik, FILE *graph_file)
{
    GONDON(lipstik)

    fprintf(graph_file,
        "digraph G {\n"
        "\tbgcolor = \"transparent\";\n"
        "\trankdir = LR;\n"
        "\tnode [shape = Mrecord, color = darkred, style = filled, fillcolor = pink];\n"
        "\tedge [color = \"white\", penwidth = 2];\n"
    );
    fprintf(graph_file, "\n");
}

void NodesCreator(ListData *lipstik, FILE *graph_file)
{
    GONDON(lipstik)

    // fprintf(graph_file, "\t{rank = same; tail; head;}\n");
    ssize_t index = 0;
    fprintf(graph_file, "\tnode%zd [label = \"{[0]} | {index | %zd} | {next | %zd} | {prev | %zd}\", fillcolor = darkgrey];\n",
                index, index, LIST[index].next, LIST[index].prev);
    index = LIST[index].next;

    for (size_t i = 0; index != 0 && i < lipstik->size; i++) {
        if (index == lipstik->head && index == lipstik->tail)
            fprintf(graph_file, "\tnode%zd [label = \"{HEAD} | {TAIL} | {%d} | {index | %zd} | {next | %zd} | {prev | %zd}\", fillcolor = crimson];\n",
                    index, LIST[index].value, index, LIST[index].next, LIST[index].prev);
        else if (index == lipstik->head)
            fprintf(graph_file, "\tnode%zd [label = \"{HEAD} | {%d} | {index | %zd} | {next | %zd} | {prev | %zd}\", fillcolor = crimson];\n",
                    index, LIST[index].value, index, LIST[index].next, LIST[index].prev);
        else if (index == lipstik->tail)
            fprintf(graph_file, "\tnode%zd [label = \"{TAIL} | {%d} | {index | %zd} | {next | %zd} | {prev | %zd}\", fillcolor = purple];\n",
                    index, LIST[index].value, index, LIST[index].next, LIST[index].prev);
        else
            fprintf(graph_file, "\tnode%zd [label = \"{%d} | {index | %zd} | {next | %zd} | {prev | %zd}\"];\n",
                    index, LIST[index].value, index, LIST[index].next, LIST[index].prev);
        index = LIST[index].next;
    }
    index = lipstik->free;
    fprintf(graph_file, "\tnode%zd [label = \"{FREE} | {%d} | {index | %zd} | {next | %zd} | {prev | %zd}\", fillcolor = darkorange];\n",
            index, LIST[index].value, index, LIST[index].next, LIST[index].prev);
    index = LIST[index].next;
    for (size_t i = 0; index != -1 && i < lipstik->capacity; i++) {
        fprintf(graph_file, "\tnode%zd [label = \"{%d} | {index | %zd} | {next | %zd} | {prev | %zd}\", fillcolor = sienna];\n",
                index, LIST[index].value, index, LIST[index].next, LIST[index].prev);
        index = LIST[index].next;
    }
    fprintf(graph_file, "\n");
}

void ConnectionCreator(ListData *lipstik, FILE *graph_file)
{
    GONDON(lipstik)

    ssize_t index = 0;
    for (size_t i = 0; i < lipstik->size && LIST[index].next != 0; i++) {
        if (index != LIST[LIST[index].next].prev)
            fprintf(graph_file, "\tnode%zd->node%zd [color = \"red\", penwidth = 5];\n", index, LIST[index].next);
        else
            fprintf(graph_file, "\tnode%zd->node%zd;\n", index, LIST[index].next);
            // fclose(graph_file);
            // assert(false);
        index = LIST[index].next;
    }

    if (LIST[LIST[index].next].prev != index)
        fprintf(graph_file, "\tnode%zd:s->node%zd:s [color = \"red\", penwidth = 3];\n", index, LIST[index].next);
    else
        fprintf(graph_file, "\tnode%zd:s->node%zd:s;\n", index, LIST[index].next);

    fprintf(graph_file, "\tnode%zd->node%zd [style = invis];\n", index, lipstik->free);

    index = lipstik->free;
    for (size_t i = 0; i < lipstik->capacity && LIST[index].next != -1; i++) {
            fprintf(graph_file, "\tnode%zd->node%zd;\n", index, LIST[index].next);
            // fclose(graph_file);
            // assert(false);
        index = LIST[index].next;
    }

    fprintf(graph_file, "}");
}

void CommandsPreparation(ListData *lipstik)
{
    GONDON(lipstik)

    ClearNameEnd(lipstik->command, '/');

    snprintf(lipstik->current_graph_name, LEN_COMMAND, "graph%d.png", lipstik->numerator_of_pictures);
    strncat(lipstik->command, lipstik->current_graph_name, strlen(lipstik->current_graph_name));

    lipstik->numerator_of_pictures++;
}

void ClearNameEnd(char *graph_name, int symbol)
{
    if (graph_name == NULL)
        return;
    char *letter = strrchr(graph_name, symbol);
    letter++;

    while (*letter != '\0')
        *(letter++) = 0;
}

void DumpStructure(ListData *lipstik)
{
    GONDON(lipstik)

    fprintf(dump_file,
        "\nstruct <span style=\"color: gold;\">ListData<%p></span> {\n"
        "\tcapacity = %zu;\n"
        "\tsize = %zu;\n\n"
        "\t<span style=\"color: purple; font-weight: bold;\">tail</span> = %zd;\n"
        "\t<span style=\"color: crimson; font-weight: bold;\">head</span> = %zu;\n\n"
        "\t<span style=\"color: darkorange; font-weight: bold;\">free</span> = %zd;\n"
        "\t<span style=\"color: red;\">err</span>  = %b;\n"
        "}\n",
        lipstik->list, lipstik->capacity, lipstik->size, lipstik->tail,
        lipstik->head, lipstik->free, lipstik->err
    );
}

void DumpTable(ListData *lipstik)
{
    GONDON(lipstik)
    // if (lipstik->err == NON_ERR)
        // return;
    fprintf(dump_file,
        "\n<table border =\"1\" style=\"width: 20%%; font-size: 22px;\">\n"
        "<tr style=\"font-weight: bold;\"> <td>Index</td><td>Value</td><td>Next</td><td>Prev</td> </tr>\n"
    );
    for (ssize_t i = 0; i < lipstik->capacity; i++) {
        if (i == lipstik->head)
            fprintf(dump_file, "<tr style=\"background-color: crimson;\"> ");
        else if (i == lipstik->tail)
            fprintf(dump_file, "<tr style=\"background-color: purple;\"> ");
        else if (i == lipstik->free)
            fprintf(dump_file, "<tr style=\"background-color: darkorange;\"> ");
        else
            fprintf(dump_file, "<tr> ");
        fprintf(dump_file,
            "<td>%zd</td>"
            "<td>%d</td>"
            "<td>%zd</td>"
            "<td>%zd</td> </tr>\n",
            i, LIST[i].value, LIST[i].next, LIST[i].prev
        );
    }
    fprintf(dump_file, "</table>\n");
}

#endif

///////////////////////////////////////////////////////////////////////////////////

// void Old_Dump(ListData *lipstik)
// {
    // GONDON(lipstik)
// 
    // printf("Struct List {\nTAIL = %zd;\nHEAD = %zd;\nFREE = %zd;\n}\n____LISTDUMP____\n\n",
            // lipstik->tail, lipstik->head, lipstik->free);
// 
    // printf("%sINDEX%s:\n", MASLO, RES);
    // for (ssize_t i = 0; i < lipstik->capacity; i++) {
        // if (i == lipstik->tail || i == lipstik->head)
            // printf("%s%6zd%s\t", ORANGE_MASLO, i, RES);
        // else
            // printf("%s%6zd%s\t", MASLO, i, RES);
    // }
    // printf("\n");
    // for (ssize_t i = 0; i < lipstik->capacity; i++) {
        // char end_ind[2] = {' ', ' '};
        // if (i == lipstik->tail && i == lipstik->head) {
            // end_ind[0] = 'T';
            // end_ind[1] = 'H';
        // }
        // else if (i == lipstik->head)
            // end_ind[1] = 'H';
        // else if (i == lipstik->tail)
            // end_ind[1] = 'T';
        // printf("%s%6s%s\t", ORANGE_MASLO, end_ind, RES);
    // }
    // printf("\n");
// 
    // printf("%sVALUE%s:\n", BLUE_F, RES);
    // for (ssize_t i = 0; i < lipstik->capacity; i++) {
        // printf("%s%6d%s\t", BLUE, LIST[i].value, RES);
    // }
    // printf("\n\n");
// 
    // printf("%sNEXT%s:\n", GREEN_F, RES);
    // for (ssize_t i = 0; i < lipstik->capacity; i++) {
        // printf("%s%6zd%s\t", GREEN, LIST[i].next, RES);
    // }
    // printf("\n\n");
// 
    // printf("%sPREV%s:\n", RED_F, RES);
    // for (ssize_t i = 0; i < lipstik->capacity; i++) {
        // printf("%s%6zd%s\t", RED, LIST[i].prev, RES);
    // }
    // printf("\n\n");
// }