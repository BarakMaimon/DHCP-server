/*
dev: barak
rev:
date:
status:
*/
#include <stdlib.h> /*malloc*/
#include <assert.h> /*assert*/

#include "trie.h"

typedef enum children
{
	LEFT,
	RIGHT,
	NUM_OF_CHILDREN
} children_t;

enum trie_status
{
	TRIE_SUCCESS,
	TRIE_FAILURE,
	TRIE_OCUPIED
};

typedef struct trie_node
{
	struct trie_node *children[NUM_OF_CHILDREN];
	struct trie_node *parent;
	int is_used;
} trie_node_t;

struct trie
{
	size_t height;
	trie_node_t root;
};

static trie_node_t *CreateNode(trie_node_t *father);
static void FindCloseRequested(unsigned char *str, size_t height);
static void PostOrderDestroy(trie_node_t *node);
static int RecCreatePath(trie_node_t *node, size_t height, unsigned char *requested, unsigned char *result);
static void RecCountLeaf(trie_node_t *node, size_t height, size_t *count);
static int IsUsed(trie_node_t *node);

trie_t *TrieCreate(size_t height)
{
	trie_t *trie = NULL;
	int i = 0;

	assert(0 < height);

	trie = malloc(sizeof(trie_t));
	if (NULL == trie)
	{
		return NULL;
	}

	trie->height = height;
	trie->root.is_used = 0;
	trie->root.parent = NULL;
	for (i = 0; i < NUM_OF_CHILDREN; ++i)
	{
		trie->root.children[i] = NULL;
	}

	return trie;
}

void TrieDestroy(trie_t *trie)
{
	PostOrderDestroy(trie->root.children[LEFT]);
	PostOrderDestroy(trie->root.children[RIGHT]);

	free(trie);
}

int TrieCreatePath(trie_t *trie, unsigned char *requested, unsigned char *result)
{
	assert(NULL != trie);
	assert(NULL != requested);
	assert(NULL != result);

	return RecCreatePath(&trie->root, trie->height, requested, result);
}

int TrieRemove(trie_t *trie, unsigned char *requested)
{
	size_t i = 0;
	trie_node_t *node = NULL;

	node = &trie->root;

	for (i = 1; i < trie->height - 1; ++i)
	{
		assert(NULL != node->children[(int)*requested]);

		node = node->children[(int)*requested];
		node->is_used = 0;
		++requested;
	}

	node = node->children[(int)*requested];
	if (node->is_used == 1)
	{
		node->is_used = 0;
		return TRIE_SUCCESS;
	}

	return TRIE_FAILURE;
}

size_t TrieCount(trie_t *trie)
{
	size_t count = 0;
	assert(NULL != trie);

	RecCountLeaf(&trie->root, trie->height, &count);
	return count;
}

/*----------------Static-------------------*/
static void PostOrderDestroy(trie_node_t *node)
{
	if (node == NULL)
	{
		return;
	}

	PostOrderDestroy(node->children[LEFT]);
	PostOrderDestroy(node->children[RIGHT]);

	free(node);
}

static trie_node_t *CreateNode(trie_node_t *father)
{
	size_t i = 0;
	trie_node_t *node = malloc(sizeof(trie_node_t));
	if (NULL == node)
	{
		return NULL;
	}
	node->parent = father;
	node->is_used = 0;
	for (i = 0; i < NUM_OF_CHILDREN; ++i)
	{
		node->children[i] = NULL;
	}

	return node;
}

static void FindCloseRequested(unsigned char *str, size_t height)
{
	size_t i = 0;
	for (i = 0; i < height; ++i)
	{
		*str = 0;
		++str;
	}
}

static int RecCreatePath(trie_node_t *node, size_t height, unsigned char *requested, unsigned char *result)
{
	int next_step = 0;
	int status = 0;

	if (1 == node->is_used)
	{
		return TRIE_OCUPIED;
	}
	if (height == 1)
	{
		node->is_used = 1;
		return TRIE_SUCCESS;
	}
	next_step = (int)*requested;

	if (NULL == node->children[next_step])
	{
		node->children[next_step] = CreateNode(node);
		if (NULL == node->children[next_step])
		{
			return TRIE_FAILURE;
		}
	}

	status = RecCreatePath(node->children[next_step], height - 1, requested + 1, result + 1);
	if (TRIE_OCUPIED == status && LEFT == next_step) /*TODO: separate function*/
	{
		FindCloseRequested(requested, height);
		next_step = RIGHT;
		if (NULL == node->children[next_step])
		{
			node->children[next_step] = CreateNode(node);
			if (NULL == node->children[next_step])
			{
				return TRIE_FAILURE;
			}
		}
		status = RecCreatePath(node->children[next_step], height - 1, requested + 1, result + 1);
	}
	if (TRIE_SUCCESS == status)
	{
		node->is_used = IsUsed(node);
		*result = next_step;
	}

	return status;
}

static int IsUsed(trie_node_t *node)
{
	int i = 0;

	for (i = 0; i < NUM_OF_CHILDREN; i++)
	{
		if (node->children[i] == NULL)
		{
			return 0;
		}
		else if (node->children[i]->is_used == 0)
		{
			return 0;
		}
	}
	return 1;
}

static void RecCountLeaf(trie_node_t *node, size_t height, size_t *count)
{
	if (NULL == node)
	{
		return;
	}
	if (1 == height)
	{
		if (1 == node->is_used)
		{
			++(*count);
		}
		return;
	}

	RecCountLeaf(node->children[LEFT], height - 1, count);
	RecCountLeaf(node->children[RIGHT], height - 1, count);
}