#ifndef _USER_H_
#define _USER_H_

struct TrieNode {
    int id;
    int child[26];
} trie[1005];
int trie_cnt, tag_id_cnt;

// XỬ LÝ RÁC: Khóa cứng tối đa 10 ký tự, dừng ngay lập tức khi gặp '\0'
int get_tag_id(const char* s) {
    int u = 0;
    for (int i = 0; i < 10 && s[i] != '\0'; ++i) {
        int c = s[i] - 'a';
        if (c < 0 || c > 25) continue; // Bỏ qua ký tự không hợp lệ nếu có
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

// Bảng băm cho Combo (Mở rộng kích thước để chứa Base-40)
int combo_head[65005];
int combo_next[5000005]; 
int combo_val[5000005];
int combo_cnt;

struct HeapNode {
    long long base_price;
    int mId;
};
HeapNode heap_pool[2500005];
int heap_pool_cnt;

struct MinHeap {
    HeapNode* arr;
    int cap, sz;
    
    void init() { arr = 0; cap = sz = 0; }
    
    void push(long long bp, int id) {
        if (sz == cap) {
            cap = (cap == 0) ? 4 : cap * 2;
            HeapNode* new_arr = &heap_pool[heap_pool_cnt];
            heap_pool_cnt += cap; 
            
            for (int i = 0; i < sz; ++i) {
                new_arr[i].base_price = arr[i].base_price;
                new_arr[i].mId = arr[i].mId;
            }
            arr = new_arr;
        }
        arr[sz].base_price = bp;
        arr[sz].mId = id;
        sz++;
        
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
        sz--;
        if (sz == 0) return;
        arr[0] = arr[sz];
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
};

long long tag_offset[45]; // Mở rộng đề phòng bẫy số lượng
bool product_deleted[1000005];
int internal_id_counter;

struct HashNode {
    long long mask; // TRÁNH TRÀN BIT: Dùng long long
    int id, next;
} hash_table[180005];
int head[65536], hash_cnt;

int mask_tags[180005][10]; // Mở rộng chiều thứ 2 lên 10
int mask_tag_cnt[180005];
MinHeap heaps[180005];

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
    for (int i = 0; i < 40; ++i) { // Quét rộng ra 40 bit
        if ((mask >> i) & 1LL) tags[t_cnt++] = i;
    }
    
    mask_tag_cnt[id] = t_cnt;
    for (int i = 0; i < t_cnt; ++i) mask_tags[id][i] = tags[i];
    
    for (int i = 0; i < t_cnt - 2; ++i) {
        for (int j = i + 1; j < t_cnt - 1; ++j) {
            for (int k = j + 1; k < t_cnt; ++k) {
                // HỆ CƠ SỐ 40: Tránh đụng độ tuyệt đối
                int combo = tags[i] * 1600 + tags[j] * 40 + tags[k];
                combo_val[++combo_cnt] = id;
                combo_next[combo_cnt] = combo_head[combo];
                combo_head[combo] = combo_cnt;
            }
        }
    }
    return id;
}

void init(int n) {
    for (int i = 0; i <= internal_id_counter; ++i) product_deleted[i] = false;
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
    for (int i = 0; i < 45; ++i) tag_offset[i] = 0;
}

void addProduct(int mPrice, int tagNum, char tagName[][10]) {
    long long mask = 0; // Tránh tràn số dương
    for (int i = 0; i < tagNum; ++i) {
        int t = get_tag_id(tagName[i]);
        mask |= (1LL << t); // Dịch bit hệ 64-bit
    }
    
    long long current_offset = 0;
    for (int i = 0; i < 40; ++i) {
        if ((mask >> i) & 1LL) current_offset += tag_offset[i];
    }
    
    long long base_price = mPrice - current_offset;
    int m_id = get_mask_id(mask);
    
    int pId = internal_id_counter++; 
    heaps[m_id].push(base_price, pId);
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
    
    // Đồng bộ với Hệ cơ số 40
    int combo = t[0] * 1600 + t[1] * 40 + t[2];
    long long min_price = -1;
    int best_mId = -1;
    
    for (int i = combo_head[combo]; i != 0; i = combo_next[i]) {
        int m_id = combo_val[i];
        
        while (!heaps[m_id].empty() && product_deleted[heaps[m_id].top().mId]) {
            heaps[m_id].pop();
        }
        
        if (!heaps[m_id].empty()) {
            long long bp = heaps[m_id].top().base_price;
            int p_mId = heaps[m_id].top().mId;
            long long curr_price = bp;
            
            for (int k = 0; k < mask_tag_cnt[m_id]; ++k) {
                curr_price += tag_offset[mask_tags[m_id][k]];
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
        return (int)min_price;
    }
    
    return -1;
}

void adjustPrice(char tag1[], int changePrice) {
    int t1 = get_tag_id_if_exists(tag1);
    if (t1 != -1) {
        tag_offset[t1] += changePrice;
    }
}

#endif
