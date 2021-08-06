#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h> 

typedef struct tree {
  int count;
  char character;
  struct tree *right;
  struct tree *left;
} tree;

typedef struct queue {
    int number;
    struct queue *ref;
} queueCode;

typedef struct arrayCode {
    char ch;
    struct queue *ref;
} arrayCode;

const int ASCII_a = 97, ILOSC_ZNAKOW = 27;
void zlicza(FILE *sentence, tree* huffmann[], int *lengthHuffmann);
tree *allocRoot(char ch);
void swap(tree *a, tree *b);
tree *merge(tree *a, tree *b);
void pushMerged(tree* arr[], int *n, tree *newMerged);
tree *extractMin(tree* arr[], int *n);
void huffmanDelete(tree *huffmann);
int left(int i);
int right(int i);
void minHeapify(tree* arr[], int n, int i);
void buildMinHeap(tree* arr[], int n);
void *createFullTree(tree* arr[], int *n);
queueCode *allocQueue();
void printCode(tree *T, arrayCode arr[], int *i);
void coding(tree *node, queueCode *root, queueCode *head, arrayCode arr[], int *i);
void printQueue(queueCode *root, queueCode *code);
void queueDelete(queueCode *root);

int main()
{
    FILE *sentence = fopen("sentence.txt", "r");    // otwiera plik do odczytu (musi istniec)
    if (sentence == NULL)
    {
        perror("Nie udalo sie otworzyc pliku sentence.txt");
        return 1;
    }
    int lengthHuffmann = 0;
    tree *huffmann[ILOSC_ZNAKOW];  //Inicjalizuje tablice drzew.
    zlicza(sentence, huffmann, &lengthHuffmann); //Tworzy tablicę rootów
    buildMinHeap(huffmann, lengthHuffmann); //Tworzy kolejkę priorytetową
    createFullTree(huffmann, &lengthHuffmann); //Tworzy drzewo rozpinające
    FILE *encrypted = fopen( "encrypted.txt", "w");
    arrayCode arr[ILOSC_ZNAKOW]; // tablica do trzymania słownika
    int i = 0;
    if ((huffmann[0]->left != NULL) || (huffmann[0]->right !=NULL)) {
        printCode(huffmann[0], arr, &i);
    }
    else {                                  // Przypadek dla jednej wartości
        arr[0].ch = huffmann[0]->character;
        arr[0].ref = allocQueue();
        arr[0].ref->ref = allocQueue();
        arr[0].ref->ref->number = 1;
        i = 1;
    }
    huffmanDelete(huffmann[0]); // czyści drzewo rozpinające

    int tmp;
    for(tmp = 0; tmp < i; tmp++){               // Zapisuje słownik do pliku wyjściowego
        queueCode *root = arr[tmp].ref->ref;
        fprintf(encrypted, "%c=", arr[tmp].ch);
        while(root) {
            fprintf(encrypted, "%d", root->number);
            root = root->ref;
        }
        fprintf(encrypted, "\n");
    }
    fprintf(encrypted, "\n");
    int set = fseek(sentence, 0, SEEK_SET);
    queueCode *encrySent = allocQueue();
    queueCode *index = encrySent;
    index->ref = allocQueue();
    char ch = getc( sentence );
    while(ch != EOF) {                              //Tworzy wiersz zakodowany słownikiem
        for(tmp = 0; arr[tmp].ch != ch; tmp++) {
        }
        queueCode *root = arr[tmp].ref->ref;
        while(root) {
            index->ref->number = root->number;
            index->ref->ref = allocQueue();
            index=index->ref;
            root = root->ref;
        }
        ch = getc( sentence );
    }
    free(index->ref);
    index->ref = NULL;
    for(tmp = 0; tmp < i; tmp++){
        queueDelete(arr[tmp].ref);
    }
    index = encrySent->ref;
    int byte;
    int count;
    while(index) {                                  // Konwertowanie z binarnego -> tekstu zakodowanego.
        byte = 0;
        for(tmp = 7; index && tmp >= 0; tmp--){
            if (index->number){
                byte += pow(2,tmp);
            }
            index=index->ref;
        }
            fprintf(encrypted, "%c", byte);
    }
    queueDelete(encrySent);
    fclose(sentence);
    fclose(encrypted);
    return 0;
}

void zlicza(FILE *sentence, tree* huffmann[], int *lengthHuffmann) {   // Tworzenie tablicy huffmanna ze zliczonymi literami i znakami 
    int i, j;
    char ch = getc( sentence );
    for (i = 0; ch != EOF; i++) {  // Przejście po tablicy zdanie i tworzenie zliczonej huffmanna
        bool flag = false;
        for (j = 0; (j < *lengthHuffmann) && (!flag) ; j++) {
            if (ch == huffmann[j]->character) {                 // Dodaje nowe wystąpienie litery
                flag = true;
                huffmann[j]->count++;
            }
        }
        if (!flag) {                        // Alokowanie kolejnej jeszcze niespotkanej litery do kolejki Priorytetowej
            huffmann[j] = allocRoot(ch);
            (*lengthHuffmann)++;
        }
        ch = getc( sentence );
    }
}

tree *allocRoot(char ch)
{
	tree *v;
	
	if( (v = (tree *)malloc(sizeof(tree))) != NULL )
	{
		v->character = ch;
		v->count = 1;
		v->left = v->right = NULL;
	}
	return(v);
}

void swap(tree *a, tree *b) {
tree temp = *a;
*a = *b;
*b = temp;
}

tree *merge(tree *a, tree *b) {
    tree *merged = allocRoot('!');
    merged->count = a->count + b->count;
    merged->left = a;
    merged->right = b;
    return merged;
}

void pushMerged(tree* arr[], int *n, tree *newMerged) {
    int i = *n;
    *n += 1;
    int j = (i - 1) / 2;
    arr[i] = newMerged;
    while (i > 0 && arr[i]->count < arr[j]->count) {
        swap(arr[i], arr[j]);
        i = j;
        j = (i - 1) / 2;
    }
}

tree *extractMin(tree* arr[], int *n) {
    if (*n>0) {
        swap(arr[0], arr[*n-1]);
        --*n;
        minHeapify(arr, *n, 0);
        return arr[*n];
    }
}

void huffmanDelete(tree *huffmann) {
    if (huffmann) {
        huffmanDelete(huffmann->left);
        huffmanDelete(huffmann->right);
        free(huffmann);
    }
}

void minHeapify(tree* arr[], int n, int i) 
{ 
    int l = left(i); 
    int r = right(i); 
    int smallest = i;
    if (l < n && arr[l]->count < arr[i]->count) 
        smallest = l;
    if (r < n && arr[r]->count < arr[smallest]->count) 
        smallest = r;
    if (smallest != i) { 
        swap(arr[i], arr[smallest]); 
        minHeapify(arr, n, smallest);
    } 
}

void buildMinHeap(tree* arr[], int n) {
int i;
for (i = n / 2 - 1; i >= 0; i--)
  minHeapify(arr, n, i);
}

int left(int i) { 
    return (2*i + 1); 
} 

int right(int i) { 
    return (2*i + 2); 
} 

void *createFullTree(tree* arr[], int *n) {
    int i = *n;
    for ( ; i > 1; i--){
        tree *tmp1 = extractMin(arr, n);
        tree *tmp2 = extractMin(arr, n);
        pushMerged(arr, n, merge(tmp1, tmp2));
    }
}

queueCode *allocQueue()
{
	queueCode *v;
	
	if( (v = (queueCode *)malloc(sizeof(queueCode))) != NULL )
	{
		v->ref = NULL;
		v->number = 5;
	}
	return(v);
}

void queueDelete(queueCode *root) {
    if (root) {
        queueDelete(root->ref);
        free(root);
    }
}

void printQueue(queueCode *root, queueCode *code) {
    if (root) {
        code->ref->number = root->number;
        code->ref->ref = allocQueue();
        printQueue(root->ref, code->ref);
    } else {
        free(code->ref);
        code->ref=NULL;
    }
}

void coding(tree *node, queueCode *root, queueCode *head, arrayCode arr[], int *i) {        //Czytanie drzewa i tworzy słownik
    if (node->left != NULL) {
        root->ref->number = 0;
        root->ref->ref = allocQueue();
        coding(node->left, root->ref, head, arr, i);
    }
    if (node->right != NULL) {
         root->ref->number = 1;
        root->ref->ref = allocQueue();
        coding(node->right, root->ref, head, arr, i);
    } else {
        free(root->ref);
        root->ref=NULL;
        arr[*i].ch = node->character;
        arr[*i].ref = allocQueue();
        arr[*i].ref->ref = allocQueue();
        printQueue(head->ref, arr[*i].ref);
        ++(*i);
    }
}

void printCode(tree *T, arrayCode arr[], int *i) {
    queueCode *root = allocQueue();
    queueCode *head = root;
    root->ref = allocQueue();
    coding(T, root, head, arr, i);
    free(head);
}