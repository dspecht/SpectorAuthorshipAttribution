#include "win32main.h"

globalVar WordList wordsListArray[9];

#define LONGEST_WORD_LENGTH 100

bool StringEquivalent(char * First, char * Second)
{
    u32 Count = 0;
    while(!(First[Count] == '\0' && Second[Count] == '\0') \
            && Count < LONGEST_WORD_LENGTH)
    {
        if(First[Count] != Second[Count])
        {
            return false;
        }
        Assert(!(First[Count] == '\0' || Second[Count] == '\0'));
        ++Count;
    }
    return true;
}

void StringReformat(char * Token)
{
    char CurrentCharacter;
    u32 Current = 0;
    u32 LastWrittenTo = 0;

    while(Token[Current] != '\0')
    {
        CurrentCharacter = Token[Current++];
        Assert(CurrentCharacter);

        if(CurrentCharacter == ' ' ||
           CurrentCharacter == '\n'||
           CurrentCharacter == '\t'||
           CurrentCharacter == '/')
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

WordTypeTag_TDE FindTag(char * Token, char **tags)
{
    if(Token[0] == '<')
    {
        for(u8 i = 0;
                i < NUMBER_OF_TAGS;
                ++i)
        {
            if(StringEquivalent(Token, tags[i]))
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

    char *Token = (char *)calloc(1, sizeof(char) * LONGEST_WORD_LENGTH);
    WordTypeTag_TDE ActiveTag = NULL_TAG;
    u32 CountInTag = 0;
    while(!feof(handle))
    {
        if(fgets(Token, 100, handle))
        {
            StringReformat(Token);
            WordTypeTag_TDE Tag = FindTag(Token, tags);
            if(Tag == ActiveTag)
            {
                continue;
            }
            else if(Tag == NON_TAG)
            {
                Assert(ActiveTag != NULL_TAG);
                wordsListArray[ActiveTag].words[CountInTag] = Token;
                ++wordsListArray[ActiveTag].count;
            }
            else
            {
                ActiveTag = Tag;
                wordsListArray[ActiveTag].count = 0;
            }
        }
    }
    free((void*)Token);
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

void main(char *args[])
{
    printf("Welcome to Spector Authorship Attribution Cpp remake");
    if(ReadDictionaryFromXMLConfigFile()) {printf("\nFile Opened Succesfully\n");}
}
