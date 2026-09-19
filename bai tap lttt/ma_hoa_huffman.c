#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TREE_HT 256

// Node của cây Huffman
struct HuffmanNode {
    char data;
    unsigned freq;
    struct HuffmanNode *left, *right;
};

// MinHeap
struct MinHeap {
    unsigned size;
    unsigned capacity;
    struct HuffmanNode **array;
};

// Tạo node mới
struct HuffmanNode* newNode(char data, unsigned freq) {
    struct HuffmanNode* node = (struct HuffmanNode*)malloc(sizeof(struct HuffmanNode));
    node->data = data;
    node->freq = freq;
    node->left = node->right = NULL;
    return node;
}

// Tạo MinHeap
struct MinHeap* createMinHeap(unsigned capacity) {
    struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));

    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (struct HuffmanNode**)malloc(capacity * sizeof(struct HuffmanNode*));

    return minHeap;
}

// Swap node
void swapNode(struct HuffmanNode** a, struct HuffmanNode** b) {
    struct HuffmanNode* temp = *a;
    *a = *b;
    *b = temp;
}

// Heapify
void minHeapify(struct MinHeap* minHeap, int idx) {

    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size &&
        minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;

    if (right < minHeap->size &&
        minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;

    if (smallest != idx) {
        swapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

// Kiểm tra size = 1
int isSizeOne(struct MinHeap* minHeap) {
    return (minHeap->size == 1);
}

// Extract min
struct HuffmanNode* extractMin(struct MinHeap* minHeap) {

    struct HuffmanNode* temp = minHeap->array[0];

    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    minHeap->size--;

    minHeapify(minHeap, 0);

    return temp;
}

// Insert node
void insertMinHeap(struct MinHeap* minHeap, struct HuffmanNode* node) {

    minHeap->size++;
    int i = minHeap->size - 1;

    while (i && node->freq < minHeap->array[(i - 1) / 2]->freq) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }

    minHeap->array[i] = node;
}

// Build heap
void buildMinHeap(struct MinHeap* minHeap) {

    int n = minHeap->size - 1;

    for (int i = (n - 1) / 2; i >= 0; i--)
        minHeapify(minHeap, i);
}

// Kiểm tra leaf
int isLeaf(struct HuffmanNode* root) {
    return !(root->left) && !(root->right);
}

// Tạo MinHeap từ data
struct MinHeap* createAndBuildMinHeap(char data[], int freq[], int size) {

    struct MinHeap* minHeap = createMinHeap(size);

    for (int i = 0; i < size; i++)
        minHeap->array[i] = newNode(data[i], freq[i]);

    minHeap->size = size;
    buildMinHeap(minHeap);

    return minHeap;
}

// Xây dựng cây Huffman
struct HuffmanNode* buildHuffmanTree(char data[], int freq[], int size) {

    struct HuffmanNode *left, *right, *top;

    struct MinHeap* minHeap = createAndBuildMinHeap(data, freq, size);

    while (!isSizeOne(minHeap)) {

        left = extractMin(minHeap);
        right = extractMin(minHeap);

        top = newNode('$', left->freq + right->freq);

        top->left = left;
        top->right = right;

        insertMinHeap(minHeap, top);
    }

    return extractMin(minHeap);
}

// In mã Huffman
void printCodes(struct HuffmanNode* root, int arr[], int top, int *compressed_bits) {

    if (root->left) {
        arr[top] = 0;
        printCodes(root->left, arr, top + 1, compressed_bits);
    }

    if (root->right) {
        arr[top] = 1;
        printCodes(root->right, arr, top + 1, compressed_bits);
    }

    if (isLeaf(root)) {

        if (top == 0) { // trường hợp chỉ có 1 ký tự
            arr[top++] = 0;
        }

        printf("%c: ", root->data);

        for (int i = 0; i < top; i++)
            printf("%d", arr[i]);

        printf("\n");

        *compressed_bits += root->freq * top;
    }
}

// Đếm tần số ký tự
void countFrequency(const char *str, int freq[]) {

    for (int i = 0; str[i] != '\0'; i++)
        freq[(unsigned char)str[i]]++;
}

// Free tree
void freeTree(struct HuffmanNode* root) {

    if (!root) return;

    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

int main() {

    char input[256];

    printf("Enter string: ");
    fgets(input, sizeof(input), stdin);

    input[strcspn(input, "\n")] = '\0';

    int len = strlen(input);

    int original_bits = len * 8;

    int freq_table[256] = {0};

    countFrequency(input, freq_table);

    char data[256];
    int freq[256];
    int size = 0;

    for (int i = 0; i < 256; i++) {
        if (freq_table[i] > 0) {
            data[size] = (char)i;
            freq[size] = freq_table[i];
            size++;
        }
    }

    struct HuffmanNode* root = buildHuffmanTree(data, freq, size);

    int arr[MAX_TREE_HT];
    int top = 0;
    int compressed_bits = 0;

    printf("\nHuffman Codes:\n");
    printCodes(root, arr, top, &compressed_bits);

    float ratio = 100.0 * (1 - ((float)compressed_bits / original_bits));

    printf("\nOriginal bits: %d\n", original_bits);
    printf("Compressed bits: %d\n", compressed_bits);
    printf("Compression saved: %.2f%%\n", ratio);

    freeTree(root);

    return 0;
}