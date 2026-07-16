#ifndef _USER_H_
#define _USER_H_

#define MAX_PRODUCTS 1000005
#define MAX_MASKS 180005

// 1. Cấu trúc Trie
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

// 2. Định nghĩa Heap lưu ID sản phẩm
struct HeapNode {
    long long price;
    int mId;
    HeapNode() { price = 0; mId = 0; }
    HeapNode(long long p, int id) { price = p; mId = id; }
};

HeapNode heap_pool[2500005];
int heap_pool_cnt;

struct MinHeap {
    HeapNode* arr;
    int cap, sz;
    
    void init() { arr = 0; cap = sz = 0; }
    
    void push(long long p, int id) {
        if (sz == cap) {
            cap = (cap == 0) ? 4 : cap * 2;
            HeapNode* new_arr = &heap_pool[heap_pool_cnt];
            heap_pool_cnt += cap; 
            for (int i = 0; i < sz; ++i) new_arr[i] = arr[i];
            arr = new_arr;
        }
        arr[sz] = HeapNode(p, id);
        sz++;
        
        int cur = sz - 1;
        while (cur > 0) {
            int parent = (cur - 1) / 2;
            if (arr[cur].price < arr[parent].price || 
               (arr[cur].price == arr[parent].price && arr[cur].mId < arr[parent].mId)) {
                HeapNode tmp = arr[cur]; arr[cur] = arr[parent]; arr[parent] = tmp;
                cur = parent;
            } else break;
        }
    }
    
    void pop() {
        if (sz == 0) return;
        sz--;
        if (sz == 0) return;
        arr[0] = arr[sz];
        int cur = 0;
        while (true) {
            int l = cur * 2 + 1, r = cur * 2 + 2;
            if (l >= sz) break;
            int min_idx = l;
            if (r < sz && (arr[r].price < arr[l].price || 
                          (arr[r].price == arr[l].price && arr[r].mId < arr[l].mId))) {
                min_idx = r;
            }
            if (arr[min_idx].price < arr[cur].price || 
               (arr[min_idx].price == arr[cur].price && arr[min_idx].mId < arr[cur].mId)) {
                HeapNode tmp = arr[cur]; arr[cur] = arr[min_idx]; arr[min_idx] = tmp;
                cur = min_idx;
            } else break;
        }
    }
    
    HeapNode top() { return arr[0]; }
    bool empty() { return sz == 0; }
};

// 3. Quản lý thông tin sản phẩm trực tiếp
long long product_price[MAX_PRODUCTS];
bool product_deleted[MAX_PRODUCTS];
int internal_id_counter;

// Quản lý xem mỗi sản phẩm chứa những Tag nào
int prod_tags[MAX_PRODUCTS][5];
int prod_tag_cnt[MAX_PRODUCTS];

// Danh sách liên kết ngược: Tag -> các Sản phẩm chứa Tag đó
int tag_to_prod_head[45];
int tag_to_prod_next[MAX_PRODUCTS];
int tag_to_prod_val[MAX_PRODUCTS];
int tag_to_prod_cnt;

// Combo List phục vụ hàm buyProduct (Hệ cơ số 40)
int combo_head[65005];
int combo_next[5000005];
int combo_val[5000005];
int combo_cnt;

// Hash Table cho Mask
struct HashNode {
    long long mask;
    int id, next;
} hash_table[MAX_MASKS];
int head[65536], hash_cnt;

MinHeap heaps[MAX_MASKS];

int get_mask_id(long long mask) {
    int h = mask & 65535;
    for (int i = head[h]; i != -1; i = hash_table[i].next) {
        if (hash_table[i].mask == mask) return hash_table[i].id;
    }
    int id = hash_cnt++;
    hash_table[id].mask = mask;
    hash_table[id].id = id;
    hash_table[id].next = head[h];
    head[h] = id;
    
    int tags[45], t_cnt = 0;
    for (int i = 0; i < 40; ++i) {
        if ((mask >> i) & 1LL) tags[t_cnt++] = i;
    }
    
    for (int i = 0; i < t_cnt - 2; ++i) {
        for (int j = i + 1; j < t_cnt - 1; ++j) {
            for (int k = j + 1; k < t_cnt; ++k) {
                int combo = tags[i] * 1600 + tags[j] * 40 + tags[k];
                combo_val[++combo_cnt] = id;
                combo_next[combo_cnt] = combo_head[combo];
                combo_head[combo] = combo_cnt;
            }
        }
    }
    return id;
}

// 4. API Functions
void init(int n) {
    for (int i = 0; i <= internal_id_counter; ++i) {
        product_deleted[i] = false;
        product_price[i] = 0;
    }
    internal_id_counter = 0;
    
    for (int i = 0; i <= trie_cnt; ++i) {
        for (int j = 0; j < 26; ++j) trie[i].child[j] = 0;
        trie[i].id = -1;
    }
    trie_cnt = tag_id_cnt = 0;
    
    for (int i = 0; i < 65536; ++i) head[i] = -1;
    for (int i = 0; i < hash_cnt; ++i) heaps[i].init();
    hash_cnt = 0;
    heap_pool_cnt = 0;
    
    for (int i = 0; i < 65005; ++i) combo_head[i] = 0;
    combo_cnt = 0;
    
    for (int i = 0; i < 45; ++i) tag_to_prod_head[i] = 0;
    tag_to_prod_cnt = 0;
}

void addProduct(int mPrice, int tagNum, char tagName[][10]) {
    long long mask = 0;
    int pId = internal_id_counter++;
    prod_tag_cnt[pId] = tagNum;
    
    for (int i = 0; i < tagNum; ++i) {
        int t = get_tag_id(tagName[i]);
        mask |= (1LL << t);
        prod_tags[pId][i] = t;
        
        // Đưa sản phẩm vào danh sách quản lý của tag t
        tag_to_prod_val[++tag_to_prod_cnt] = pId;
        tag_to_prod_next[tag_to_prod_cnt] = tag_to_prod_head[t];
        tag_to_prod_head[t] = tag_to_prod_cnt;
    }
    
    product_price[pId] = mPrice;
    int m_id = get_mask_id(mask);
    heaps[m_id].push(mPrice, pId);
}

int buyProduct(char tag1[], char tag2[], char tag3[]) {
    int t[3];
    t[0] = get_tag_id_if_exists(tag1);
    t[1] = get_tag_id_if_exists(tag2);
    t[2] = get_tag_id_if_exists(tag3);
    
    if (t[0] == -1 || t[1] == -1 || t[2] == -1) return -1;
    
    if (t[0] > t[1]) { int tmp = t[0]; t[0] = t[1]; t[1] = tmp; }
    if (t[0] > t[2]) { int tmp = t[0]; t[0] = t[2]; t[2] = tmp; }
    if (t[1] > t[2]) { int tmp = t[1]; t[1] = t[2]; t[2] = tmp; }
    
    int combo = t[0] * 1600 + t[1] * 40 + t[2];
    long long min_price = -1;
    int best_pId = -1;
    int best_m_id = -1;
    
    for (int i = combo_head[combo]; i != 0; i = combo_next[i]) {
        int m_id = combo_val[i];
        
        // Đồng bộ Heap: Cập nhật giá thực tế mới nhất lên đỉnh Heap nếu giá bị thay đổi từ adjustPrice
        while (!heaps[m_id].empty()) {
            HeapNode top_node = heaps[m_id].top();
            if (product_deleted[top_node.mId]) {
                heaps[m_id].pop();
            } else if (product_price[top_node.mId] != top_node.price) {
                // Nếu giá thực tế đã bị thay đổi, pop node cũ và đẩy node mới có giá chính xác vào lại
                heaps[m_id].pop();
                heaps[m_id].push(product_price[top_node.mId], top_node.mId);
            } else {
                break;
            }
        }
        
        if (!heaps[m_id].empty()) {
            long long curr_price = heaps[m_id].top().price;
            int p_mId = heaps[m_id].top().mId;
            
            if (min_price == -1 || curr_price < min_price || 
               (curr_price == min_price && p_mId < best_pId)) {
                min_price = curr_price;
                best_pId = p_mId;
                best_m_id = m_id;
            }
        }
    }
    
    if (best_pId != -1) {
        product_deleted[best_pId] = true;
        heaps[best_m_id].pop(); // Xoá luôn khỏi Heap của nó
        return (int)min_price;
    }
    
    return -1;
}

void adjustPrice(char tag1[], int changePrice) {
    int t1 = get_tag_id_if_exists(tag1);
    if (t1 == -1) return;
    
    // Duyệt qua tất cả sản phẩm chứa tag1 để cập nhật giá trực tiếp
    for (int i = tag_to_prod_head[t1]; i != 0; i = tag_to_prod_next[i]) {
        int pId = tag_to_prod_val[i];
        if (!product_deleted[pId]) {
            long long next_price = product_price[pId] + changePrice;
            if (next_price <= 0) next_price = 1; // KHỐNG CHẾ: Giá trị sau thay đổi luôn lớn hơn 0
            product_price[pId] = next_price;
        }
    }
}

#endif
