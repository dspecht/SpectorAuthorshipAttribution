#include "win32main.h"

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

void StripTags(char *Token)
{
    char CurrentCharacter;
    int Current = 0;
    int LastWrittenTo = 0;

    while(Token[Current] != '\0')
    {
        CurrentCharacter = Token[Current++];
        Assert(CurrentCharacter);

        if(CurrentCharacter == '>' ||
           CurrentCharacter == '<'||
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

Node * NewNode(char *Token)
{
    Node *NewNode = (Node *)calloc(1, sizeof(Node));
    CopyString(Token, (char *)NewNode->Tag);
    NewNode->RightSibling = NewNode;
    NewNode->LeftSibling = NewNode;

    return NewNode;
}

WordList * InitializeWordList()
{
    WordList *List = (WordList *)calloc(1, sizeof(WordList));
    List->count = 0;
    return List;
}

void PopToParent(NodeTree *Tree, char *Token)
{
    Assert(CompareString(Token, (char *)Tree->CurrentNode->Tag));
    Assert(Tree->CurrentNode->Tag != Tree->RootNode->Tag);

    Tree->CurrentNode = Tree->CurrentNode->Parent;
}

void PushToChild(NodeTree *Tree, char * Token)
{
    if(!Tree->CurrentNode->Child)
    {
        Tree->CurrentNode->Child = NewNode(Token);
        Tree->CurrentNode->Child->Parent = Tree->CurrentNode;
        Tree->CurrentNode = Tree->CurrentNode->Child;

        Tree->CurrentNode->LeftSibling = Tree->CurrentNode;
        Tree->CurrentNode->RightSibling = Tree->CurrentNode;
    }
    else
    {
        //create a circular linked list for sibling nodes so that
        //there is always only one node to traverse to add a sibling
        //parents can only point to one child and must traverse the linked
        //list to find all siblings
        Node *ReferenceNode = Tree->CurrentNode->Child;
        ReferenceNode->LeftSibling->RightSibling = NewNode(Token);
        ReferenceNode->LeftSibling->RightSibling->LeftSibling = ReferenceNode->LeftSibling;
        ReferenceNode->LeftSibling = ReferenceNode->LeftSibling->RightSibling;
        ReferenceNode->LeftSibling->Parent = Tree->CurrentNode;

        Tree->CurrentNode = ReferenceNode->LeftSibling;
    }
    ++Tree->NumberOfNodes;
}

void AddToList(Node *ActiveNode, char *Token)
{
    if(!ActiveNode->WordsInCategory)
    {
        ActiveNode->WordsInCategory = InitializeWordList();
    }

    char *WordLocation = (char *)calloc(1,sizeof(char) * getStringLength(Token));
    WordList *ActiveList = ActiveNode->WordsInCategory;
    CopyString(Token, WordLocation);
    ActiveList->words[ActiveList->count++] = WordLocation;
}

void CheckXMLTags(char *Token, bool *OpeningTag, bool *ClosingTag)
{
    *OpeningTag = false;
    *ClosingTag = false;
    if(Token[0] == '<')
    {
        if(Token[1] == '/')
        {
            *ClosingTag = true;
        }
        else
        {
            *OpeningTag = true;
        }
    }
    StripTags(Token);
}

void ExtractXMLNodeContents(FILE *handle, NodeTree * XMLTree)
{
    rewind(handle);
    char *Token = (char *)calloc(1,sizeof(char) * LONGEST_WORD_LENGTH);

    bool OpeningTag = false;
    bool ClosingTag = false;
    while(!feof(handle))
    {
        if(fgets(Token, 100, handle))
        {
            StringReformat(Token);
            CheckXMLTags(Token, &OpeningTag, &ClosingTag);
            if(!(OpeningTag || ClosingTag))
            {
                AddToList(XMLTree->CurrentNode, Token);
            }
            else
            {
                if(OpeningTag)
                {
                    if(!XMLTree->RootNode)
                    {
                        XMLTree->RootNode = NewNode(Token);
                        XMLTree->CurrentNode = XMLTree->RootNode;
                        ++XMLTree->NumberOfNodes;
                    }
                    else
                    {
                        PushToChild(XMLTree, Token);
                    }
                }
                if(ClosingTag)
                {
                    Assert(XMLTree->CurrentNode);
                    if(XMLTree->CurrentNode->Tag == XMLTree->RootNode->Tag)
                    {
                        //CreateSibling
                        break;
                    }
                    else
                    {
                        PopToParent(XMLTree, Token);
                    }
                }
            }
        }
    }
    free((void *)Token);
}

Node * TraverseTree(Node * node)
{
    if(node->RightSibling == node)
    {
        if(node->Parent)
        {
            return 0;
        }
        else
        {
            return TraverseTree(node->Parent);
        }
    }
    else
    {
        return node->RightSibling;
    }
}

bool CheckCategory(char *Word, WordList *List)
{
    char **WordList = List->words;
    for(int ListIndex = 0;
        ListIndex < List->count;
        ++ListIndex)
    {
        char *WordToCheck = WordList[ListIndex];
        if(CompareString(WordToCheck,Word))
        {
            if(CompareString(Word, WordToCheck))
            {
                return true;
            }
        }
    }
    return false;
}

char * FindWordCategory(NodeTree * Tree, char * Word)
{
    bool Found = false;
    int NodesTraversed = 0;
    Node *TraversalNode = Tree->RootNode;
    while(!Found)
    {
        if(TraversalNode->WordsInCategory)
        {
            WordList *List = TraversalNode->WordsInCategory;
            if(CheckCategory(Word, List))
            {
                return TraversalNode->Tag;
            }
            else
            {
                TraversalNode = TraverseTree(TraversalNode);
                if(!TraversalNode)
                {
                    return false;
                }
            }
        }
        ++NodesTraversed;
        if(NodesTraversed > Tree->NumberOfNodes)
        {
            Found = false;
        }
    }
    return false;
}

NodeTree * ReadDictionaryFromXMLConfigFile()
{
    NodeTree * ParsedXML = (NodeTree *)calloc(1,sizeof(NodeTree));
    FILE *handle = OpenFile("dictConfig.xml", "r");
    if(handle == NULL)
    {
        printf("\nFailed to open config file, please ensure the xml file is in \
                the same dir as you launched this from\n");
        return 0;
    }
    else
    {
        ExtractXMLNodeContents(handle, ParsedXML);
    }

    fclose(handle);
    return ParsedXML;
}

inline void AddToDocumentWordCount(DocumentWord *docWordList, u32 docCount, char *word, char * tag)
{
    u32 index = 0;
    bool wordFound = false;
    while(index < docCount)
    {
        if(docWordList[index].tag == tag)
        {
            Assert(word != NULL);
            Assert(docWordList[index].word != NULL);
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

bool ReadDocument(char *documentFilePath, NodeTree * XMLTree) //TODO:(dustin) Do a Char by Char read of the file to create the words
{
    FILE *handle = OpenFile(documentFilePath, "r");
    if(handle == NULL)
    {
        printf("\nFailed to open %s file, please ensure the xml file is in \
                the same dir as you launched this from\n", documentFilePath);
        return false;
    }
    rewind(handle);

    u32 documentWordCount = 0;
    //NOTE:(down) Should not have to do this but ya know how bad things have gotten
    DocumentWord *docWordList = (DocumentWord*)calloc(1, sizeof(DocumentWord) * MAX_WORDS_IN_A_LIST);
    char *token = (char *)calloc(1, sizeof(char) * LONGEST_WORD_LENGTH);
    char temp = NULL;
    u32 tagProcessingAmount = 0;

    Assert(handle != NULL); // remove later
    while(!feof(handle))
    {
        u32 tagProcessingAmount = 0;

        while(temp != ' ')
        {
            temp = (char)fgetc(handle);

            //TODO: Find a way to do punctuation that does not cause loss of data
            //if(isInATag((char*)temp,Punctuation_tag)) {return (char*)temp;}
            if(temp == '\n' || temp == '\t') {temp = NULL;}
            if(!temp == NULL)
            {
                CatString(token, (char*)temp, token);
            }
        }

 //       StripTags(token);  //Why do we need this here?

        char *Tag = FindWordCategory(XMLTree, token);
        if(Tag)
        {
            AddToDocumentWordCount(docWordList, documentWordCount, token, Tag);
            break;
        }
        token = {};//clear to all 0's so we don't have left over chars that could mess up the next check
    }

    fclose(handle);
    return true;
}

void main(char *args[])
{
    printf("Welcome to Spector Authorship Attribution Cpp remake");
    NodeTree * ParsedXML = ReadDictionaryFromXMLConfigFile();

    if(ParsedXML) {printf("\nFile Opened Succesfully\n");}

    if(ReadDocument("testDocument.txt",ParsedXML)) {printf("File Read fully\n");}
}
