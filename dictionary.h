#ifndef DICTIONARY_H
#define DICTIONARY_H

size_t djb33x_hash(const char *key, const size_t keylen);

typedef struct dictNode {
    const char *key;
    size_t keyLen;
    void *value;
    struct dictNode *next;
} dictNode_s;

typedef struct dict {
    dictNode_s **nodes;
    size_t hashmapSize;
} dict_s;

dict_s *DictInit(const size_t hashmap_size);

dictNode_s *DictAdd(dict_s *table, const char *key, size_t keyLen, void *value);

void *DictGet(dict_s *table, const char *key, size_t keyLen);

int DictCount(dict_s *table);

int DictRemove(dict_s *table, const char *key, size_t keyLen);

void DictDealloc(dict_s *table);

#endif