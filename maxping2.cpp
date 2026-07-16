#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <cstring>
#include <climits>

static const int MAXP = 30000 + 5;
static const int MAXNODE = 30000 * 10 + 5;
static const int KEYMAX = 31 * 31 * 31;   // ids 1..30
static const int HSIZE = 128;

// key list for each tag (all 3-combinations containing tag)
static const int MAX_KEYS_PER_TAG = 500;  // actual max C(30-1,2)=406; 500 safe

struct HEntry {
    char key[10];
    unsigned char id;   // 1..30
    unsigned char used;
};

struct Product {
    int base;
    unsigned char tcnt;
    unsigned char tags[5];
    int tagVer[5];      // version snapshot for fast cache validity
    unsigned char alive;
};

struct KeyCache {
    int bestPid;
    int bestPrice;
    unsigned char dirty;
};

static int gN;
static int prodCnt, nodeCnt;
static Product P[MAXP];

// tag state
static int deltaTag[31];
static int verTag[31];

// bucket linked list per key
static int head[KEYMAX];
static int nxt[MAXNODE];
static int pidOfNode[MAXNODE];

// cache per key
static KeyCache KC[KEYMAX];

// tag -> list of keys containing that tag
static int tagKeys[31][MAX_KEYS_PER_TAG];
static int tagKeyCnt[31];
static unsigned char keyMarked[KEYMAX]; // temp for dedup while building

// dictionary
static HEntry H[HSIZE];
static unsigned char nextTagId;

// ---------------- utils ----------------
static inline unsigned int hfunc(const char s[]) {
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
    int idx = (int)(hfunc(s) & (HSIZE - 1));
    while (1) {
        if (!H[idx].used) {
            H[idx].used = 1;
            H[idx].id = ++nextTagId;
            int k = 0; do { H[idx].key[k] = s[k]; } while (s[k++] != '\0');
            return H[idx].id;
        }
        if (streq(H[idx].key, s)) return H[idx].id;
        idx = (idx + 1) & (HSIZE - 1);
    }
}
static inline int getTagIdFind(const char s[]) {
    int idx = (int)(hfunc(s) & (HSIZE - 1));
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
static inline void sortUpTo5(int a[], int n) {
    for (int i = 1; i < n; ++i) {
        int v = a[i], j = i - 1;
        while (j >= 0 && a[j] > v) { a[j + 1] = a[j]; --j; }
        a[j + 1] = v;
    }
}
static inline int key3(int a, int b, int c) {
    return (a * 31 + b) * 31 + c;
}
static inline int curPrice(const Product &pr) {
    int v = pr.base + deltaTag[pr.tags[0]] + deltaTag[pr.tags[1]] + deltaTag[pr.tags[2]];
    if (pr.tcnt >= 4) v += deltaTag[pr.tags[3]];
    if (pr.tcnt == 5) v += deltaTag[pr.tags[4]];
    return v;
}
static inline int versionsMatch(const Product &pr) {
    if (verTag[pr.tags[0]] != pr.tagVer[0]) return 0;
    if (verTag[pr.tags[1]] != pr.tagVer[1]) return 0;
    if (verTag[pr.tags[2]] != pr.tagVer[2]) return 0;
    if (pr.tcnt >= 4 && verTag[pr.tags[3]] != pr.tagVer[3]) return 0;
    if (pr.tcnt == 5 && verTag[pr.tags[4]] != pr.tagVer[4]) return 0;
    return 1;
}
static inline void snapshotVer(Product &pr) {
    pr.tagVer[0] = verTag[pr.tags[0]];
    pr.tagVer[1] = verTag[pr.tags[1]];
    pr.tagVer[2] = verTag[pr.tags[2]];
    if (pr.tcnt >= 4) pr.tagVer[3] = verTag[pr.tags[3]];
    if (pr.tcnt == 5) pr.tagVer[4] = verTag[pr.tags[4]];
}

// --------------- API ---------------
void init(int N)
{
    gN = N;
    prodCnt = nodeCnt = 0;
    nextTagId = 0;

    for (int i = 0; i <= 30; ++i) {
        deltaTag[i] = 0;
        verTag[i] = 0;
        tagKeyCnt[i] = 0;
    }
    for (int i = 0; i < KEYMAX; ++i) {
        head[i] = -1;
        KC[i].bestPid = -1;
        KC[i].bestPrice = INT_MAX;
        KC[i].dirty = 1;
        keyMarked[i] = 0;
    }
    for (int i = 0; i < HSIZE; ++i) H[i].used = 0;

    // precompute keys containing each tag
    for (int t = 1; t <= 30; ++t) {
        for (int b = 1; b <= 30; ++b) if (b != t) {
            for (int c = b + 1; c <= 30; ++c) if (c != t) {
                int a1=t, b1=b, c1=c;
                sort3(a1,b1,c1);
                int k = key3(a1,b1,c1);
                if (!keyMarked[k]) {
                    keyMarked[k] = 1;
                    tagKeys[t][tagKeyCnt[t]++] = k;
                }
            }
        }
        // clear marks
        for (int i = 0; i < tagKeyCnt[t]; ++i) keyMarked[tagKeys[t][i]] = 0;
    }
}

void addProduct(int mPrice, int tagNum, char tagName[][10])
{
    int ids[5];
    for (int i = 0; i < tagNum; ++i) ids[i] = getTagIdCreate(tagName[i]);
    sortUpTo5(ids, tagNum);

    Product &pr = P[prodCnt];
    pr.tcnt = (unsigned char)tagNum;
    pr.alive = 1;

    int dsum = 0;
    for (int i = 0; i < tagNum; ++i) {
        pr.tags[i] = (unsigned char)ids[i];
        dsum += deltaTag[ids[i]];
    }
    pr.base = mPrice - dsum;
    snapshotVer(pr);

    int pid = prodCnt++;

    for (int i = 0; i < tagNum; ++i)
        for (int j = i + 1; j < tagNum; ++j)
            for (int k = j + 1; k < tagNum; ++k) {
                int key = key3(ids[i], ids[j], ids[k]);
                int nd = nodeCnt++;
                pidOfNode[nd] = pid;
                nxt[nd] = head[key];
                head[key] = nd;
                KC[key].dirty = 1; // new candidate may become min
            }
}

int buyProduct(char tag1[], char tag2[], char tag3[])
{
    int a = getTagIdFind(tag1), b = getTagIdFind(tag2), c = getTagIdFind(tag3);
    if (!a || !b || !c) return -1;
    sort3(a,b,c);
    int key = key3(a,b,c);

    KeyCache &cc = KC[key];

    // fast path
    if (!cc.dirty && cc.bestPid >= 0) {
        Product &bp = P[cc.bestPid];
        if (bp.alive && versionsMatch(bp)) {
            int ans = cc.bestPrice;
            bp.alive = 0;
            cc.dirty = 1; // best removed
            return ans;
        }
    }

    // rebuild min by scanning bucket
    int nd = head[key];
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

    if (bestPid < 0) {
        cc.bestPid = -1;
        cc.bestPrice = INT_MAX;
        cc.dirty = 0;
        return -1;
    }

    // cache fresh min
    Product &bp = P[bestPid];
    snapshotVer(bp);
    cc.bestPid = bestPid;
    cc.bestPrice = bestPrice;
    cc.dirty = 0;

    // perform buy
    bp.alive = 0;
    cc.dirty = 1;
    return bestPrice;
}

void adjustPrice(char tag1[], int changePrice)
{
    int t = getTagIdFind(tag1);
    if (!t) return;

    deltaTag[t] += changePrice;
    ++verTag[t];

    // invalidate only keys containing t
    int cnt = tagKeyCnt[t];
    for (int i = 0; i < cnt; ++i) {
        KC[tagKeys[t][i]].dirty = 1;
    }
}