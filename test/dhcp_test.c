/*
dev: barak
rev:
date:
status:
*/
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "dhcp.h"
#include "test.h"

static void TestDHCPCreate(void);
static void TestDHCPAllocate(void);
static void TestDHCPFree(void);
static void TestDHCPCount(void);

int main()
{
	TestDHCPCreate();
	TestDHCPAllocate();
	TestDHCPFree();
	TestDHCPCount();
	return 0;
}

static void TestDHCPCreate(void)
{
	dhcp_t *dhcp = NULL;
	unsigned char subnet_ip[IPV] = {255, 128, 255, 0};

	PrintName("Create");
	dhcp = DHCPCreate(subnet_ip, 8);

	Test(NULL != dhcp);
	Test(253 == DHCPCountFree(dhcp));

	DHCPDestroy(dhcp);
}

static void TestDHCPAllocate(void)
{
	unsigned char subnet_ip[IPV] = {192, 168, 31, 0};
	unsigned char alloc_ips[][IPV] = {
		{192, 168, 31, 11},
		{192, 168, 31, 12},
		{192, 168, 31, 13},
		{192, 168, 31, 14},
		{192, 168, 31, 15}};
	unsigned char res_ip[IPV] = {0, 0, 0, 0};
	size_t size = 5;
	size_t i = 0;
	dhcp_t *dhcp = DHCPCreate(subnet_ip, 8);

	PrintName("allocate");
	if (NULL == dhcp)
	{
		Test(0);
		return;
	}

	for (i = 0; i < size - 1; i++)
	{
		/* printf("%lu \n", DHCPCountFree(dhcp)); */
		Test(pow(2, 8) - 3 - i == DHCPCountFree(dhcp));
		Test(DHCP_SUCCESS == DHCPAllocateIp(dhcp, alloc_ips[i], res_ip));
		Test(0 == memcmp(res_ip, alloc_ips[i], IPV));
	}

	Test(DHCP_SUCCESS == DHCPAllocateIp(dhcp, alloc_ips[0], res_ip));
	Test(0 == memcmp(res_ip, alloc_ips[size - 1], IPV));

	PrintName("Recueste 10 time 192, 168, 31, 11 after until 15 is full\n");
	for (i = 0; i < 10; i++)
	{
		DHCPAllocateIp(dhcp, alloc_ips[0], res_ip);
		printf("%d.%d.%d.%d\n", res_ip[0], res_ip[1], res_ip[2], res_ip[3]);
	}

	DHCPDestroy(dhcp);
}

static void TestDHCPFree(void)
{
	dhcp_t *dhcp = NULL;
	unsigned char subnet_ip[IPV] = {192, 168, 31, 0};
	unsigned char alloc_ips[][IPV] = {
		{192, 168, 31, 11},
		{192, 168, 31, 12},
		{192, 168, 31, 13},
		{192, 168, 31, 14},
		{192, 168, 31, 15}};
	unsigned char res_ip[IPV] = {0, 0, 0, 0};
	size_t size = sizeof(alloc_ips) / sizeof(alloc_ips[0]);
	size_t i = 0;
	printf("\n");

	PrintName("Free");

	dhcp = DHCPCreate(subnet_ip, 8);
	if (NULL == dhcp)
	{
		Test(0);
		return;
	}

	for (i = 0; i < size; ++i)
	{
		Test(pow(2, 8) - 3 - i == DHCPCountFree(dhcp));
		Test(DHCP_SUCCESS == DHCPAllocateIp(dhcp, alloc_ips[i], res_ip));
	}

	DHCPFree(dhcp, alloc_ips[2]);

	Test(DHCP_SUCCESS == DHCPAllocateIp(dhcp, alloc_ips[0], res_ip));
	Test(0 == memcmp(res_ip, alloc_ips[2], IPV));

	for (i = 0; i < size; ++i)
	{
		Test(pow(2, 8) - 3 - size + i == DHCPCountFree(dhcp));
		DHCPFree(dhcp, alloc_ips[i]);
	}

	DHCPDestroy(dhcp);
	printf("\n");
}

static void TestDHCPCount(void)
{
	dhcp_t *dhcp = NULL;
	unsigned char subnet_ip[IPV] = {192, 168, 31, 0};
	unsigned char alloc_ip[IPV] = {192, 168, 31, 11};
	unsigned char res_ip[IPV] = {0, 0, 0, 0};

	PrintName("Count");

	dhcp = DHCPCreate(subnet_ip, 8);
	if (NULL == dhcp)
	{
		Test(0);
		return;
	}

	Test(pow(2, 8) - 3 == DHCPCountFree(dhcp));

	Test(DHCP_SUCCESS == DHCPAllocateIp(dhcp, alloc_ip, res_ip));
	Test(pow(2, 8) - 4 == DHCPCountFree(dhcp));

	Test(DHCP_SUCCESS == DHCPAllocateIp(dhcp, alloc_ip, res_ip));
	Test(DHCP_SUCCESS == DHCPAllocateIp(dhcp, alloc_ip, res_ip));
	Test(DHCP_SUCCESS == DHCPAllocateIp(dhcp, alloc_ip, res_ip));

	Test(pow(2, 8) - 7 == DHCPCountFree(dhcp));

	DHCPFree(dhcp, alloc_ip);
	Test(pow(2, 8) - 6 == DHCPCountFree(dhcp));

	DHCPFree(dhcp, res_ip);
	Test(pow(2, 8) - 5 == DHCPCountFree(dhcp));

	Test(DHCP_SUCCESS == DHCPAllocateIp(dhcp, alloc_ip, res_ip));
	Test(pow(2, 8) - 6 == DHCPCountFree(dhcp));

	DHCPDestroy(dhcp);
}
