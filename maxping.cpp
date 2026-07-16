#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <cstring>
#include <climits>

static const int MAXP = 30000 + 5;
static const int MAXNODE = 30000 * 10 + 5;   // each product => up to C(5,3)=10
static const int KEYMAX = 31 * 31 * 31;      // tag id 1..30
static const int HSIZE = 128;                // for <=30 tags, open addressing

struct HEntry {
    char key[10];
    unsigned char id;   // 1..30
    unsigned char used;
};

struct Product {
    int base;           // mPrice - sum(delta[tag]) at add time
    unsigned int mask;  // bit i means tag i exists (id 1..30 -> bit (id-1))
    unsigned char alive;
};

static int N;
static int prodCnt;
static Product P[MAXP];

// delta per tag id [1..30]
static int deltaTag[31];

// bucket linked-list for each 3-tag key
static int head[KEYMAX];
static int nxt[MAXNODE];
static int pidOfNode[MAXNODE];
static int nodeCnt;

// tag dictionary
static HEntry H[HSIZE];
static unsigned char nextTagId; // 0..30

// -------- utils --------
static inline unsigned int hfunc(const char s[]) {
    // cheap djb2 variant
    unsigned int h = 5381u;
    for (int i = 0; s[i]; ++i) h = ((h << 5) + h) ^ (unsigned char)s[i];
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
static inline int getTagIdCreate(const char s[]) {
    unsigned int h = hfunc(s);
    int idx = (int)(h & (HSIZE - 1));
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
static inline int getTagIdFind(const char s[]) {
    unsigned int h = hfunc(s);
    int idx = (int)(h & (HSIZE - 1));
    while (1) {
        if (!H[idx].used) return 0; // not found
        if (streq(H[idx].key, s)) return H[idx].id;
        idx = (idx + 1) & (HSIZE - 1);
    }
}
static inline void sort3(int &a, int &b, int &c) {
    if (a > b) { int t = a; a = b; b = t; }
    if (b > c) { int t = b; b = c; c = t; }
    if (a > b) { int t = a; a = b; b = t; }
}
static inline void sort5(int a[], int n) {
    for (int i = 1; i < n; ++i) {
        int v = a[i], j = i - 1;
        while (j >= 0 && a[j] > v) { a[j + 1] = a[j]; --j; }
        a[j + 1] = v;
    }
}
static inline int key3(int a, int b, int c) {
    return (a * 31 + b) * 31 + c;
}

// Sum deltas for tags in bitmask (<=5 bits set, very fast with ctz loop)
static inline int sumDeltaMask(unsigned int m) {
    int s = 0;
    while (m) {
        unsigned int lsb = m & -m;
#if defined(_MSC_VER)
        unsigned long idx;
        _BitScanForward(&idx, lsb);
        s += deltaTag[(int)idx + 1];
#else
        int idx = __builtin_ctz(lsb);
        s += deltaTag[idx + 1];
#endif
        m ^= lsb;
    }
    return s;
}

// -------- API --------
void init(int n)
{
    N = n;
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
    sort5(ids, tagNum);

    unsigned int mask = 0u;
    int dsum = 0;
    for (int i = 0; i < tagNum; ++i) {
        mask |= (1u << (ids[i] - 1));
        dsum += deltaTag[ids[i]];
    }

    int pid = prodCnt++;
    P[pid].base = mPrice - dsum;
    P[pid].mask = mask;
    P[pid].alive = 1;

    // Register all C(tagNum,3)
    for (int i = 0; i < tagNum; ++i)
        for (int j = i + 1; j < tagNum; ++j)
            for (int k = j + 1; k < tagNum; ++k) {
                int key = key3(ids[i], ids[j], ids[k]);
                int nd = nodeCnt++;
                pidOfNode[nd] = pid;
                nxt[nd] = head[key];
                head[key] = nd;
            }
}

int buyProduct(char tag1[], char tag2[], char tag3[])
{
    int a = getTagIdFind(tag1);
    int b = getTagIdFind(tag2);
    int c = getTagIdFind(tag3);
    if (a == 0 || b == 0 || c == 0) return -1; // unknown tag => impossible

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
            int price = pr.base + sumDeltaMask(pr.mask);
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
    if (t) deltaTag[t] += changePrice; // unknown tag => no affected product
}