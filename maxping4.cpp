#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <climits>
#include <vector>
#include <queue>
#include <functional>

// ------------------------- hằng số -------------------------
static const int MAXTAG      = 30;
static const int HSIZE       = 128;          // bảng băm tên tag -> id
static const int KEYMAX      = 31 * 31 * 31; // không gian key của bộ 3 (id 1..30)
static const int GMASK_HSIZE = 1 << 16;      // bảng băm mask -> group
static const int MAXGROUP    = 30000 + 5;    // <= số sản phẩm

// ------------------- từ điển tên tag -> id -------------------
struct HEntry { char key[10]; unsigned char id; unsigned char used; };
static HEntry H[HSIZE];
static int nextTagId;

// ------------------- delta động theo tag -------------------
static int deltaTag[MAXTAG + 1];

// ------------------- nhóm (mỗi mask = 1 nhóm) -------------------
static int           groupCnt;
static int           gTcnt[MAXGROUP];
static unsigned char gTags[MAXGROUP][5];   // các id tag, đã sắp xếp
// min-heap trên base của các sản phẩm còn sống trong nhóm
static std::priority_queue<int, std::vector<int>, std::greater<int> > gHeap[MAXGROUP];

// ------------------- băm mask -> group (open addressing) -------------------
static int maskKeyOf[GMASK_HSIZE]; // mask đã lưu (0 = trống, mask hợp lệ luôn != 0)
static int maskGrpOf[GMASK_HSIZE];

// ------------------- bucket: key bộ-3 -> danh sách group là superset -------------------
static std::vector<int> bucket[KEYMAX];

// ------------------------- tiện ích -------------------------
static inline unsigned int hfunc(const char s[]) {
    unsigned int h = 5381u;
    for (int i = 0; s[i]; ++i) h = ((h << 5) + h) ^ (unsigned char)s[i];
    return h;
}
static inline int streq(const char a[], const char b[]) {
    int i = 0;
    while (a[i] && b[i]) { if (a[i] != b[i]) return 0; ++i; }
    return a[i] == b[i];
}
static inline int getTagIdCreate(const char s[]) {
    int idx = (int)(hfunc(s) & (HSIZE - 1));
    while (1) {
        if (!H[idx].used) {
            H[idx].used = 1;
            H[idx].id = (unsigned char)(++nextTagId);
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
static inline int key3(int a, int b, int c) { return (a * 31 + b) * 31 + c; }

// tìm hoặc tạo nhóm cho một bộ tag (đã sắp xếp, phân biệt)
static inline int findOrCreateGroup(const int ids[], int n) {
    int mask = 0;
    for (int i = 0; i < n; ++i) mask |= (1 << (ids[i] - 1)); // mask != 0 vì n>=3

    unsigned int h = (unsigned int)mask * 2654435761u;
    int idx = (int)(h & (GMASK_HSIZE - 1));
    while (maskKeyOf[idx] != 0) {
        if (maskKeyOf[idx] == mask) return maskGrpOf[idx];
        idx = (idx + 1) & (GMASK_HSIZE - 1);
    }

    // tạo nhóm mới
    int g = groupCnt++;
    gTcnt[g] = n;
    for (int i = 0; i < n; ++i) gTags[g][i] = (unsigned char)ids[i];
    maskKeyOf[idx] = mask;
    maskGrpOf[idx] = g;

    // đăng ký nhóm vào mọi bucket bộ-3 con của mask (chỉ một lần cho mỗi mask)
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            for (int k = j + 1; k < n; ++k)
                bucket[key3(ids[i], ids[j], ids[k])].push_back(g);

    return g;
}

// ------------------------- API -------------------------
void init(int N) {
    (void)N;
    // dọn heap của các nhóm đã dùng ở test trước
    for (int i = 0; i < groupCnt; ++i) {
        std::priority_queue<int, std::vector<int>, std::greater<int> > empty;
        gHeap[i].swap(empty);
    }
    groupCnt = 0;
    nextTagId = 0;

    for (int i = 0; i <= MAXTAG; ++i) deltaTag[i] = 0;
    for (int i = 0; i < HSIZE; ++i) H[i].used = 0;
    for (int i = 0; i < GMASK_HSIZE; ++i) maskKeyOf[i] = 0;
    for (int i = 0; i < KEYMAX; ++i) bucket[i].clear();
}

void addProduct(int mPrice, int tagNum, char tagName[][10]) {
    int ids[5];
    for (int i = 0; i < tagNum; ++i) ids[i] = getTagIdCreate(tagName[i]);
    sortUpTo5(ids, tagNum);

    int dsum = 0;
    for (int i = 0; i < tagNum; ++i) dsum += deltaTag[ids[i]];
    int base = mPrice - dsum; // hằng số; giá = base + tổng delta hiện tại

    int g = findOrCreateGroup(ids, tagNum);
    gHeap[g].push(base);
}

int buyProduct(char tag1[], char tag2[], char tag3[]) {
    int a = getTagIdFind(tag1);
    int b = getTagIdFind(tag2);
    int c = getTagIdFind(tag3);
    if (!a || !b || !c) return -1;

    sort3(a, b, c);
    std::vector<int>& gs = bucket[key3(a, b, c)];

    int best = INT_MAX, bestG = -1;
    for (size_t i = 0; i < gs.size(); ++i) {
        int g = gs[i];
        if (gHeap[g].empty()) continue;
        int d = 0, n = gTcnt[g];
        for (int t = 0; t < n; ++t) d += deltaTag[gTags[g][t]];
        int price = gHeap[g].top() + d; // top = base nhỏ nhất trong nhóm
        if (price < best) { best = price; bestG = g; }
    }
    if (bestG < 0) return -1;

    gHeap[bestG].pop(); // mua = bỏ sản phẩm rẻ nhất khớp
    return best;
}

void adjustPrice(char tag1[], int changePrice) {
    int t = getTagIdFind(tag1);
    if (!t) return;            // chưa sản phẩm nào mang tag này
    deltaTag[t] += changePrice;
}