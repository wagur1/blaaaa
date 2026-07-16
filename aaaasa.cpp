#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <cstring>
#include <climits>

///////////////////////////////////////////////////////////////////////////
// Constraints
// N tags <= 30
// addProduct <= 30000
// each product has 3~5 tags => max C(5,3)=10 triples per product
///////////////////////////////////////////////////////////////////////////

static const int MAXP = 30000 + 5;
static const int MAXN = 30 + 1;          // tag id: 1..30
static const int KEYMAX = 31 * 31 * 31;  // direct key space
static const int MAXNODE = 30000 * 10 + 5;

// custom hash table for tag string -> id
static const int HSIZE = 128; // enough for <=30 tags, keep load very low

struct HEntry {
    char key[10];   // 3~9 chars + '\0'
    unsigned char id;
    unsigned char used; // 0/1
};

struct Product {
    int base;               // base price after subtracting deltas at add time
    unsigned char tcnt;     // 3..5
    unsigned char tags[5];  // sorted ids [1..30]
    unsigned char alive;    // 0/1
};

static int gN;
static int prodCnt;

// tag delta
static int deltaTag[MAXN];

// products
static Product prods[MAXP];

// triple bucket: linked list over node pool
static int head[KEYMAX];
static int nodeNext[MAXNODE];
static int nodePid[MAXNODE];
static int nodeCnt;

// hash table
static HEntry htab[HSIZE];
static unsigned char nextTagId; // 1..N

// ----------------------------------------------------------------------

static inline unsigned int hashTag(const char s[]) {
    // FNV-1a 32-bit
    unsigned int h = 2166136261u;
    for (int i = 0; s[i]; ++i) {
        h ^= (unsigned char)s[i];
        h *= 16777619u;
    }
    return h;
}

static inline int strEq(const char a[], const char b[]) {
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return 0;
        ++i;
    }
    return a[i] == b[i];
}

static inline int getTagId(const char s[]) {
    unsigned int h = hashTag(s);
    int idx = (int)(h & (HSIZE - 1)); // HSIZE power of 2

    while (1) {
        if (!htab[idx].used) {
            // new tag
            htab[idx].used = 1;
            unsigned char nid = ++nextTagId;   // 1-based
            htab[idx].id = nid;
            // copy string
            int k = 0;
            do { htab[idx].key[k] = s[k]; } while (s[k++] != '\0');
            return (int)nid;
        }
        if (strEq(htab[idx].key, s)) return (int)htab[idx].id;
        idx = (idx + 1) & (HSIZE - 1);
    }
}

static inline void sort3(int &a, int &b, int &c) {
    if (a > b) { int t = a; a = b; b = t; }
    if (b > c) { int t = b; b = c; c = t; }
    if (a > b) { int t = a; a = b; b = t; }
}

static inline void sortUpTo5(int arr[], int n) {
    // insertion sort (n<=5)
    for (int i = 1; i < n; ++i) {
        int v = arr[i], j = i - 1;
        while (j >= 0 && arr[j] > v) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = v;
    }
}

static inline int key3(int a, int b, int c) {
    // 1..30 each
    return (a * 31 + b) * 31 + c; // < 31^3
}

static inline int curPrice(const Product &p) {
    int v = p.base;
    // unroll small loop by switch
    switch (p.tcnt) {
    case 5: v += deltaTag[p.tags[4]];
    case 4: v += deltaTag[p.tags[3]];
    case 3: v += deltaTag[p.tags[2]];
            v += deltaTag[p.tags[1]];
            v += deltaTag[p.tags[0]];
            break;
    default:
            for (int i = 0; i < p.tcnt; ++i) v += deltaTag[p.tags[i]];
    }
    return v;
}

// ----------------------------------------------------------------------

void init(int N)
{
    gN = N;
    prodCnt = 0;
    nodeCnt = 0;
    nextTagId = 0;

    // reset arrays
    for (int i = 0; i < MAXN; ++i) deltaTag[i] = 0;
    for (int i = 0; i < KEYMAX; ++i) head[i] = -1;
    for (int i = 0; i < HSIZE; ++i) htab[i].used = 0;
}

void addProduct(int mPrice, int tagNum, char tagName[][10])
{
    int ids[5];
    for (int i = 0; i < tagNum; ++i) ids[i] = getTagId(tagName[i]);
    sortUpTo5(ids, tagNum);

    Product &p = prods[prodCnt];
    p.tcnt = (unsigned char)tagNum;
    p.alive = 1;

    int sdelta = 0;
    for (int i = 0; i < tagNum; ++i) {
        p.tags[i] = (unsigned char)ids[i];
        sdelta += deltaTag[ids[i]];
    }
    p.base = mPrice - sdelta;

    int pid = prodCnt++;
    // register all 3-combinations
    for (int i = 0; i < tagNum; ++i) {
        for (int j = i + 1; j < tagNum; ++j) {
            for (int k = j + 1; k < tagNum; ++k) {
                int key = key3(ids[i], ids[j], ids[k]);
                int nd = nodeCnt++;
                nodePid[nd] = pid;
                nodeNext[nd] = head[key];
                head[key] = nd;
            }
        }
    }
}

int buyProduct(char tag1[], char tag2[], char tag3[])
{
    int a = getTagId(tag1);
    int b = getTagId(tag2);
    int c = getTagId(tag3);
    sort3(a, b, c);

    int key = key3(a, b, c);
    int nd = head[key];
    if (nd == -1) return -1;

    int bestPid = -1;
    int bestPrice = INT_MAX;

    while (nd != -1) {
        int pid = nodePid[nd];
        Product &p = prods[pid];
        if (p.alive) {
            int price = curPrice(p);
            if (price < bestPrice) {
                bestPrice = price;
                bestPid = pid;
            }
        }
        nd = nodeNext[nd];
    }

    if (bestPid == -1) return -1;
    prods[bestPid].alive = 0;
    return bestPrice;
}

void adjustPrice(char tag1[], int changePrice)
{
    int t = getTagId(tag1);
    deltaTag[t] += changePrice;
}