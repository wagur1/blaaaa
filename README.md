# blaaaa

#ifndef _USER_H_
#define _USER_H_

// Cấu trúc Trie để map char* -> int (0..29)
struct TrieNode {
    int id;
    int child[26];
} trie[1005];
int trie_cnt, tag_id_cnt;

int get_tag_id(const char* s) {
    int u = 0;
    for (int i = 0; s[i]; ++i) {
        int c = s[i] - 'a';
        if (!trie[u].child[c]) trie[u].child[c] = ++trie_cnt;
        u = trie[u].child[c];
    }
    if (trie[u].id == -1) trie[u].id = tag_id_cnt++;
    return trie[u].id;
}

int get_tag_id_if_exists(const char* s) {
    int u = 0;
    for (int i = 0; s[i]; ++i) {
        int c = s[i] - 'a';
        if (!trie[u].child[c]) return -1;
        u = trie[u].child[c];
    }
    return trie[u].id;
}

// Cấu trúc Vector tự tạo (Tối ưu Memory)
struct IntVector {
    int* arr;
    int cap, sz;
    void push(int val) {
        if (sz == cap) {
            cap = (cap == 0) ? 4 : cap * 2;
            int* new_arr = new int[cap];
            for (int i = 0; i < sz; ++i) new_arr[i] = arr[i];
            delete[] arr;
            arr = new_arr;
        }
        arr[sz++] = val;
    }
    void clear() {
        if (arr) delete[] arr;
        arr = nullptr;
        cap = sz = 0;
    }
};

// Cấu trúc Heap tự tạo (Tối ưu Access Time)
struct HeapNode {
    long long base_price;
    int mId;
};

struct MinHeap {
    HeapNode* arr;
    int cap, sz;
    void push(long long bp, int id) {
        if (sz == cap) {
            cap = (cap == 0) ? 4 : cap * 2;
            HeapNode* new_arr = new HeapNode[cap];
            for (int i = 0; i < sz; ++i) new_arr[i] = arr[i];
            delete[] arr;
            arr = new_arr;
        }
        arr[sz++] = {bp, id};
        int cur = sz - 1;
        while (cur > 0) {
            int p = (cur - 1) / 2;
            if (arr[cur].base_price < arr[p].base_price || 
               (arr[cur].base_price == arr[p].base_price && arr[cur].mId < arr[p].mId)) {
                HeapNode tmp = arr[cur]; arr[cur] = arr[p]; arr[p] = tmp;
                cur = p;
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
            if (r < sz && (arr[r].base_price < arr[l].base_price || 
                          (arr[r].base_price == arr[l].base_price && arr[r].mId < arr[l].mId))) {
                min_idx = r;
            }
            if (arr[min_idx].base_price < arr[cur].base_price || 
               (arr[min_idx].base_price == arr[cur].base_price && arr[min_idx].mId < arr[cur].mId)) {
                HeapNode tmp = arr[cur]; arr[cur] = arr[min_idx]; arr[min_idx] = tmp;
                cur = min_idx;
            } else break;
        }
    }
    HeapNode top() { return arr[0]; }
    bool empty() { return sz == 0; }
    void clear() {
        if (arr) delete[] arr;
        arr = nullptr;
        cap = sz = 0;
    }
};

// Global Variables
long long tag_offset[35];
bool product_deleted[1000005];

// Map: Mask -> Mask_ID
struct HashNode {
    int mask, id, next;
} hash_table[50005];
int head[65536], hash_cnt;
int mask_of_id[50005];

MinHeap heaps[50005];
IntVector combo_list[27005]; // Lưu các mask_id thuộc cùng 1 tổ hợp 3 tag

int get_mask_id(int mask) {
    int h = mask & 65535;
    for (int i = head[h]; i != -1; i = hash_table[i].next) {
        if (hash_table[i].mask == mask) return hash_table[i].id;
    }
    int id = hash_cnt++;
    hash_table[id].mask = mask;
    hash_table[id].id = id;
    hash_table[id].next = head[h];
    head[h] = id;
    mask_of_id[id] = mask;
    
    // Tạo mọi tổ hợp 3 tags từ mask hiện tại
    int tags[5], t_cnt = 0;
    for (int i = 0; i < 30; ++i) {
        if ((mask >> i) & 1) tags[t_cnt++] = i;
    }
    for (int i = 0; i < t_cnt - 2; ++i) {
        for (int j = i + 1; j < t_cnt - 1; ++j) {
            for (int k = j + 1; k < t_cnt; ++k) {
                int combo = tags[i] * 900 + tags[j] * 30 + tags[k];
                combo_list[combo].push(id);
            }
        }
    }
    return id;
}

// API Functions
void init(int n) {
    for (int i = 0; i <= trie_cnt; ++i) {
        for (int j = 0; j < 26; ++j) trie[i].child[j] = 0;
        trie[i].id = -1;
    }
    trie_cnt = tag_id_cnt = 0;
    
    for (int i = 0; i < 65536; ++i) head[i] = -1;
    hash_cnt = 0;
    
    for (int i = 0; i < 50005; ++i) heaps[i].clear();
    for (int i = 0; i < 27005; ++i) combo_list[i].clear();
    for (int i = 0; i < 35; ++i) tag_offset[i] = 0;
    
    // Note: product_deleted tracking depends on the maximum mId. 
    // Usually mId is bounded, resetting a large array here is safe.
    for (int i = 0; i < 1000005; ++i) product_deleted[i] = false;
}

void addProduct(int mId, int mPrice, int tagNum, char tagName[][10]) {
    int mask = 0;
    for (int i = 0; i < tagNum; ++i) {
        int t = get_tag_id(tagName[i]);
        mask |= (1 << t);
    }
    
    long long current_offset = 0;
    for (int i = 0; i < 30; ++i) {
        if ((mask >> i) & 1) current_offset += tag_offset[i];
    }
    
    long long base_price = mPrice - current_offset;
    int m_id = get_mask_id(mask);
    
    heaps[m_id].push(base_price, mId);
    product_deleted[mId] = false;
}

int buyProduct(char tag1[], char tag2[], char tag3[]) {
    int t[3];
    t[0] = get_tag_id_if_exists(tag1);
    t[1] = get_tag_id_if_exists(tag2);
    t[2] = get_tag_id_if_exists(tag3);
    
    if (t[0] == -1 || t[1] == -1 || t[2] == -1) return -1;
    
    // Sắp xếp các tag ID để tính index đồng nhất
    for (int i = 0; i < 2; ++i) {
        for (int j = i + 1; j < 3; ++j) {
            if (t[i] > t[j]) { int tmp = t[i]; t[i] = t[j]; t[j] = tmp; }
        }
    }
    
    int combo = t[0] * 900 + t[1] * 30 + t[2];
    long long min_price = -1;
    int best_mId = -1;
    
    // Duyệt qua tất cả các Mask có chứa 3 thẻ này
    for (int i = 0; i < combo_list[combo].sz; ++i) {
        int m_id = combo_list[combo].arr[i];
        
        // Loại bỏ rác (các product đã bị mua) trên đỉnh Heap
        while (!heaps[m_id].empty() && product_deleted[heaps[m_id].top().mId]) {
            heaps[m_id].pop();
        }
        
        if (!heaps[m_id].empty()) {
            long long bp = heaps[m_id].top().base_price;
            int p_mId = heaps[m_id].top().mId;
            
            // Tính toán giá thực tế hiện tại
            long long curr_price = bp;
            int mask = mask_of_id[m_id];
            for (int bit = 0; bit < 30; ++bit) {
                if ((mask >> bit) & 1) curr_price += tag_offset[bit];
            }
            
            if (min_price == -1 || curr_price < min_price || 
               (curr_price == min_price && p_mId < best_mId)) {
                min_price = curr_price;
                best_mId = p_mId;
            }
        }
    }
    
    if (best_mId != -1) {
        product_deleted[best_mId] = true;
    }
    
    return best_mId;
}

void adjustPrice(char tag1[], int changePrice) {
    int t1 = get_tag_id_if_exists(tag1);
    if (t1 != -1) {
        tag_offset[t1] += changePrice;
    }
}

#endif
