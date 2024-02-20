#ifndef __TRIE_H__
#define __TRIE_H__

typedef struct trie trie_t;

#include <stdio.h>

/*
typedef enum children
{
    LEFT,
    RIGHT,
    NUM_OF_CHILDREN
}children_t;

typedef struct trie_node
{
    struct trie_node *children[NUM_OF_CHILDREN];
    struct trie_node *parent;
    int is_full;
}trie_node_t;

typedef struct trie
{
    size_t height;
    trie_node_t *root;
}trie_t;
*/

trie_t *TrieCreate(size_t height);

void TrieDestroy(trie_t *trie);

int TrieCreatePath(trie_t *trie, unsigned char *requested, unsigned char *result);

int TrieRemove(trie_t *trie, unsigned char *requested);

size_t TrieCount(trie_t *trie);

#endif /*__TRIE_H__*/
