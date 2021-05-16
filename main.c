/**
***********************************************************************************************
************** Criando um algorítmo que printa os nós possíveis de um puzzle ******************
***********************************************************************************************
*Obs.: Foi utilizado a ide QTCreator, sem o core do framework qt e compilado em C11.
*Jefferson Balduino - 1900659
*Paula Tanaka = 1902879
*Victor Hugo Martins de Oliveira - 2004526
*
*Obs2.: para altrar o estado inicial do puzzle e o estado final, pule para a defini��o das vari�veis
*       initMatrix e endMatrix e altere-as.
**/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <conio.h>


/////////CONFIG HEURISTICA////////////
#define DISTANCIA 1 //primeira heurística |  soma de distância de cada ladrilho até a posição final
#define SIMPLES 0 //segunda heurística | conta quantos ladrilhos estão fora da posição HAMMING

int HTYPE;
HTYPE = SIMPLES; //setando a heurística

//////////CONFIG TIPO DE PATHFINDING//////////////
#define AStar 0
#define AGulosa 1
int TYPE = AStar;
//DEFINE TYPE OF PathFinding

#define DEBUG 0
//////////CONFIG GERAL///////////
int icognitaValue = 0; //representado por 'b'
///////////////////////
typedef struct NodeList NodeList;
typedef struct Node Node;
typedef struct Vec2 Vec2;
typedef struct NodeArray NodeArray;

//////////////////////
/////CONFIGURAÇÃO/////
//////////////////////
//int initMatrix[3][3] = {
//                  {8, 6, 4},
//                  {5, 1, 0},
//                  {7, 2, 3}
//                 };

//int endMatrix[3][3] = {
//                  {8, 6, 4},
//                  {5, 0, 3},
//                  {7, 1, 2}
//                 };

int initMatrix[3][3] = {
                  {2, 8, 3},
                  {1, 6, 4},
                  {7, 0, 5}
                 };

int endMatrix[3][3] = {
                  {1, 2, 3},
                  {8, 0, 4},
                  {7, 6, 5}
                 };

NodeArray* nodeOpen;
NodeArray* nodeClosed;

NodeArray* mainArray;

int vezesQueGerou = 0;

int branchDepth = 0;

struct Vec2{
    int x, y;
};

struct Node{
    int matrix[3][3];
    bool active;
    int distFim; //inicia com -1
    int distCusto;
    NodeList * nodeListMainParent;
    NodeArray * childs;
};

struct NodeList{
    NodeArray * mainArray; // main linked list
    NodeList * nextNode;
    NodeList * prevNode;
    Node * node;
};

struct NodeArray{
    Node* mainNode;
    NodeList* head;
    NodeList* tail;
    int size;
};


NodeList* createNodeList()
{
    NodeList* p = (NodeList*) malloc(sizeof(NodeList));
    p->mainArray = NULL;
    p->nextNode = NULL;
    p->prevNode = NULL;
    p->node = NULL;
    return p;
}

Node* CreateNode()
{
    Node* n = (Node*) malloc(sizeof(Node));

    n->active = true;
    n->distFim = -1;
    n->distCusto = 0;
    n->nodeListMainParent = NULL;
    n->childs = NULL;

    return n;
}

NodeArray* createNodeArray()
{
    NodeArray* nA = (NodeArray*) malloc(sizeof(NodeArray));
    nA->head = NULL;
    nA->tail = NULL;
    nA->mainNode = NULL;
    nA->size = 0;

    return nA;
}

int getNodeIndexInArray(NodeArray* na, Node* n);
void printMatrix(Node* n );
// esta função faz o push simples de um node para um array,
// porém não transforma manterá o nodeList parent do node
void void_push(NodeArray* nodeArray, Node* node_)
{
    NodeList* nl = createNodeList();
    nl->node = node_;

    if(nodeArray == NULL)
    {
        nodeArray = createNodeArray();
    }

    nl->mainArray = nodeArray;

    if(nodeArray->head == NULL)
    {
        nodeArray->head = nl;
        nodeArray->tail = nl;
    }
    else
    {
        NodeList* tmpNodeList = nodeArray->tail;
        nodeArray->tail = nl;
        tmpNodeList->nextNode = nl;
        nl->prevNode = tmpNodeList;
    }
    nodeArray->size += 1;
}

void push(NodeArray* nodeArray, Node* node_)
{

    NodeList* nl = createNodeList();
    nl->node = node_;
    node_->nodeListMainParent = nl;

    if(nodeArray == NULL)
    {
        nodeArray = createNodeArray();
    }

    nl->mainArray = nodeArray;

    if(nodeArray->head == NULL || nodeArray == NULL)
    {
        nodeArray->head = nl;
        nodeArray->tail = nl;
    }
    else
    {
        NodeList* tmpNodeList = nodeArray->tail;
        nodeArray->tail = nl;
        tmpNodeList->nextNode = nl;
        nl->prevNode = tmpNodeList;
    }
    //nodeArray->size += 1;
}

Node* getNodeAt(NodeArray* array, int index)
{
//    if(index <= 0)
//        exit(14);

    NodeList* tmp = array->head;

    for(int i = 0; i < index; i++)
    {
        tmp = tmp->nextNode;
    }
    return tmp->node;
}

NodeList* getNodeListAt(NodeArray* array, int index)
{
//    if(index <= 0)
//        exit(14);

    NodeList* tmp = array->head;

    for(int i = 0; i < index; i++)
    {
        tmp = tmp->nextNode;
    }
    return tmp;
}
//define das direÃ¯Â¿Å“Ã¯Â¿Å“es
#define DOWN 0
#define LEFT 1
#define RIGHT 2
#define UP 3

//remove o arraylist, concerta a continuidade da array e retorna o ponteiro para NodeList, podendo trata-lo, ou deleta-lo
NodeList* removeAt(NodeArray* nArr, int index)
{
    Node* delIt = getNodeAt(nArr, index);
    NodeList* delNl = delIt->nodeListMainParent;
    NodeList* delNl_left = delNl->prevNode;
    NodeList* delNl_right = delNl->nextNode;

    if(delNl_left == NULL)
    {
        delNl_right->prevNode = NULL;
        nArr->head = delNl->nextNode;
    }
    else if(delNl_right == NULL)
    {
        delNl_left->nextNode = NULL;
        nArr->tail = delNl->prevNode;
    }
    else
    {
        delNl_right->prevNode = delNl->prevNode;
        delNl_left->nextNode = delNl->nextNode;

    }

    delNl->prevNode = NULL;
    delNl->nextNode = NULL;
//    delNl->mainArray = NULL;
    nArr->size -= 1;

    return delNl;
}

NodeList* removeNodeFromArray(Node* nn)
{
    Node* delIt = nn;
    NodeList* delNl = delIt->nodeListMainParent;
    NodeList* delNl_left;
    NodeList* delNl_right;

    NodeArray* nArr = delNl->mainArray;

    if(delNl->prevNode == NULL)
    {
        delNl_right = NULL;
    }
    else
        delNl_right = delNl->prevNode;

    if(delNl->nextNode == NULL)
    {
        delNl_left = NULL;
    }
    else
        delNl_left = delNl->nextNode;

    if(delNl_right != NULL)
    {
        if(delNl_left != NULL)
            delNl_right->prevNode = delNl_left;
        else
            delNl_right->prevNode = NULL;
    }

    if(delNl_left != NULL)
    {
        if(delNl_right != NULL)
            delNl_left->prevNode = delNl_right;
        else
            delNl_left->prevNode = NULL;
    }

    delNl->prevNode = NULL;
    delNl->nextNode = NULL;
    delNl->mainArray = NULL;
    nArr->size -= 1;

    return delNl;
}



NodeList* voidRemoveNodeFromArray(Node* nn, NodeArray* nA)
{

    //Totlmente errada

    Node* delIt = nn;
    NodeList* delNl = delIt->nodeListMainParent;
    NodeList* delNl_left;
    NodeList* delNl_right;


    if(delNl->prevNode == NULL)
    {
        delNl_right = NULL;
    }
    else
        delNl_right = delNl->prevNode;

    if(delNl->nextNode == NULL)
    {
        delNl_left = NULL;
    }
    else
        delNl_left = delNl->nextNode;

    if(delNl_right != NULL)
    {
        if(delNl_left != NULL)
            delNl_right->prevNode = delNl_left;
        else
            delNl_right->prevNode = NULL;
    }

    if(delNl_left != NULL)
    {
        if(delNl_right != NULL)
            delNl_left->prevNode = delNl_right;
        else
            delNl_left->prevNode = NULL;
    }

    delNl->prevNode = NULL;
    delNl->nextNode = NULL;
    // FIX to see if nodeList will keep with own main array variable
//    delNl->mainArray = NULL;

    nA->size--;

    return delNl;
}

//irá funcionar apenas com as variáveis nodeOpen e nodeClosed
void closeNode(Node * node)
{
    if(node->active)
    {


        //node index dentro da array openNodes
        int nodeIndex = getNodeIndexInArray(nodeOpen, node);

        NodeList* delNl = getNodeListAt(nodeOpen, nodeIndex);
//        getNodeAt()
//        nodeOpen
        /////////////////////////////

        NodeList* delNl_left = delNl->prevNode;
        NodeList* delNl_right = delNl->nextNode;

        if(delNl_left == NULL)
        {
            delNl_right->prevNode = NULL;
            nodeOpen->head = delNl->nextNode;
        }
        else if(delNl_right == NULL)
        {
            delNl_left->nextNode = NULL;
            nodeOpen->tail = delNl->prevNode;
        }
        else
        {
            delNl_right->prevNode = delNl->prevNode;
            delNl_left->nextNode = delNl->nextNode;

        }

        delNl->prevNode = NULL;
        delNl->nextNode = NULL;
    //    delNl->mainArray = NULL;
        nodeOpen->size -= 1;


        free(delNl);
        ////////////////////////////

//        nodeClosed

//        voidRemoveNodeFromArray(node, nodeOpen);// isso ta mt errado
        //removeAt(arr, index)

//        node->active = false;

//        NodeList* nL = node->nodeListMainParent;
//        free(nL);

        //node->nodeListMainParent = NULL;

        void_push(nodeClosed, node);
    }
}



// TODO free all malloc before leave
// TODO pathfinding

// TODO evitar criar ramo de arvore repetido

Vec2 procurarPosicao(Node* node, int valor);
int generateNodePossibility(Node* testNode, int posX, int posY, Node* final);

//Custo Previsto
int pesoDePosicaoDiferente(Node* atual, Node* final)
{
    int ret = 0;

    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            if(atual->matrix[i][j] != final->matrix[i][j])
            {
                ret++;
            }
        }
    }

    return ret;
}

//Manhatam
int getDistanceToEndNode(Node* nodeNow, Node* endNode, int icognita)
{
    Vec2 pos1, pos2;

    // BUG está checando apenas a icognita
    pos1 = procurarPosicao(nodeNow, icognita);
    pos2 = procurarPosicao(endNode, icognita);

    Vec2 rVec2;
    rVec2.x = abs(pos2.x - pos1.x);
    rVec2.y = abs(pos2.y - pos1.y);

    return rVec2.x + rVec2.y;
}


//GERA A os filhos
int generate(Node* nodes, int icognitas, Node* end) //generate childs
{
    int possibilidades[4];

    possibilidades[0] = -1;
    possibilidades[1] = -1;
    possibilidades[2] = -1;
    possibilidades[3] = -1;
    int index = 0;

    Vec2 posNode = procurarPosicao(nodes, icognitas);

    //int tmpValue;

    //alimenta as possiliblidades, setando valor no vetor
    //testa cantos

    //DOWN
    int valTest = posNode.y - 1;
    if(valTest >= 0 && valTest < 3)
    {
        possibilidades[index] = DOWN;
        index++;
    }

    //UP
    valTest = posNode.y + 1;
    if(valTest >= 0 && valTest < 3)
    {
        possibilidades[index] = UP;
        index++;
    }

    //LEFT
    valTest = posNode.x - 1;
    if(valTest >= 0 && valTest < 3)
    {
        possibilidades[index] = LEFT;
        index++;
    }

    //RIGHT
    valTest = posNode.x + 1;
    if(valTest >= 0 && valTest < 3)
    {
        possibilidades[index] = RIGHT;
        index++;
    }

    index++; //a partir de agora, serÃ¯Â¿Å“ usado como o tamanho do array para return

    //criando a lista
    //NodeList* tmpNodeList = 0;
    //NodeList* firstNodeList = 0;
    int j = 0;
    int k = 0;
    while(j < 4) // possÃ­veis posiÃ§Ãµes
    {
        // TODO melhorar a criação de node repetido

        if(possibilidades[j] == UP)
        {
            k+=generateNodePossibility(nodes, 0, 1, end);

        }
        else if(possibilidades[j] == LEFT)
        {
            k+=generateNodePossibility(nodes, -1, 0, end);

        }
        else if(possibilidades[j] == RIGHT)
        {
            k+=generateNodePossibility(nodes, 1, 0, end);

        }
        else if(possibilidades[j] == DOWN)
        {
            k+=generateNodePossibility(nodes, 0, -1, end);

        }
        j++;

    }

    nodes->childs->size = k;

    return k;

    /*
    saida = CreateListNode();
    saida.node = //o primeiro test que Ã¯Â¿Å“ possÃ¯Â¿Å“vel
    */

}

Vec2 procurarPosicao(Node* node, int valor)
{
    for(int x = 0; x < 3; x++)
    {
        for(int y = 0; y < 3; y++)
        {
            if(node->matrix[x][y] == valor)
            {
                Vec2 vReturn;
                vReturn.x = x;
                vReturn.y = y;
                return vReturn;
            }
        }
    }
    Vec2 v;v.x = -1;v.y = -1;
    return v;

}

Node* getHeadNodeOfArray(Node* node)
{
    Node* p = node->nodeListMainParent->mainArray->head->node;
    return p;

}

void arraycpy(Node* to, int fromMatrix[3][3])
{
    for(int x = 0; x < 3; x ++)
    {
        for(int y = 0; y < 3; y ++)
        {
            to->matrix[x][y] = fromMatrix[x][y];
//to->matrix[x][y] = from->matrix[x][y];
        }
    }
}

// if is equal, returns 0, else, returns -1
// isEqual==true?0:1;
int arraycmp(Node* to, Node* to2)
{
    for(int x = 0; x < 3; x ++)
    {
        for(int y = 0; y < 3; y ++)
        {
            if(to->matrix[x][y] != to2->matrix[x][y])
                return -1;
        }
    }
    return 0;
}

bool nodeMatrixExists(NodeArray* na, Node* n)
{
    if(na == NULL)
        return false;

    if(na->size > 0)
    {
        for(int i = 0; i < na->size; i++)
        {
            Node* tmpNode = getNodeAt(na, i);
            if(arraycmp(tmpNode, n) == 0)
            {
                return true;
            }

        }
    }

    return false;
}

int getNodeIndexInArray(NodeArray* na, Node* n)
{
    int r = 0;

    if(na == NULL)
        return false;

    if(na->size > 0)
    {
        for(int i = 0; i < na->size; i++)
        {
            Node* tmpNode = getNodeAt(na, i);
            if(arraycmp(tmpNode, n) == 0)
            {
                return r;
            }
            r++;

        }
    }

    return r;
}

// TODO if node->matrix already exist in nodeOpen or nodeClosed variable
int generateNodePossibility(Node* testNode, int posY, int posX, Node* final)
{
    //Node* fromNode = testNode->nodeListMainParent->mainArray;

//    if(testNode->active == false)
//    {
//        printf("\nErro inesperado, consulte o programador ERR::FUNCTION::generateNodePossibilities(1)\n");
//        system("PAUSE");
//        return 0;
//    }

    int qx = posX;
    int qy = posY; //valor que alterarÃ¯Â¿Å“ as posiÃ¯Â¿Å“Ã¯Â¿Å“es

    if(testNode->childs == NULL)
    {
        testNode->childs = createNodeArray();
    }

    Node* tmpNode = CreateNode();

    arraycpy(tmpNode, testNode->matrix);

    Vec2 pos = procurarPosicao(tmpNode, icognitaValue);

    int tmpVal = tmpNode->matrix[pos.x][pos.y];
    tmpNode->matrix[pos.x][pos.y] = tmpNode->matrix[pos.x+qy][pos.y+qx];
    tmpNode->matrix[pos.x+qy][pos.y+qx] = tmpVal;
//tem um erro aq

    // TODO test if exists this matrix
    if(nodeMatrixExists(nodeOpen, tmpNode) || nodeMatrixExists(nodeClosed, tmpNode))
    {
//        NodeList* tmpNL = removeNodeFromArray(tmpNode);
//        testNode->nodeListMainParent// é isso q vai mudar
    //destroy all and return
        free(tmpNode);
        // TODO free tmpNode
        return 0;
    }

    push(testNode->childs, tmpNode);

    testNode->childs->mainNode = testNode;

    void_push(nodeOpen, tmpNode);



    if(TYPE == AStar)
        tmpNode->distCusto = 1;
    else if(TYPE == AGulosa)
        tmpNode->distCusto = 0;

    tmpNode->distFim = 0;
    if(HTYPE == DISTANCIA)
    {
        for(int i = 0; i < 9; i++)
            tmpNode->distFim += getDistanceToEndNode(tmpNode, final, i);
    }
    else if(HTYPE == SIMPLES)
        tmpNode->distFim = pesoDePosicaoDiferente(tmpNode, final);




    return 1;
}


void printMatrix(Node* n )
{
    printf("\n%i | %i | %i", n->matrix[0][0], n->matrix[0][1], n->matrix[0][2]);

    printf("\n%i | %i | %i", n->matrix[1][0], n->matrix[1][1], n->matrix[1][2]);

    printf("\n%i | %i | %i", n->matrix[2][0], n->matrix[2][1], n->matrix[2][2]);

    printf("\nDistancia para a resolucao (h) : %d ", n->distFim);
    if(TYPE == AStar && HTYPE == DISTANCIA)
    {
        printf("\nDistancia para o proximo passo(g): %d ", n->distCusto);
        printf("\nCusto total da Heuristica A*(g + h): %d \n", n->distCusto+n->distFim);
    }
}



//limpa todos os caracteres \n e \0 para ' ' até que termine o vetor
void limparFimString(char* string, int tam)
{
    for (int i = 0; i < tam; i ++)
    {
        if(string[i] == '\n' || string[i] == '\0')
            string[i] = ' ';
    }
}

void printChilds(Node* n )
{
    char top[50], middle[50], bot[50], valdist[50], valanda[50], custo_final[50];

    sprintf(top, "\n0 | 0 | 0   0 | 0 | 0   0 | 0 | 0   0 | 0 | 0 ");
    sprintf(middle, "\n0 | 0 | 0   0 | 0 | 0   0 | 0 | 0   0 | 0 | 0 ");
    sprintf(bot, "\n0 | 0 | 0   0 | 0 | 0   0 | 0 | 0   0 | 0 | 0 ");
    sprintf(valdist, "\n                                              ");
    sprintf(valanda, "\n                                              ");
    sprintf(custo_final, "\n                                              ");

    int index = 1;

    int control = 1;
    int control2 = 1;
    int control3 = 1;

    for(int i = 0; i < n->childs->size; i++)
    {
        //tmpValues[7] = '\0';
        //n->childs->

        //int index = index+16 * i;
        Node* nTmp = getNodeAt(n->childs, i);
        // TODO Parse int to char - itoa
        top[index] = nTmp->matrix[0][0] + '0';
        middle[index] = nTmp->matrix[1][0] + '0';
        bot[index] = nTmp->matrix[2][0] + '0';

        index += 4;
        top[index] = nTmp->matrix[0][1] + '0';
        middle[index] = nTmp->matrix[1][1] + '0';
        bot[index] = nTmp->matrix[2][1] + '0';

        index += 4;
        top[index] = nTmp->matrix[0][2] + '0';
        middle[index] = nTmp->matrix[1][2] + '0';
        bot[index] = nTmp->matrix[2][2] + '0';

        index += 4;

        char tmp1[7] = "       ";
        sprintf(tmp1, "%d",nTmp->distFim );
        limparFimString(tmp1, 7);
        for(int j = 0; j <= 7; j++)
        {
            valdist[control] = tmp1[j];
            control++;
        }
        control += 4;

        char tmp2[7] = "       ";
        sprintf(tmp2, "%d",nTmp->distCusto );
        limparFimString(tmp2, 7);
        for(int j = 0; j <= 7; j++)
        {
            valanda[control2] = tmp2[j];
            control2++;
        }
        control2 += 4;

        char tmp3[7] = "       ";
        sprintf(tmp3, "%d",(nTmp->distCusto + nTmp->distFim));
        limparFimString(tmp3, 7);
        for(int j = 0; j <= 7; j++)
        {
            custo_final[control3] = tmp3[j];
            control3++;
        }
        control3 += 4;


    }

    limparFimString(valdist, 50);
    limparFimString(valanda, 50);
    limparFimString(custo_final, 50);

    top[index] = '\0';
    middle[index] = '\0';
    bot[index] = '\0';
    valdist[control] = '\0';
    valanda[control2] = '\0';
    custo_final[control3] = '\0';

    valdist[control-1] = '$';
    valanda[control2-1] = '#';
    custo_final[control3-1] = '@';

    valdist[0] = '\n';
    valanda[0] = '\n';
    custo_final[0] = '\n';

    printf("%s", top);
    printf("%s", middle);
    printf("%s", bot);
    printf("%s", valdist);
    if(TYPE == AStar && HTYPE == DISTANCIA)
    {
        printf("%s", valanda);
        printf("%s\n", custo_final);
    }


//    printf("\n%i | %i | %i \t %i | %i | %i \t %i | %i | %i \t %i | %i | %i \t");

//    printf("\n%i | %i | %i", n->matrix[0][0], n->matrix[0][1], n->matrix[0][2]);

//    printf("\n%i | %i | %i", n->matrix[1][0], n->matrix[1][1], n->matrix[1][2]);

//    printf("\n%i | %i | %i", n->matrix[2][0], n->matrix[2][1], n->matrix[2][2]);

//    printf("\nDistanca: %d \n", n->distFim);
}

///
/// \brief getMenorValor de childs do node
/// \param nde
/// \return o node com o menor valor
/// é percorrido os childs e retornado o child com o menor valor de distancia
/// posso fazer uma array que vai apontando para os nodes, depois destruo os objetos
Node* getMenorValor(Node* nde)
{
    Node* nLess = getNodeAt(nde->childs, 0);

    for(int i = 0; i < nde->childs->size; i++)
    {
        Node* ar_node = getNodeAt(nde->childs, i);
        int custoTotal = ar_node->distCusto + ar_node->distFim;
        if(custoTotal < (nLess->distCusto + nLess->distFim))
        {
            nLess = ar_node;
        }

    }
    return nLess;
}

Node* getMenorValorNA(NodeArray* nde)
{
    Node* nLess = getNodeAt(nde, 0);

    for(int i = 0; i < nde->size; i++)
    {
        Node* ar_node = getNodeAt(nde, i);
        int custoTotal = ar_node->distCusto + ar_node->distFim;
        if(custoTotal < (nLess->distCusto + nLess->distFim))
        {
            nLess = ar_node;
        }

    }
    return nLess;
}

// TODO criar no ref push noRefPush()
// esta função faz o push em nodes, mas não altera o main NodeList do Node
//
Node* FindPath(Node* nodeNow, Node* end)
{
    //checa se achou
    int distTotal = nodeNow->distCusto + nodeNow->distFim;
    if(distTotal == 1 || distTotal == 0)
    {
        return nodeNow;
    }
    else
    {
        static int vezesQueGerous = 0;
//        int ic = icognitaValue; 0 hard coded
        generate(nodeNow, 0, end);


        printf("\n\n--Selected--");
        printMatrix(nodeNow);
        printf("\n--Childs--");
        printChilds(nodeNow);

//        vezesQueGerous +=

        closeNode(nodeNow);
        //buga quando gera o último

        if(distTotal > 1){

            Node* nextN = getMenorValorNA(nodeNow->childs);

            // TODO change to that
//            Node* nextN = getMenorValorNA(nodeOpen);


            return FindPath(nextN, end);
        }
        return nodeNow;
        //procura o node de menor valor no array aberto
        //chama FindPath no menor node encontrado
    }
}

int nodesTotaisGerados()
{

//    NodeArray* na = nodeOpen;
//    NodeArray* nb = nodeClosed;
//    return nb->size;
//    int total = (nodeOpen->size + nodeClosed->size);
//    return total;
//    return nodeClosed->size;
}

int printBestPath(Node* foundNode, int val)
{
    int r = val;

    printMatrix(foundNode);
    printf("\n");

    if(foundNode->nodeListMainParent->mainArray != NULL)
    {
        int nodeIndex = getNodeIndexInArray(foundNode->nodeListMainParent->mainArray, foundNode);


        if(foundNode->nodeListMainParent->mainArray->mainNode != NULL)
        {
            Node* wr = foundNode->nodeListMainParent->mainArray->mainNode;
            return printBestPath(wr, ++r);
        }
    }

    return val;
}

int main()
{

    char r;
    bool done = true;

    if(DEBUG != 1)
    {
        while(done)
        {
            printf("Escolha a Heuristica:\n");
            printf("1. HEURISTICA MANHATAN\n");
            printf("2. HEURISTICA HAMMING\n");
            fflush(stdin);
            scanf("%c", &r);

            switch (r) {
            case '1':
                HTYPE = DISTANCIA;
                done = false;
                break;
            case '2':
                HTYPE = SIMPLES;
                done = false;
                break;
            default:
                printf("\n\nEscolha 1 ou 2.\n\n");
                break;
            }

        }
        done = true;
        char r2;
        while(done)
        {
            printf("Escolha o tipo:\n");
            printf("1. A Estrela\n");
            printf("2. A Gulosa\n");
            fflush(stdin);
            fflush(stdout);
            scanf("%c", &r2);

            switch (r2) {
            case '1':
                TYPE = AStar;
                done = false;
                break;
            case '2':
                TYPE = AGulosa;
                done = false;
                break;
            default:
                printf("\n\nEscolha 1 ou 2.\n\n");
                break;
            }

        }
    }
    system("cls");
    //printf("\x1B[31mTexting\033[0m\t\t");

    printf("\n--CONFIGURACOES:--\n");
    (HTYPE == DISTANCIA) ? printf("Heuristica de Manhatan.\n") :
                           printf("Heuristica de Hamming.\n");
    (TYPE == AStar) ? printf("Algoritmo A estrela.\n") :
                           printf("Algoritmo A gulosa.\n");


    printf("\n###LEGENDA###\n");

    printf("$ : h(n)\n");
    if(TYPE == AStar && HTYPE == DISTANCIA)
    {
        printf("# : g(n)\n");
        printf("@ : g(n)+h(n)\n");
    }
    printf("---------------\n\n");

    Node* mainNode = CreateNode();

    arraycpy(mainNode, initMatrix);

    Node* endNode = CreateNode();

    arraycpy(endNode, endMatrix);

    NodeArray* no = createNodeArray();
    nodeOpen = no;

    NodeArray* nc = createNodeArray();
    nodeClosed = nc;

    push(mainArray, mainNode);
    void_push(nodeOpen, mainNode);

    //mainNode->distFim = pesoDePosicaoDiferente(mainNode, endNode);

    mainNode->distFim = 0;
    if(HTYPE == DISTANCIA)
    {
        for(int i = 0; i < 9; i++)
            mainNode->distFim += getDistanceToEndNode(mainNode, endNode, i);
    }
    else if(HTYPE == SIMPLES)
        mainNode->distFim = pesoDePosicaoDiferente(mainNode, endNode);


    printf("PUZZLE INICIAL");

    printMatrix(mainNode);

    printf("\n\nPUZZLE FINAL");

    printMatrix(endNode);

    printf("\n\n---------------------\nINICIANDO BUSCA:");
    //FindPath! WOooooooooooooooooow 8h +- até aqui
    Node* foundNode = FindPath(mainNode, endNode);


//    int total = nodesTotaisGerados();
    printf("\n\nACHOU O MELHOR CAMINHO!\n");

    printf("TOTAL DE NODES GERADOS: %d\n\n", nodeClosed->size + nodeOpen->size);

    //printBestPath(foundNode, 0);

//    int a = 0;

//    printf("%d", mainArray->size);
//    generate(mainNode, 0, endNode);

//    int size = mainNode->childs->size;
//    NodeList * nl = mainNode->childs->head;

//    printChilds(mainNode);

//    for(int i = 0; i < size; i++)
//    {
//        printMatrix(nl->node->matrix);
//        if(i<size-1) nl = nl->nextNode;
//        printf("\n");
//    }

//    printf("\n%x", mainNode->childs->head->node->matrix);

    //printf("\ntamanho: %i", mainNode->childs->size);
    //printMatrix(mainNode->childs->head->node);

    system("PAUSE");
    return 0;
}
