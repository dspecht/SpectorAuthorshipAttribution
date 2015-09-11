#include "win32main.h"

globalVar WordList wordsListArray[9];

void ExtractXMLNodeContents(FILE *handle, char *tag, char *endTag, u8 WLIndex)
{
    //NOTE do we really need ftell here? it was recomended on the fseek MSDN page

    fseek(handle, ftell(handle), SEEK_SET); // move to start of file
    char *token = NULL;
    while(token != tag) { fscanf_s(handle, "%s", token); }
    Assert(token == tag);
    fscanf_s(handle, "%s", token); // move token past tag to get wanted info
    do
    {
        wordsListArray[WLIndex].words[wordsListArray[WLIndex].count] = token;
        wordsListArray[WLIndex].count++;

        fscanf_s(handle, "%s", token);
    }
    while(token != endTag);
}

bool ReadDictionaryFromXMLConfigFile()
{
    char *TagsOpen[9] = {"<FunctionWords>", "<Punctuation>", "<Pronoun>", "<Verb>", "<Adverb>" "<Adjective>", "<Preposition>", "<Determiniers>", "<Profanities>"};
    char *TagsEnd[9] = {"</FunctionWords>","</Punctuation>","</Pronun>","</Verb>","</Adverb>","</Adjective>","</Preposition>","</Determiniers>","</Profanities>"};

    FILE *handle = NULL;
    fopen_s(&handle, "dictConfig.xml", "r");
    if(handle == NULL) { printf("\nFailed to open config file, please ensure \
            the xml file is in the same dir as you launched this from\n");
        return false; }
    for(u8 i = 0; i < (NUMBER_OF_TAGS+1); i++)
    {
        ExtractXMLNodeContents(handle, TagsOpen[i], TagsEnd[i], i);
    }

    fclose(handle);
    return true;
}

void main(char *args[])
{
    printf("Welcome to Spector Authorship Attribution Cpp remake");
    if(ReadDictionaryFromXMLConfigFile()) {printf("\nFile Opened Succesfully\n");}
}
