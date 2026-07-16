#include <queue>

#define MAX_PRODUCTS 100000
#define HASH_SIZE 262144 // 2^18, đủ lớn để không bị hash collision nhiều

// Hàm hỗ trợ bare-metal thay cho <cstring>
int my_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}
void my_strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

struct Product {
    int basePrice;
    int mask;
    bool deleted;
};

struct HeapNode {
    int pID;
    int basePrice;
    bool operator<(const HeapNode& other) const {
        return basePrice > other.basePrice; // Min-Heap
    }
};

// Global variables
Product prods[MAX_PRODUCTS];
int pCount;

char tagNames[32][10];
int tagOffset[32];
int tagCnt;

// Custom Static Hash Map để map: Mask -> Priority Queue
struct HashEntry {
    int mask;
    std::priority_queue<HeapNode> pq;
    bool used;
} hashTable[HASH_SIZE];

void init(int numTags) {
    pCount = 0;
    tagCnt = 0;
    for (int i = 0; i < 32; ++i) {
        tagOffset[i] = 0;
    }
    for (int i = 0; i < HASH_SIZE; ++i) {
        if (hashTable[i].used) {
            hashTable[i].pq = std::priority_queue<HeapNode>(); // Clear Queue
            hashTable[i].used = false;
        }
    }
}

// Lấy ID của thẻ, nếu chưa có thì tạo mới
int getTagId(const char* name) {
    for (int i = 0; i < tagCnt; ++i) {
        if (my_strcmp(tagNames[i], name) == 0) return i;
    }
    my_strcpy(tagNames[tagCnt], name);
    return tagCnt++;
}

// Lấy tham chiếu đến Queue của một Mask cụ thể (Linear Probing)
std::priority_queue<HeapNode>& getQueue(int mask) {
    int idx = mask & (HASH_SIZE - 1);
    while (hashTable[idx].used && hashTable[idx].mask != mask) {
        idx = (idx + 1) & (HASH_SIZE - 1);
    }
    if (!hashTable[idx].used) {
        hashTable[idx].used = true;
        hashTable[idx].mask = mask;
    }
    return hashTable[idx].pq;
}

void addProduct(int mPrice, int tagNum, char tagName[][10]) {
    int mask = 0;
    int currentOffsetSum = 0;
    
    for (int i = 0; i < tagNum; ++i) {
        int tId = getTagId(tagName[i]);
        mask |= (1 << tId);
        currentOffsetSum += tagOffset[tId];
    }
    
    int pID = pCount++;
    prods[pID].mask = mask;
    // Khử offset hiện tại để lưu basePrice
    prods[pID].basePrice = mPrice - currentOffsetSum; 
    prods[pID].deleted = false;
    
    getQueue(mask).push({pID, prods[pID].basePrice});
}

void adjustPrice(char* tagName, int changePrice) {
    int tId = getTagId(tagName);
    tagOffset[tId] += changePrice;
}

// Hàm kiểm tra một mask và cập nhật kết quả rẻ nhất
void checkMask(int mask, int& best_pID, int& best_price) {
    int idx = mask & (HASH_SIZE - 1);
    while (hashTable[idx].used) {
        if (hashTable[idx].mask == mask) {
            auto& pq = hashTable[idx].pq;
            
            // Xóa các sản phẩm rác (đã bị mua) ở đỉnh Heap
            while (!pq.empty() && prods[pq.top().pID].deleted) {
                pq.pop();
            }
            
            if (!pq.empty()) {
                int pID = pq.top().pID;
                int baseP = pq.top().basePrice;
                
                // Tính giá thực tế (Actual Price)
                int actualPrice = baseP;
                for (int i = 0; i < tagCnt; ++i) {
                    if ((mask >> i) & 1) actualPrice += tagOffset[i];
                }
                
                if (actualPrice < best_price) {
                    best_price = actualPrice;
                    best_pID = pID;
                }
            }
            break;
        }
        idx = (idx + 1) & (HASH_SIZE - 1);
    }
}

int buyProduct(char* t1, char* t2, char* t3) {
    int id1 = getTagId(t1);
    int id2 = getTagId(t2);
    int id3 = getTagId(t3);
    int q_mask = (1 << id1) | (1 << id2) | (1 << id3);
    
    int best_pID = -1;
    int best_price = 2e9; // Khởi tạo vô cùng lớn
    
    // Tìm các thẻ KHÔNG có trong câu truy vấn
    int rem_mask = ((1 << tagCnt) - 1) ^ q_mask;
    int rem_tags[30];
    int rem_cnt = 0;
    for (int i = 0; i < tagCnt; ++i) {
        if ((rem_mask >> i) & 1) rem_tags[rem_cnt++] = i;
    }
    
    // TH1: Sản phẩm chỉ có đúng 3 thẻ này (0 thẻ dư)
    checkMask(q_mask, best_pID, best_price);
    
    // TH2: Sản phẩm có 4 thẻ (1 thẻ dư)
    for (int i = 0; i < rem_cnt; ++i) {
        checkMask(q_mask | (1 << rem_tags[i]), best_pID, best_price);
    }
    
    // TH3: Sản phẩm có 5 thẻ (2 thẻ dư)
    for (int i = 0; i < rem_cnt; ++i) {
        for (int j = i + 1; j < rem_cnt; ++j) {
            checkMask(q_mask | (1 << rem_tags[i]) | (1 << rem_tags[j]), best_pID, best_price);
        }
    }
    
    if (best_pID != -1) {
        prods[best_pID].deleted = true; // Xóa khỏi thị trường
        return best_price; // Trả về giá của sản phẩm (hoặc có thể trả về best_pID tùy config của main.cpp)
    }
    return -1;
}
