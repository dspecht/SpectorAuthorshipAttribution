#ifndef WIN32MAIN_H

#include "stdlib.h"
#include "malloc.h"
#include "stdio.h"
#include "stdint.h"
#include "cStringLib.h"

#define internal static
#define globalVar static
#define localPersist static

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
//-------------------------
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float r32;

#define SPECTOR_DEBUG_MODE 1
//TODO: When Casey Updates the HMH Assertion macro update this
#if SPECTOR_DEBUG_MODE
    #define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
    #define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) (Megabytes(Value)*1024)
#define Terabytes(Value) (Gigabytes(Value)*1024)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define LONGEST_WORD_LENGTH 100
#define MAX_WORDS_IN_A_LIST 100000
#define MAX_WORDS_PER_LIST 1000

//flesh out more what the documend structure is going to look like

struct WordList
{
    char *words[MAX_WORDS_PER_LIST];
    int count = 0;
};

struct Node
{
    char Tag[LONGEST_WORD_LENGTH];
    WordList *WordsInCategory;
    
    Node *Parent;

    Node *LeftSibling;
    Node *RightSibling;
    
    Node *Child;
};

struct NodeTree
{
    Node *RootNode;
    Node *CurrentNode;

    int NumberOfNodes;
};

struct DocumentWord
{
    char *word;
    char *tag;
    u32 count = 0;
};


#define WIN32MAIN_H
#endif
