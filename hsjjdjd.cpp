#ifndef _USER_H_
#define _USER_H_

#define MAX_PRODUCTS 1000005
#define MAX_MASKS 180005
#define HEAP_POOL_SIZE 10000005 // Tăng dung lượng để lưu các thay đổi giá

struct TrieNode {
    int id;
    int child[26];
} trie[1005];
int trie_cnt, tag_id_cnt;

int get_tag_id(const char* s) {
    int u = 0;
    for (int i = 0; i < 10 && s[i] != '\0'; ++i) {
        int c = s[i] - 'a';
        if (c < 0 || c > 25) continue;
        if (!trie[u].child[c]) trie[u].child[c] = ++trie_cnt;
        u = trie[u].child[c];
    }
    if (trie[u].id == -1) trie[u].id = tag_id_cnt++;
    return trie[u].id;
}

int get_tag_id_if_exists(const char* s) {
    int u = 0;
    for (int i = 0; i < 10 && s[i] != '\0'; ++i) {
        int c = s[i] - 'a';
        if (c < 0 || c > 25) continue;
        if (!trie[u].child[c]) return -1;
        u = trie[u].child[c];
    }
    return trie[u].id;
}

struct HeapNode {
    long long price;
    int mId;
    HeapNode() { price = 0; mId = 0; }
    HeapNode(long long p, int id) { price = p; mId = id; }
};

HeapNode heap_pool[HEAP_POOL_SIZE];
int heap_pool_cnt;

struct MinHeap {
    HeapNode* arr;
    int cap, sz;
    
    void init() { arr = 0; cap = sz = 0; }
    
    void push(long long p, int id) {
        if (sz == cap) {
            int new_cap = (cap == 0) ? 4 : cap * 2;
            HeapNode* new_arr = &heap_pool[heap_pool_cnt];
            heap_pool_cnt += new_cap; 
            for (int i = 0; i < sz; ++i) new_arr[i] = arr[i];
            arr = new_arr;
            cap = new_cap;
        }
        arr[sz] = HeapNode(p, id);
        sz++;
        int cur = sz - 1;
        while (cur > 0) {
            int parent = (cur - 1) / 2;
            if (arr[cur].price < arr[parent].price || (arr[cur].price == arr[parent].price && arr[cur].mId < arr[parent].mId)) {
                HeapNode tmp = arr[cur]; arr[cur] = arr[parent]; arr[parent] = tmp;
                cur = parent;
            } else break;
        }
    }
    
    void pop() {
        if (sz == 0) return;
        arr[0] = arr[--sz];
        int cur = 0;
        while (true) {
            int l = cur * 2 + 1, r = cur * 2 + 2;
            if (l >= sz) break;
            int min_idx = l;
            if (r < sz && (arr[r].price < arr[l].price || (arr[r].price == arr[l].price && arr[r].mId < arr[l].mId))) min_idx = r;
            if (arr[min_idx].price < arr[cur].price || (arr[min_idx].price == arr[cur].price && arr[min_idx].mId < arr[cur].mId)) {
                HeapNode tmp = arr[cur]; arr[cur] = arr[min_idx]; arr[min_idx] = tmp;
                cur = min_idx;
            } else break;
        }
    }
    HeapNode top() { return arr[0]; }
    bool empty() { return sz == 0; }
};

long long product_price[MAX_PRODUCTS];
bool product_deleted[MAX_PRODUCTS];
int prod_mask_id[MAX_PRODUCTS];
int internal_id_counter;

int tag_to_prod_head[45], tag_to_prod_next[MAX_PRODUCTS], tag_to_prod_val[MAX_PRODUCTS], tag_to_prod_cnt;
int combo_head[65005], combo_next[5000005], combo_val[5000005], combo_cnt;
int head[65536], hash_cnt;

struct HashNode { long long mask; int id, next; } hash_table[MAX_MASKS];
MinHeap heaps[MAX_MASKS];

int get_mask_id(long long mask) {
    int h = mask & 65535;
    for (int i = head[h]; i != -1; i = hash_table[i].next) if (hash_table[i].mask == mask) return hash_table[i].id;
    int id = hash_cnt++;
    hash_table[id].mask = mask; hash_table[id].id = id; hash_table[id].next = head[h]; head[h] = id;
    int tags[45], t_cnt = 0;
    for (int i = 0; i < 40; ++i) if ((mask >> i) & 1LL) tags[t_cnt++] = i;
    for (int i = 0; i < t_cnt - 2; ++i)
        for (int j = i + 1; j < t_cnt - 1; ++j)
            for (int k = j + 1; k < t_cnt; ++k) {
                int combo = tags[i] * 1600 + tags[j] * 40 + tags[k];
                combo_val[++combo_cnt] = id; combo_next[combo_cnt] = combo_head[combo]; combo_head[combo] = combo_cnt;
            }
    return id;
}

void init(int n) {
    internal_id_counter = 0; trie_cnt = tag_id_cnt = 0; hash_cnt = 0; heap_pool_cnt = 0; combo_cnt = 0; tag_to_prod_cnt = 0;
    for (int i = 0; i < 1005; ++i) { trie[i].id = -1; for(int j=0; j<26; ++j) trie[i].child[j]=0; }
    for (int i = 0; i < 65536; ++i) head[i] = -1;
    for (int i = 0; i < 65005; ++i) combo_head[i] = 0;
    for (int i = 0; i < 45; ++i) tag_to_prod_head[i] = 0;
    for (int i = 0; i < MAX_MASKS; ++i) heaps[i].init();
}

void addProduct(int mPrice, int tagNum, char tagName[][10]) {
    long long mask = 0; int pId = internal_id_counter++;
    for (int i = 0; i < tagNum; ++i) {
        int t = get_tag_id(tagName[i]);
        mask |= (1LL << t);
        tag_to_prod_val[++tag_to_prod_cnt] = pId;
        tag_to_prod_next[tag_to_prod_cnt] = tag_to_prod_head[t];
        tag_to_prod_head[t] = tag_to_prod_cnt;
    }
    product_price[pId] = mPrice;
    int m_id = get_mask_id(mask);
    prod_mask_id[pId] = m_id;
    heaps[m_id].push(mPrice, pId);
}

int buyProduct(char tag1[], char tag2[], char tag3[]) {
    int t[3] = {get_tag_id_if_exists(tag1), get_tag_id_if_exists(tag2), get_tag_id_if_exists(tag3)};
    if (t[0] == -1 || t[1] == -1 || t[2] == -1) return -1;
    for(int i=0; i<2; ++i) for(int j=i+1; j<3; ++j) if(t[i]>t[j]) { int tmp=t[i]; t[i]=t[j]; t[j]=tmp; }
    int combo = t[0] * 1600 + t[1] * 40 + t[2];
    long long min_price = -1; int best_pId = -1, best_m_id = -1;
    for (int i = combo_head[combo]; i != 0; i = combo_next[i]) {
        int m_id = combo_val[i];
        while (!heaps[m_id].empty() && (product_deleted[heaps[m_id].top().mId] || product_price[heaps[m_id].top().mId] != heaps[m_id].top().price))
            heaps[m_id].pop();
        if (!heaps[m_id].empty()) {
            HeapNode top = heaps[m_id].top();
            if (min_price == -1 || top.price < min_price || (top.price == min_price && top.mId < best_pId)) {
                min_price = top.price; best_pId = top.mId; best_m_id = m_id;
            }
        }
    }
    if (best_pId != -1) { product_deleted[best_pId] = true; heaps[best_m_id].pop(); return (int)min_price; }
    return -1;
}

void adjustPrice(char tag1[], int changePrice) {
    int t1 = get_tag_id_if_exists(tag1);
    if (t1 == -1) return;
    for (int i = tag_to_prod_head[t1]; i != 0; i = tag_to_prod_next[i]) {
        int pId = tag_to_prod_val[i];
        if (!product_deleted[pId]) {
            product_price[pId] = (product_price[pId] + changePrice <= 0) ? 1 : product_price[pId] + changePrice;
            heaps[prod_mask_id[pId]].push(product_price[pId], pId);
        }
    }
}
#endif
