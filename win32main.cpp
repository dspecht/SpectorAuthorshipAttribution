#include "win32main.h"

FILE* OpenFile(char *filePath, char *openFMT="rw")
{
    FILE *handle = NULL;
    fopen_s(&handle, filePath, openFMT);
    Assert(handle != NULL);
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
    }
    else
    {
        Node *ReferenceNode = Tree->CurrentNode->Child;
        while(ReferenceNode->RightSibling)
        {
            ReferenceNode = ReferenceNode->RightSibling;
        }
        ReferenceNode->RightSibling = NewNode(Token);
        ReferenceNode->RightSibling->LeftSibling = ReferenceNode;
        ReferenceNode->RightSibling->Parent = ReferenceNode->Parent;
        Tree->CurrentNode = ReferenceNode->RightSibling;
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
    ActiveList->words[ActiveList->count++]->word = WordLocation;
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
    bool RecentlyPopped = false;
    while(!feof(handle))
    {
        if(fgets(Token, 100, handle))
        {
            StringReformat(Token);
            CheckXMLTags(Token, &OpeningTag, &ClosingTag);
            if(RecentlyPopped)
            {
                RecentlyPopped = false;
                continue;
            }
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
                    if(!XMLTree->CurrentNode->Parent)
                    {
                        XMLTree->CurrentNode->RightSibling = NewNode(Token);
                        XMLTree->CurrentNode->RightSibling->LeftSibling = XMLTree->CurrentNode;
                        XMLTree->CurrentNode = XMLTree->CurrentNode->RightSibling;
                        break;
                    }
                    else
                    {
                        PopToParent(XMLTree, Token);
                        RecentlyPopped = true;
                    }
                }
            }
        }
    }
    free((void *)Token);
}

Node * ReverseTree(Node *node)
{
    if(node->RightSibling)
    {
        return node->RightSibling;
    }
    else
    {
        if(node->Parent)
        {
            return ReverseTree(node->Parent);
        }
        else
        {
            return 0;
        }
    }
}

Node * TraverseTree(Node * node)
{
    if(node->Child)
    {
        return node->Child;
    }
    else
    {
        if(node->RightSibling)
        {
            return node->RightSibling;
        }
        else
        {
            return ReverseTree(node);
        }
    }
}

bool CheckCategory(char *Word, WordList *List)
{
    for(u32 ListIndex = 0;
        ListIndex < List->count;
        ++ListIndex)
    {
        char *WordToCheck = List->words[ListIndex]->word;
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
    u32 NodesTraversed = 0;
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
                    break;
                }
            }
        }
        else
        {
            TraverseTree(TraversalNode);
        }
        ++NodesTraversed;
        if(NodesTraversed > Tree->NumberOfNodes)
        {
            break;
        }
    }
    return 0;
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

void FindAndIcreament(NodeTree *tree, char *token, u32 documentIndex)
{
    r32 found = false;
    u32 nodesTraversed = 0;
    Node *traversalNode = tree->RootNode;
    while(!found)
    {
        if(traversalNode->WordsInCategory)
        {
            WordList *list = traversalNode->WordsInCategory;

            for(u32 index=0; index < list->count; ++index)
            {
                char *wordToCheck = list->words[index]->word;
                if(CompareString(wordToCheck, token))
                { ++list->words[index]->count[documentIndex]; }
            }
        }
        else {TraverseTree(traversalNode);}
        ++nodesTraversed;
        if(nodesTraversed > tree->NumberOfNodes) {break;}
    }
}

bool ReadDocument(char *documentFilePath, NodeTree * XMLTree, u32 documentIndex) //TODO:(dustin) Do a Char by Char read of the file to create the words
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
    char *tempString = (char *)calloc(1, sizeof(char) * LONGEST_WORD_LENGTH);
    char temp = NULL;
    u32 tagProcessingAmount = 0;

    Assert(handle != NULL); // remove later
    while(!feof(handle))
    {
        u32 tagProcessingAmount = 0;

        while(temp != ' ')
        {
            //Think how to do the puncation that it does not
            //Disrupt the flow and not add twice tree traversals
            temp = (char)fgetc(handle);
            if(temp == '\n' || temp == '\t') {temp = NULL;}
			if (temp != NULL)
            {
                Assert(temp != 0);
                CatString(tempString, (char*)&temp, token);
            }
        }

        Assert(token != NULL);
        FindAndIcreament(XMLTree, token, documentIndex);
        token = {}; //Clear
    }
    fclose(handle);
    return true;
}

void main(char *args[])
{
    printf("Welcome to Spector Authorship Attribution Cpp remake");
    NodeTree * ParsedXML = ReadDictionaryFromXMLConfigFile();

    if(ParsedXML) {printf("\nFile Opened Succesfully\n");}

    if(ReadDocument("testDocument.txt",ParsedXML,0)) {printf("File Read fully\n");}
}
