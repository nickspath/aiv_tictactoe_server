#include "dictionary.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

size_t djb33x_hash(const char *key, const size_t keylen) {
    size_t hash = 5381;
    for (size_t i = 0; i < keylen; i++) {
        hash = ((hash << 5) + hash) ^ key[i];
    }
    return hash;
}

dict_s *DictInit(const size_t hashmapSize) {
    dict_s *table = malloc(sizeof(dict_s));
    if (!table) {
        return NULL;
    }
    table->hashmapSize = hashmapSize;
    table->nodes = calloc(table->hashmapSize, sizeof(dictNode_s *)); //multiple blocks of memory // nodes is **
    if (!table->nodes) {
        free(table);
        return NULL;
    }
    return table;
}

dictNode_s *DictAdd(dict_s *table, const char *key, size_t keyLen, void *value) {
    size_t hash = djb33x_hash(key, keyLen);
    size_t index = hash % table->hashmapSize;

    dictNode_s *head = table->nodes[index];   //get pointer of interested memory block
    while (head) {  // scroll through all the nodes  in the same index
        // If the key already exists, update the value
        if (keyLen == head->keyLen && strncmp(key, head->key, keyLen) == 0) {
            head->value = value;
            return head;
        }
        head = head->next;
    }

    // Key not found, insert a new node
    dictNode_s *new_item = malloc(sizeof(dictNode_s));
    if (!new_item) {
        return NULL;
    }
    new_item->key = key;
    new_item->keyLen = keyLen;
    new_item->value = value;
    new_item->next = table->nodes[index];   // to be connected with the same other nodes in that index
    table->nodes[index] = new_item;
    return new_item;
}

void *DictGet(dict_s *table, const char *key, size_t keyLen) {
    size_t hash = djb33x_hash(key, keyLen);
    size_t index = hash % table->hashmapSize;

    dictNode_s *head = table->nodes[index];
    while (head) {
        if (keyLen == head->keyLen && strncmp(key, head->key, keyLen) == 0) {
            return head->value;
        }
        head = head->next;
    }

    return NULL; // Key not found
}

int DictCount(dict_s *table) {
    int count = 0;
    for (size_t i = 0; i < table->hashmapSize; i++) {
        dictNode_s *node = table->nodes[i];
        while (node) {
            dictNode_s *next_node = node->next;
            count++;
            node = next_node;
        }
    }
    return count;
}

int DictRemove(dict_s *table, const char *key, size_t keyLen) {
    size_t hash = djb33x_hash(key, keyLen);
    size_t index = hash % table->hashmapSize;

    dictNode_s *prev = NULL;
    dictNode_s *current = table->nodes[index];

    while (current) {
        if (keyLen == current->keyLen && strncmp(key, current->key, keyLen) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                table->nodes[index] = current->next;
            }
            free(current);
            return 0; // Successfully removed
        }
        prev = current;
        current = current->next;
    }

    return -1; // Key not found
}

void DictDealloc(dict_s *table) {
    int count = 0;
    for (size_t i = 0; i < table->hashmapSize; i++) {
        dictNode_s *node = table->nodes[i];
        while (node) {
            dictNode_s *next_node = node->next;
            free(node);
            count++;
            node = next_node;
        }
    }
    free(table->nodes);
    free(table);
}