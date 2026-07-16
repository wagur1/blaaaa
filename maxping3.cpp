#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <cstring>
#include <climits>

static const int MAXP = 30000 + 5;
static const int MAXNODE = 30000 * 10 + 5; // <= C(5,3)*30000
static const int KEYMAX = 31 * 31 * 31;    // id in [1..30]
static const int HSIZE = 128;

struct HEntry {
    char key[10];
    unsigned char id;
    unsigned char used;
};

struct Product {
    int base;
    unsigned char tcnt;      // 3..5
    unsigned char tags[5];   // sorted
    int tagVer[5];           // snapshot when price cached/rebuilt
    unsigned char alive;
};

struct KeyCache {
    int bestPid;
    int bestPrice;
};

static Product P[MAXP];
static int prodCnt;

// tag dynamic state
static int deltaTag[31];
static int verTag[31];

// dictionary
static HEntry H[HSIZE];
static unsigned char nextTagId;

// bucket linked-list
static int head[KEYMAX];
static int nxt[MAXNODE];
static int pidOfNode[MAXNODE];
static int nodeCnt;

// cache per key
static KeyCache KC[KEYMAX];

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
static inline void snapshotVer(Product &pr) {
    pr.tagVer[0] = verTag[pr.tags[0]];
    pr.tagVer[1] = verTag[pr.tags[1]];
    pr.tagVer[2] = verTag[pr.tags[2]];
    if (pr.tcnt >= 4) pr.tagVer[3] = verTag[pr.tags[3]];
    if (pr.tcnt == 5) pr.tagVer[4] = verTag[pr.tags[4]];
}
static inline int versionsMatch(const Product &pr) {
    if (verTag[pr.tags[0]] != pr.tagVer[0]) return 0;
    if (verTag[pr.tags[1]] != pr.tagVer[1]) return 0;
    if (verTag[pr.tags[2]] != pr.tagVer[2]) return 0;
    if (pr.tcnt >= 4 && verTag[pr.tags[3]] != pr.tagVer[3]) return 0;
    if (pr.tcnt == 5 && verTag[pr.tags[4]] != pr.tagVer[4]) return 0;
    return 1;
}

// compact one key list when too many dead nodes encountered
static inline void compactKey(int key) {
    int nd = head[key];
    int newHead = -1;
    while (nd != -1) {
        int nx = nxt[nd];
        if (P[pidOfNode[nd]].alive) {
            nxt[nd] = newHead;
            newHead = nd;
        }
        nd = nx;
    }
    head[key] = newHead;
}

// rebuild best for key by full scan
static inline int rebuildBest(int key) {
    int nd = head[key];
    int bestPid = -1;
    int bestPrice = INT_MAX;
    int liveCnt = 0, deadCnt = 0;

    while (nd != -1) {
        int pid = pidOfNode[nd];
        Product &pr = P[pid];
        if (pr.alive) {
            ++liveCnt;
            int price = curPrice(pr);
            if (price < bestPrice) {
                bestPrice = price;
                bestPid = pid;
            }
        } else {
            ++deadCnt;
        }
        nd = nxt[nd];
    }

    // if many dead, compact to speed next queries
    if (deadCnt > 32 && deadCnt > (liveCnt << 1)) {
        compactKey(key);
    }

    KC[key].bestPid = bestPid;
    KC[key].bestPrice = bestPrice;

    if (bestPid >= 0) snapshotVer(P[bestPid]);
    return bestPid;
}

// ---------------- API ----------------
void init(int N)
{
    (void)N; // N only bounds number of possible tags
    prodCnt = 0;
    nodeCnt = 0;
    nextTagId = 0;

    for (int i = 0; i <= 30; ++i) {
        deltaTag[i] = 0;
        verTag[i] = 0;
    }
    for (int i = 0; i < HSIZE; ++i) H[i].used = 0;
    for (int i = 0; i < KEYMAX; ++i) {
        head[i] = -1;
        KC[i].bestPid = -1;
        KC[i].bestPrice = INT_MAX;
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

    // add to all triple buckets + invalidate cache for these keys
    for (int i = 0; i < tagNum; ++i) {
        for (int j = i + 1; j < tagNum; ++j) {
            for (int k = j + 1; k < tagNum; ++k) {
                int key = key3(ids[i], ids[j], ids[k]);
                int nd = nodeCnt++;
                pidOfNode[nd] = pid;
                nxt[nd] = head[key];
                head[key] = nd;
                KC[key].bestPid = -1; // force lazy rebuild
            }
        }
    }
}

int buyProduct(char tag1[], char tag2[], char tag3[])
{
    int a = getTagIdFind(tag1);
    int b = getTagIdFind(tag2);
    int c = getTagIdFind(tag3);
    if (!a || !b || !c) return -1;

    sort3(a, b, c);
    int key = key3(a, b, c);

    int pid = KC[key].bestPid;

    // fast path: cached pid still valid
    if (pid >= 0) {
        Product &bp = P[pid];
        if (bp.alive && versionsMatch(bp)) {
            int ans = KC[key].bestPrice;
            bp.alive = 0;
            KC[key].bestPid = -1; // best removed, lazy rebuild next time
            return ans;
        }
    }

    // rebuild
    pid = rebuildBest(key);
    if (pid < 0) return -1;

    int ans = KC[key].bestPrice;
    P[pid].alive = 0;
    KC[key].bestPid = -1; // force next-time rebuild for this key
    return ans;
}

void adjustPrice(char tag1[], int changePrice)
{
    int t = getTagIdFind(tag1);
    if (!t) return;  // no existing product has this tag
    deltaTag[t] += changePrice;
    ++verTag[t];
}