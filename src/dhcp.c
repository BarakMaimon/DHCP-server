/*
dev: barak
rev:
date:
status:
*/
#include <string.h> /*srcmp*/
#include <stdlib.h> /*malloc*/
#include <assert.h> /*assert*/

#include "trie.h"
#include "dhcp.h"

#define BITS_IN_BYTE 8
#define BITS_IN_IPV (IPV * BITS_IN_BYTE)
#define TWO_POW(num) (1 << num)
#define TOTAL_IPS(dhcp) (TWO_POW(dhcp->num_of_bits))
#define SUBNET_BITS(dhcp) (BITS_IN_IPV - dhcp->num_of_bits)

struct dhcp
{
	unsigned char subnet_ip[IPV];
	size_t num_of_bits;
	trie_t *addresses;
};

static trie_t *InitTrie(size_t num_of_bits_in_host);
static void BytesArrToBitsArr(const unsigned char *bytes, unsigned char *bits);
static void BitsArrToBytesArr(const unsigned char *bits, unsigned char *bytes);
static unsigned char GetBit(const unsigned char *bytes, size_t index);
static unsigned char GetByte(const unsigned char *bits, size_t index);

/*========================FUNCTIONS====================================*/

dhcp_t *DHCPCreate(const unsigned char subnet_ip[IPV], size_t num_of_bits)
{
	dhcp_t *dhcp = NULL;

	assert(NULL != subnet_ip);
	assert(0 < num_of_bits);
	assert((IPV * BITS_IN_BYTE) > num_of_bits);

	dhcp = malloc(sizeof(dhcp_t));
	if (NULL == dhcp)
	{
		return NULL;
	}
	memcpy(dhcp->subnet_ip, subnet_ip, IPV);
	dhcp->num_of_bits = num_of_bits;
	dhcp->addresses = InitTrie(num_of_bits);

	return dhcp;
}

void DHCPDestroy(dhcp_t *dhcp)
{
	assert(NULL != dhcp);

	TrieDestroy(dhcp->addresses);
	free(dhcp);
}

dhcp_status_t DHCPAllocateIp(dhcp_t *dhcp, const unsigned char Ip[IPV], unsigned char result[IPV])
{
	unsigned char ip_in_bits[BITS_IN_IPV + 1];
	unsigned char result_in_bits[BITS_IN_IPV + 1];

	assert(NULL != dhcp);
	assert(NULL != Ip);
	assert(NULL != result);
	assert(0 != strncmp((char *)Ip, (char *)dhcp->subnet_ip, SUBNET_BITS(dhcp)));

	BytesArrToBitsArr(Ip, ip_in_bits); /*TODO: optimize*/
	BytesArrToBitsArr(Ip, result_in_bits);

	if (DHCP_SUCCESS != TrieCreatePath(dhcp->addresses, ip_in_bits + SUBNET_BITS(dhcp), result_in_bits + SUBNET_BITS(dhcp)))
	{
		return DHCP_FAILURE;
	}
	BitsArrToBytesArr(result_in_bits, result);

	return DHCP_SUCCESS;
}

size_t DHCPCountFree(const dhcp_t *dhcp)
{
	assert(NULL != dhcp);

	return (TOTAL_IPS(dhcp) - TrieCount(dhcp->addresses));
}

dhcp_status_t DHCPFree(dhcp_t *dhcp, const unsigned char ip[IPV])
{
	unsigned char ip_bits[BITS_IN_IPV] = {'\0'};

	assert(NULL != dhcp);
	assert(NULL != ip);

	BytesArrToBitsArr(ip, ip_bits); /**/

	if (DHCP_SUCCESS != TrieRemove(dhcp->addresses, (ip_bits + SUBNET_BITS(dhcp))))
	{
		return DHCP_DOUBLE_FREE;
	}

	return DHCP_SUCCESS;
}

/*----------------static ----------------------*/
static trie_t *InitTrie(size_t num_of_bits_in_host)
{
	unsigned char *preallocated_address = NULL;
	trie_t *trie = TrieCreate(num_of_bits_in_host + 1);
	size_t i = 0;
	int status = 0;
	if (NULL == trie)
	{
		return NULL;
	}

	preallocated_address = (unsigned char *)malloc(num_of_bits_in_host * sizeof(char));
	if (NULL == preallocated_address)
	{
		TrieDestroy(trie);
		return NULL;
	}

	for (i = 0; i < num_of_bits_in_host; ++i)
	{
		preallocated_address[i] = 0;
	}

	status = TrieCreatePath(trie, preallocated_address, preallocated_address);
	if (DHCP_SUCCESS != status)
	{
		TrieDestroy(trie);
		free(preallocated_address);
		return NULL;
	}

	for (i = 0; i < num_of_bits_in_host; ++i)
	{
		preallocated_address[i] = 1;
	}

	status = TrieCreatePath(trie, preallocated_address, preallocated_address);
	if (DHCP_SUCCESS != status)
	{
		TrieDestroy(trie);
		free(preallocated_address);
		return NULL;
	}

	preallocated_address[num_of_bits_in_host - 1] = 0;
	status = TrieCreatePath(trie, preallocated_address, preallocated_address);
	if (DHCP_SUCCESS != status)
	{
		TrieDestroy(trie);
		free(preallocated_address);
		return NULL;
	}

	free(preallocated_address);

	return trie;
}

static void BytesArrToBitsArr(const unsigned char *bytes, unsigned char *bits)
{
	size_t i = 0;

	for (i = 0; i < IPV * BITS_IN_BYTE; ++i)
	{
		bits[i] = GetBit(bytes, i);
	}
}

static void BitsArrToBytesArr(const unsigned char *bits, unsigned char *bytes)
{
	size_t i = 0;

	for (i = 0; i < IPV; ++i)
	{
		bytes[i] = GetByte(bits, i);
	}
}

static unsigned char GetBit(const unsigned char *bytes, size_t index)
{
	assert(NULL != bytes);

	while (BITS_IN_BYTE <= index)
	{
		index -= BITS_IN_BYTE;
		++bytes;
	}

	return ((*bytes >> (BITS_IN_BYTE - index - 1)) & 1);
}
static unsigned char GetByte(const unsigned char *bits, size_t index)
{
	size_t i = 0;
	unsigned char byte = 0;

	assert(NULL != bits);

	while (0 < index)
	{
		--index;
		bits += BITS_IN_BYTE;
	}

	for (i = 0; i < BITS_IN_BYTE; ++i)
	{
		byte <<= 1;
		byte += bits[i];
	}

	return byte;
}
