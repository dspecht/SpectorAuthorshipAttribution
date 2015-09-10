#include <cstdio>
#include "win32main.h"

globalVar WordList wordsListArray[6];

void ExtractXMLNodeContents(FILE *handle, char *tag, char *endTag, u8 wordListArrayIndex)
{
    fseek(handle, ftell(handle), SEEK_SET);//NOTE Put us at the start of the stream
    char *token = NULL;
    while(token != tag) { fscanf_s(handle, "%s", token); }
    while(token != endTag)
    {

    }

}

bool ReadDictionaryFromXMLConfigFile()
{
    FILE *handle = NULL;
    fopen_s(&handle, "dictConfig.xml", "r");
    if(handle == NULL) { printf("\nFailed to open config file\n"); return false; }



    return true;
}

void main(char *args[])
{
    printf("Welcome to Spector Authorship Attribution Cpp remake");
    if(ReadDictionaryFromXMLConfigFile()) {printf("\nFile Opened Succesfully\n");}
}
