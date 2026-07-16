#ifndef _USER_H_
#define _USER_H_

// 1. Cấu trúc Trie
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

// 2. Danh sách liên kết tĩnh (Thay thế Vector) - Tốc độ O(1), 0 tốn cấp phát
int combo_head[27005];
int combo_next[500005];
int combo_val[500005];
int combo_cnt;

// 3. Memory Pool cho toàn bộ Heap - Không dùng lệnh "new"
struct HeapNode {
    long long base_price;
    int mId;
};
HeapNode heap_pool[2000005]; // Cấp sẵn 2 triệu ô nhớ
int heap_pool_cnt;

struct MinHeap {
    HeapNode* arr;
    int cap, sz;
    
    void init() { arr = 0; cap = sz = 0; }
    
    void push(long long bp, int id) {
        if (sz == cap) {
            cap = (cap == 0) ? 4 : cap * 2;
            HeapNode* new_arr = &heap_pool[heap_pool_cnt];
            heap_pool_cnt += cap; // Chỉ trỏ con trỏ lên, không tốn time cấp phát
            
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

// 4. Bảng Băm & Lưu trữ trạng thái hệ thống
long long tag_offset[35];
bool product_deleted[1000005];
int internal_id_counter;

struct HashNode {
    int mask, id, next;
} hash_table[50005];
int head[65536], hash_cnt;

// TỐI ƯU CỰC HẠN: Lưu sẵn các tag của mỗi Mask thay vì duyệt 30 bit
int mask_tags[50005][5];
int mask_tag_cnt[50005];
MinHeap heaps[50005];

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
    
    int tags[35], t_cnt = 0;
    for (int i = 0; i < 30; ++i) {
        if ((mask >> i) & 1) tags[t_cnt++] = i;
    }
    
    // Lưu cấu hình tag để tính giá nhanh O(1) sau này
    mask_tag_cnt[id] = t_cnt;
    for (int i = 0; i < t_cnt; ++i) mask_tags[id][i] = tags[i];
    
    // Khai triển tổ hợp 3
    for (int i = 0; i < t_cnt - 2; ++i) {
        for (int j = i + 1; j < t_cnt - 1; ++j) {
            for (int k = j + 1; k < t_cnt; ++k) {
                int combo = tags[i] * 900 + tags[j] * 30 + tags[k];
                // Add vào Linked List (O(1))
                combo_val[++combo_cnt] = id;
                combo_next[combo_cnt] = combo_head[combo];
                combo_head[combo] = combo_cnt;
            }
        }
    }
    return id;
}

// 5. API Functions
void init(int n) {
    // TỐI ƯU: Chỉ reset phần ID đã dùng ở testcase trước, không lặp 1 triệu lần
    for (int i = 0; i <= internal_id_counter; ++i) product_deleted[i] = false;
    internal_id_counter = 0;
    
    for (int i = 0; i <= trie_cnt; ++i) {
        for (int j = 0; j < 26; ++j) trie[i].child[j] = 0;
        trie[i].id = -1;
    }
    trie_cnt = tag_id_cnt = 0;
    
    for (int i = 0; i < 65536; ++i) head[i] = -1;
    hash_cnt = 0;
    
    // TỐI ƯU: Reset Memory Pool cực nhanh
    heap_pool_cnt = 0; 
    for (int i = 0; i < 50005; ++i) heaps[i].init();
    
    for (int i = 0; i < 27005; ++i) combo_head[i] = 0;
    combo_cnt = 0;
    
    for (int i = 0; i < 35; ++i) tag_offset[i] = 0;
}

void addProduct(int mPrice, int tagNum, char tagName[][10]) {
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
    
    int combo = t[0] * 900 + t[1] * 30 + t[2];
    long long min_price = -1;
    int best_mId = -1;
    
    // Duyệt danh sách liên kết siêu tốc
    for (int i = combo_head[combo]; i != 0; i = combo_next[i]) {
        int m_id = combo_val[i];
        
        while (!heaps[m_id].empty() && product_deleted[heaps[m_id].top().mId]) {
            heaps[m_id].pop();
        }
        
        if (!heaps[m_id].empty()) {
            long long bp = heaps[m_id].top().base_price;
            int p_mId = heaps[m_id].top().mId;
            long long curr_price = bp;
            
            // TỐI ƯU: Chỉ lặp 3-5 lần lấy đúng tag đang có thay vì 30 lần quét Bit
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
