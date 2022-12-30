#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
typedef struct _HashPair HashPair;
typedef struct _HashTableEntry HashTableEntry;
typedef struct _HashTable HashTable;
typedef void *HashTableKey;
typedef void *HashTableValue;
typedef int (*HashFunc)(HashTableKey key);
typedef bool (*HashEqual)(HashTableKey key, );
typedef void (*HashTableFreeKey)(HashTableKey key);
typedef void (*HashTableFreeValue)(HashTableValue value);
struct _HashPair {
    void *key;
    void *value;
};
struct _HashTableEntry {
    HashPair pair;
    HashTableEntry *next;
};
struct _HashTable {
    HashTableEntry **table;
    HashFunc hash_func;
    HashEqual hash_equal;
    HashTableFreeKey free_key;
    HashTableFreeValue free_value;
    int entries;
    int hash_table_size;
};

int hash_table_enlarge(HashTable * hashtable ) {
    HashTableEntry *rover;
    HashTableEntry *next;
    HashPair *pair;
    HashTableEntry **old_table = hashtable->table;
    int old_size = hashtable->hash_table_size;
    int index;

    hashtable->hash_table_size *= 10;
    if ( !hash_table_allocate_table(hashtable) ) {
        hashtable->table = old_table;
        hashtable->hash_table_size = old_size;
        return 0;
    }

    for ( int i = 0 ; i < old_size; i++ ) {
        rover = old_table[i];
        while ( rover ) {
            next = rover->next;
            pair = &(rover->pair);
            index = hashtable->hash_func(pair->key) % hashtable->hash_table_size;
            rover->next = hashtable->table[index];
            hashtable->table[index] = rover;
            rover = next;
        }
    }
}

int hash_table_allocate_table(HashTable * hashtable) {
    hashtable->table = calloc(hashtable->hash_table_size, sizeof(HashTableEntry *));
    return hashtable->table != NULL;
};



HashTable *hash_table_new(HashFunc hash_func, HashEqual hash_equal) {
    HashTable *hashtable;
    hashtable = (HashTable *) calloc(1, sizeof(HashTable));
    hashtable->entries = 0;
    hashtable->hash_table_size = 16;
    hashtable->hash_func = hash_func;
    hashtable->hash_equal = hash_equal;
    if ( !hash_table_allocate_table(hashtable) ) {
        free(hashtable);
        return NULL;
    }
    return hashtable;
}

int hash_table_insert(HashTable *hashtable, HashTableKey key, HashTableValue value) {
    int index;
    HashTableEntry *rover;
    HashTableEntry *newentry;
    HashPair *pair;
    if ( hashtable->entries * 3 >= hashtable->hash_table_size ) {
        if ( !hash_table_enlarge(hashtable) ) {
            return 0;
        }
    }

    index = hashtable->hash_func(key) % hashtable->hash_table_size;
    rover = hashtable->table[index];

    while ( rover ) {
        pair = &(rover->pair);
        if ( hashtable->hash_equal(pair->key, key ) == 0 ) {
            if ( hashtable->free_key != NULL ) {
                hashtable->free_key(pair->key);
            }
            if ( hashtable->free_value != NULL ) {
                hashtable->free_value(pair->value);
            }
            pair->key = key;
            pair->value = value;
            return 1;
        }
        rover = rover->next;
    }

    newentry = (HashTableEntry *)calloc(1, sizeof(HashTableEntry));
    newentry->pair.key = key;
    newentry->pair.value = value;

    newentry->next = hashtable->table[index];
    hashtable->table[index] = newentry;

    hashtable->hash_table_size++;
    return 1;
}

int main()
{

}