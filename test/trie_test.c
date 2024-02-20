#include <stdio.h>
#include <string.h>

#include "trie.h"
#include "test.h"

static void TestCreate(void);
static void TestPath(void);

int main()
{
	TestCreate();
	TestPath();
	return 0;
}

static void TestCreate(void)
{
	trie_t *trie = NULL;

	PrintName("Create");

	trie = TrieCreate(9);
	Test(NULL != trie);

	TrieDestroy(trie);
}

static void TestPath(void)
{
	trie_t *trie = NULL;
	int i = 0;
	unsigned char ip[10] = {1, 0, 0, 1, 1, 0, 0, 1, 1, '\0'};
	unsigned char ip2[10] = {1, 1, 1, 1, 1, 1, 1, 1, 0, '\0'};

	unsigned char result[10] = {'\0'};

	PrintName("Create Path");
	trie = TrieCreate(10);
	if (trie == NULL)
	{
		return;
	}
	Test(0 == TrieCreatePath(trie, ip, result));

	for (i = 0; i < 9; ++i)
	{
		/* printf("ip = %d, result = %d\n", ip[i], result[i]); */
		Test(ip[i] == result[i]);
	}

	Test(0 == TrieCreatePath(trie, ip2, result));
	for (i = 0; i < 8; ++i)
	{
		Test(result[i] == 1);
	}
	Test(result[8] == 0);
	Test(0 == TrieCreatePath(trie, ip2, result));
	for (i = 0; i < 9; ++i)
	{
		Test(result[i] == 1);
	}

	PrintName("Remove Path");
	Test(3 == TrieCount(trie));

	Test(0 == TrieRemove(trie, result));
	Test(2 == TrieCount(trie));
	Test(0 != TrieRemove(trie, result));

	TrieDestroy(trie);
}
