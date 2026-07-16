#include <queue>

#define MAX_PRODUCTS 100000
#define HASH_SIZE 262144 

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
    
    // Tinh chỉnh 1: Thêm Constructor rõ ràng cho VS 2012
    HeapNode(int id, int price) {
        pID = id;
        basePrice = price;
    }
    
    bool operator<(const HeapNode& other) const {
        return basePrice > other.basePrice; 
    }
};

Product prods[MAX_PRODUCTS];
int pCount;

char tagNames[32][10];
int tagOffset[32];
int tagCnt;

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
    
    // Tinh chỉnh 2: Reset Queue an toàn bằng biến rỗng
    std::priority_queue<HeapNode> emptyPq; 
    for (int i = 0; i < HASH_SIZE; ++i) {
        if (hashTable[i].used) {
            hashTable[i].pq = emptyPq; 
            hashTable[i].used = false;
        }
    }
}

int getTagId(const char* name) {
    for (int i = 0; i < tagCnt; ++i) {
        if (my_strcmp(tagNames[i], name) == 0) return i;
    }
    my_strcpy(tagNames[tagCnt], name);
    return tagCnt++;
}

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
    prods[pID].basePrice = mPrice - currentOffsetSum; 
    prods[pID].deleted = false;
    
    // Tinh chỉnh 3: Gọi Constructor rõ ràng thay vì Initialization List
    getQueue(mask).push(HeapNode(pID, prods[pID].basePrice));
}

void adjustPrice(char* tagName, int changePrice) {
    int tId = getTagId(tagName);
    tagOffset[tId] += changePrice;
}

void checkMask(int mask, int& best_pID, int& best_price) {
    int idx = mask & (HASH_SIZE - 1);
    while (hashTable[idx].used) {
        if (hashTable[idx].mask == mask) {
            std::priority_queue<HeapNode>& pq = hashTable[idx].pq;
            
            while (!pq.empty() && prods[pq.top().pID].deleted) {
                pq.pop();
            }
            
            if (!pq.empty()) {
                int pID = pq.top().pID;
                int baseP = pq.top().basePrice;
                
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
    int best_price = 2e9; 
    
    int rem_mask = ((1 << tagCnt) - 1) ^ q_mask;
    int rem_tags[30];
    int rem_cnt = 0;
    
    for (int i = 0; i < tagCnt; ++i) {
        if ((rem_mask >> i) & 1) rem_tags[rem_cnt++] = i;
    }
    
    checkMask(q_mask, best_pID, best_price);
    
    for (int i = 0; i < rem_cnt; ++i) {
        checkMask(q_mask | (1 << rem_tags[i]), best_pID, best_price);
    }
    
    for (int i = 0; i < rem_cnt; ++i) {
        for (int j = i + 1; j < rem_cnt; ++j) {
            checkMask(q_mask | (1 << rem_tags[i]) | (1 << rem_tags[j]), best_pID, best_price);
        }
    }
    
    if (best_pID != -1) {
        prods[best_pID].deleted = true; 
        return best_price; 
    }
    return -1;
}
