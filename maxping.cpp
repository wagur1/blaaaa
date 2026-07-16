#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <cstring>
#include <climits>

static const int MAXP = 30000 + 5;
static const int MAXNODE = 30000 * 10 + 5;   // each product contributes <=10 triples
static const int KEYMAX = 31 * 31 * 31;      // tag id 1..30
static const int HSIZE = 128;                // open addressing for <=30 tags

struct HEntry {
    char key[10];          // 3~9 + '\0'
    unsigned char id;      // 1..30
    unsigned char used;    // 0/1
};

struct Product {
    int base;              // mPrice - sum(delta of tags) at add time
    unsigned char tcnt;    // 3..5
    unsigned char tags[5]; // tag ids (sorted)
    unsigned char alive;   // 0/1
};

static int gN;
static int prodCnt;
static Product P[MAXP];

// delta per tag id [1..30]
static int deltaTag[31];

// triple bucket -> linked list
static int head[KEYMAX];
static int nxt[MAXNODE];
static int pidOfNode[MAXNODE];
static int nodeCnt;

// tag dictionary
static HEntry H[HSIZE];
static unsigned char nextTagId; // 0..30

// ------------------------------------------------------------

static inline unsigned int hfunc(const char s[]) {
    // djb2-xor, nhẹ và đủ ổn cho key ngắn
    unsigned int h = 5381u;
    int i = 0;
    while (s[i]) {
        h = ((h << 5) + h) ^ (unsigned char)s[i];
        ++i;
    }
    return h;
}

static inline int streq(const char a[], const char b[]) {
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return 0;
        ++i;
    }
    return a[i] == b[i];
}

// create if absent
static inline int getTagIdCreate(const char s[]) {
    unsigned int h = hfunc(s);
    int idx = (int)(h & (HSIZE - 1)); // HSIZE=2^k

    while (1) {
        if (!H[idx].used) {
            H[idx].used = 1;
            H[idx].id = ++nextTagId;

            int k = 0;
            do { H[idx].key[k] = s[k]; } while (s[k++] != '\0');
            return H[idx].id;
        }
        if (streq(H[idx].key, s)) return H[idx].id;
        idx = (idx + 1) & (HSIZE - 1);
    }
}

// find only, return 0 if absent
static inline int getTagIdFind(const char s[]) {
    unsigned int h = hfunc(s);
    int idx = (int)(h & (HSIZE - 1));

    while (1) {
        if (!H[idx].used) return 0;
        if (streq(H[idx].key, s)) return H[idx].id;
        idx = (idx + 1) & (HSIZE - 1);
    }
}

static inline void sort3(int &a, int &b, int &c) {
    if (a > b) { int t = a; a = b; b = t; }
    if (b > c) { int t = b; b = c; c = t; }
    if (a > b) { int t = a; a = b; b = t; }
}

static inline void sortUpTo5(int arr[], int n) {
    // insertion sort n<=5
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
    // a,b,c in [1..30]
    return (a * 31 + b) * 31 + c;
}

static inline int curPrice(const Product &pr) {
    int v = pr.base;
    // tcnt is 3~5, unroll nhẹ
    v += deltaTag[pr.tags[0]];
    v += deltaTag[pr.tags[1]];
    v += deltaTag[pr.tags[2]];
    if (pr.tcnt >= 4) v += deltaTag[pr.tags[3]];
    if (pr.tcnt == 5) v += deltaTag[pr.tags[4]];
    return v;
}

// ------------------------------------------------------------

void init(int N)
{
    gN = N;
    prodCnt = 0;
    nodeCnt = 0;
    nextTagId = 0;

    for (int i = 0; i <= 30; ++i) deltaTag[i] = 0;
    for (int i = 0; i < KEYMAX; ++i) head[i] = -1;
    for (int i = 0; i < HSIZE; ++i) H[i].used = 0;
}

void addProduct(int mPrice, int tagNum, char tagName[][10])
{
    int ids[5];
    for (int i = 0; i < tagNum; ++i) ids[i] = getTagIdCreate(tagName[i]);
    sortUpTo5(ids, tagNum);

    Product &pr = P[prodCnt];
    pr.tcnt = (unsigned char)tagNum;
    pr.alive = 1;

    int sdelta = 0;
    for (int i = 0; i < tagNum; ++i) {
        pr.tags[i] = (unsigned char)ids[i];
        sdelta += deltaTag[ids[i]];
    }
    pr.base = mPrice - sdelta;

    int pid = prodCnt++;
    // register all 3-combinations
    for (int i = 0; i < tagNum; ++i) {
        for (int j = i + 1; j < tagNum; ++j) {
            for (int k = j + 1; k < tagNum; ++k) {
                int key = key3(ids[i], ids[j], ids[k]);
                int nd = nodeCnt++;
                pidOfNode[nd] = pid;
                nxt[nd] = head[key];
                head[key] = nd;
            }
        }
    }
}

int buyProduct(char tag1[], char tag2[], char tag3[])
{
    int a = getTagIdFind(tag1);
    int b = getTagIdFind(tag2);
    int c = getTagIdFind(tag3);

    if (a == 0 || b == 0 || c == 0) return -1;

    sort3(a, b, c);
    int key = key3(a, b, c);

    int nd = head[key];
    if (nd == -1) return -1;

    int bestPid = -1;
    int bestPrice = INT_MAX;

    while (nd != -1) {
        int pid = pidOfNode[nd];
        Product &pr = P[pid];

        if (pr.alive) {
            int price = curPrice(pr);
            if (price < bestPrice) {
                bestPrice = price;
                bestPid = pid;
            }
        }
        nd = nxt[nd];
    }

    if (bestPid == -1) return -1;
    P[bestPid].alive = 0;
    return bestPrice;
}

void adjustPrice(char tag1[], int changePrice)
{
    int t = getTagIdFind(tag1);
    if (t) deltaTag[t] += changePrice;
}