#include "win32main.h"

globalVar WordList wordsListArray[9];

#define LONGEST_WORD_LENGTH 100
#define MAX_WORDS_IN_A_LIST 100000

bool StringEquivalent(char * first, char * second)
{
    u32 count = 0;
    while(!(first[count] == '\0' && second[count] == '\0') \
            && count < LONGEST_WORD_LENGTH)
    {
        if(first[count] != second[count])
        {
            return false;
        }
        Assert(!(first[count] == '\0' || second[count] == '\0'));
        ++count;
    }
    return true;
}

void StringReformat(char * token)
{
    char currentCharacter;
    u32 current = 0;
    u32 lastWrittenTo = 0;

    while(token[current] != '\0')
    {
        currentCharacter = token[current++];
        Assert(currentCharacter);

        if(currentCharacter == ' ' ||
           currentCharacter == '\n'||
           currentCharacter == '\t'||
           currentCharacter == '/')
        {
            continue;
        }
        else
        {
            token[lastWrittenTo++] = currentCharacter;
        }
    }
    if(lastWrittenTo > 0)
    {
        token[lastWrittenTo] = '\0';
    }
    else
    {
        token = "";
    }
}

WordTypeTag_TDE FindTag(char * token, char **tags)
{
    if(token[0] == '<')
    {
        for(u8 i = 0;
                i < NUMBER_OF_TAGS;
                ++i)
        {
            if(StringEquivalent(token, tags[i]))
            {
                return (WordTypeTag_TDE)i;
            }
        }
        return NULL_TAG;
    }
    else
    {
        return NON_TAG;
    }
}

void ExtractXMLNodeContents(FILE *handle, char **tags)
{
    rewind(handle);

    char *token = (char *)calloc(1, sizeof(char) * LONGEST_WORD_LENGTH);
    WordTypeTag_TDE ActiveTag = NULL_TAG;
    u32 countInTag = 0;
    while(!feof(handle))
    {
        if(fgets(token, 100, handle))
        {
            StringReformat(token);
            WordTypeTag_TDE Tag = FindTag(token, tags);
            if(Tag == ActiveTag)
            {
                continue;
            }
            else if(Tag == NON_TAG)
            {
                Assert(ActiveTag != NULL_TAG);
                wordsListArray[ActiveTag].words[countInTag] = token;
                ++wordsListArray[ActiveTag].count;
            }
            else
            {
                ActiveTag = Tag;
                wordsListArray[ActiveTag].count = 0;
            }
        }
    }
    free((void*)token);
}

bool ReadDictionaryFromXMLConfigFile()
{
    char *Tags[NUMBER_OF_TAGS] = {"<FunctionWords>", "<Punctuation>", "<Pronoun>",\
        "<Verb>", "<Adverb>", "<Adjective>", "<Preposition>", "<Determiniers>", "<Profanities>"};

    FILE *handle = NULL;
    fopen_s(&handle, "dictConfig.xml", "r");
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

bool isPunctuation(char token)
{
    // check wordListArray to see if this char in the puncation table
    for(u32 i = 0; i < wordsListArray[Punctuation_tag].count; i++)
    {
        if((char*)token == wordsListArray[Punctuation_tag].words[i])
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
            if(docWordList[index].word == word)
            {
                wordFound = true;
                docWordList[index].count++;
                continue;
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
    FILE *handle = NULL;
    fopen_s(&handle, documentFilePath, "r");
    if(handle == NULL)
    {
        printf("Failed to open text document, check your document path");
        return false;
    }
    u32 documentWordCount = 0;
    DocumentWord docWordList[MAX_WORDS_IN_A_LIST];
    char *token = (char *)calloc(1, sizeof(char) * LONGEST_WORD_LENGTH);

    char *temp = token;
    while(!feof(handle))
    {
        if(fgets(token, LONGEST_WORD_LENGTH, handle))
            temp = token;
            while(*temp++ != '\0') // rethink to actually work
            {
                if (isPunctuation(temp[0]))
                {
                    AddToDocumentWordCount(docWordList, documentWordCount, (char*)temp[0], Punctuation_tag);
                }
            }
    }

    return true;
}

void main(char *args[])
{
    printf("Welcome to Spector Authorship Attribution Cpp remake");
    if(ReadDictionaryFromXMLConfigFile()) {printf("\nFile Opened Succesfully\n");}
}
