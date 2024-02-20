#ifndef __DHCP_H__
#define __DHCP_H__

#include <stdio.h>

typedef struct dhcp dhcp_t;

#define IPV (4)

typedef enum dhcp_status
{
    DHCP_SUCCESS,
    DHCP_FAILURE,
    DHCP_DOUBLE_FREE
} dhcp_status_t;

/*
*    Creates DHCP 
*
*    Arguments:
*		subnet_ip[IPV]: IP network address. Must be valid
*       num_of_bits: Represents the number of bits of host IP. Must be greater than zero.
*
*    Return: a pointer to the new DHCP if successful,
*        otherwise returns NULL.
*
*    Time complexity: O(log n) best/average/worst
*    Space complexity: O(log n) best/average/worst
*/
dhcp_t *DHCPCreate(const unsigned char subnet_ip[IPV], size_t num_of_bits);

/*
*    Deallocates all memory allocated for DHCP
*
*    Arguments:
*        DHCP - Pointer to the DHCP to destroy. Must be valid.
*
*    Return: void (Always succeeds).
*
*    Time complexity: O(n) best/average/worst
*    Space complexity: O(1) best/average/worst
*/
void DHCPDestroy(dhcp_t *dhcp);

/*
*	Allocates an IP address.
*
*	Arguments:
*		DHCP - Pointer to the DHCP. must be valid.
*       IP[IPV] - Required IP address. Must be valid.
*       result[IPV] - Container for the returned address. Returns the required address if available, 
*                       otherwise returns next available address.
*
*	Return:	DHCP status.
*
*	Time complexity: O(log n) best/average/worst
*	Space complexity: O(1) best, O(log n) average/worst
*/
dhcp_status_t DHCPAllocateIp(dhcp_t *dhcp, const unsigned char Ip[IPV], unsigned char result[IPV]);

/*
*	Deallocates an IP address.
*
*	Arguments:
*		DHCP - Pointer to the DHCP. Must be valid.
*       IP[IPV] - IP address to be free. Must be valid.
*       
*	Return:	DHCP status.
*
*	Time complexity: O(log n) best/average/worst
*	Space complexity: O(1) best/average/worst
*/
dhcp_status_t DHCPFree(dhcp_t *dhcp, const unsigned char ip[IPV]);

/*
*	Counts the number of available IP addresses.
*
*	Arguments:
*		DHCP - Pointer to the DHCP. Must be valid.
*       
*	Return:	number of available IP addresses.
*
*	Time complexity: O(n) best/average/worst
*	Space complexity: O(1) best/average/worst
*/
size_t DHCPCountFree(const dhcp_t *dhcp);

#endif /*__DHCP_H__*/