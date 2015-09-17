#include "win32main.h"

globalVar WordList wordsListArray[9];

#define MAX_XML_DEPTH 10
#define LONGEST_WORD_LENGTH 100
#define MAX_WORDS_IN_A_LIST 100000

struct TagStack
{
    WordTypeTag_TDE Tags[MAX_XML_DEPTH];
    int CurrentTagDepth;
};

void PushTag(WordTypeTag_TDE Tag, TagStack * Stack)
{
    int StackIndex = ++Stack->CurrentTagDepth;
    Stack->Tags[StackIndex] = Tag;
}

WordTypeTag_TDE PopTag(TagStack * Stack)
{
    int StackIndex = Stack->CurrentTagDepth--;
    return Stack->Tags[StackIndex];
}
FILE* OpenFile(char *filePath, char *openFMT="rw")
{
    FILE *handle = NULL;
    fopen_s(&handle, filePath, openFMT);
    return handle;
}

void StringReformat(char * Token)
{
    char CurrentCharacter;
    int Current = 0;
    int LastWrittenTo = 0;

    while(Token[Current] != '\0')
    {
        CurrentCharacter = Token[Current++];
        Assert(CurrentCharacter);

        //TODO(Ruy) - MAIN - Remove '/' from tags after implementing stack-based XML parsing

        if(CurrentCharacter == ' ' ||
           CurrentCharacter == '\n'||
           CurrentCharacter == '\t')
        {
            continue;
        }
        else
        {
            Token[LastWrittenTo++] = CurrentCharacter;
        }
    }
    if(LastWrittenTo > 0)
    {
        Token[LastWrittenTo] = '\0';
    }
    else
    {
        Token = "";
    }
}

WordTypeTag_TDE FindTag(char * Token, char **tags, TagStack * Stack)
{
    if(Token[0] == '<')
    {
        if(Token[1] == '/')
        {
            WordTypeTag_TDE Tag = PopTag(Stack);
            return NULL_TAG;
        }
        else
        {
            for(int i = 0;
                i < NUMBER_OF_TAGS;
                ++i)
            {
                if(CompareString(Token, tags[i]))
                {
                    PushTag(WordTypeTag_TDE(i), Stack);
                    return NEW_TAG;
                }
            }
        }
        return NULL_TAG;
    }
    return NON_TAG;
}

void ExtractXMLNodeContents(FILE *handle, char **tags)
{
    rewind(handle);
    char *Token = (char *)calloc(1,sizeof(char) * LONGEST_WORD_LENGTH);

    TagStack XMLStack = {};
    WordTypeTag_TDE ActiveTag = NULL_TAG;

    while(!feof(handle))
    {
        if(fgets(Token, 100, handle))
        {
            StringReformat(Token);
            WordTypeTag_TDE Tag = FindTag(Token, tags, &XMLStack);

            if(Tag == NEW_TAG)
            {
                ActiveTag = XMLStack.Tags[XMLStack.CurrentTagDepth];
                Assert(XMLStack.CurrentTagDepth >= 0);
                Assert((int)ActiveTag >= (int)FunctionWords_tag && (int)ActiveTag < (int)NUMBER_OF_TAGS);

                wordsListArray[ActiveTag].count = 0;
                continue;
            }
            else if(Tag == NON_TAG)
            {
                char *WordStorage = (char *)calloc(1,sizeof(char) * getStringLength(Token));
                CopyString(Token, WordStorage);

                wordsListArray[ActiveTag].words[wordsListArray[ActiveTag].count++] = WordStorage;
            }
            else if(Tag == NULL_TAG)
            {
                ActiveTag = XMLStack.Tags[XMLStack.CurrentTagDepth];
                continue;
            }
        }
    }
    free((void *)Token);
}

bool ReadDictionaryFromXMLConfigFile()
{
    char *Tags[NUMBER_OF_TAGS] = {"<FunctionWords>", "<Punctuation>", "<Pronoun>",\
        "<Verb>", "<Adverb>", "<Adjective>", "<Preposition>", "<Determiniers>", "<Profanities>"};

    FILE *handle = OpenFile("dictConfig.xml", "r");
    if(handle == NULL)
    {
        printf("\nFailed to open config file, please ensure the xml file is in \
                the same dir as you launched this from\n");
        return false;
    }
    else
    {
        ExtractXMLNodeContents(handle, Tags);
    }

    fclose(handle);
    return true;
}

bool isInATag(char *token, WordTypeTag_TDE tag)
{
    u32 i = 0;
    while(i++ < wordsListArray[tag].count)
    {
        if(CompareString((char*)token, wordsListArray[tag].words[i]))
        {
            return true;
        }
    }
    return false;
}

inline void AddToDocumentWordCount(DocumentWord *docWordList, u32 docCount, char *word, WordTypeTag_TDE tag)
{
    u32 index = 0;
    bool wordFound = false;
    while(index < docCount)
    {
        if(docWordList[index].tag == tag)
        {
            if(CompareString(docWordList[index].word, word))
            {
                wordFound = true;
                docWordList[index].count++;
            }
        }
        index++;
    }
    if(!wordFound)
    {
       u32 newIndex = docCount + 1;
       docWordList[newIndex].word = word;
       docWordList[newIndex].tag = tag;
       docWordList[newIndex].count = 0;
    }
}

bool ReadDocument(char *documentFilePath)
{
    FILE *handle = OpenFile(documentFilePath, "r");
    u32 documentWordCount = 0;
    DocumentWord docWordList[MAX_WORDS_IN_A_LIST];
    char *token = (char *)calloc(1, sizeof(char) * LONGEST_WORD_LENGTH);

    char *temp = token;
    while(!feof(handle))
    {
        if(fgets(token, LONGEST_WORD_LENGTH, handle))
        {
            temp = token;
            bool hasEndingPuncChar = false;
            u32 length = getStringLength(token);
            u32 tagProcessingAmount = 0;
            while(*temp++ != '\0') // rethink to actually work
            {
                if(isInATag((char*)temp[0],Punctuation_tag))
                {
                    AddToDocumentWordCount(docWordList, documentWordCount, temp, Punctuation_tag);
                    hasEndingPuncChar = true;
                    continue;
                }
            }
            if(hasEndingPuncChar)
            {
                SplitString(token, token[length-1], 0, token);
            }
            while(tagProcessingAmount++ < NUMBER_OF_TAGS)
            {
                if(isInATag(token, (WordTypeTag_TDE)tagProcessingAmount))
                {
                    AddToDocumentWordCount(docWordList, documentWordCount, token, (WordTypeTag_TDE)tagProcessingAmount);
                }
             // We need to figure out how we are going to do the other tag with the dynamic list
             //   else
             //   {
             //      AddToDocumentWordCount(docWordList, documentWordCount, token, Other_Tag);
             //   }
            }
        }
    }
    return true;
}

void main(char *args[])
{
    printf("Welcome to Spector Authorship Attribution Cpp remake");
    if(ReadDictionaryFromXMLConfigFile()) {printf("\nFile Opened Succesfully\n");}
    if(ReadDocument("testDocument.txt")) {printf("File Read fully\n");}
}
