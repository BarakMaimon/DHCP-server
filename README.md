# DHCP Server

![C](https://img.shields.io/badge/C-ANSI-A8B9CC?logo=c&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20POSIX-FCC624?logo=linux&logoColor=black)
![Data Structure](https://img.shields.io/badge/Data%20Structure-Binary%20Trie-blue)

An IP address pool manager for a DHCP server, implemented in C using a **binary trie** over the host portion of the subnet. Allocates addresses in O(log n) time, returns the next available address when the requested one is taken, and detects double-frees.

---

## The core idea

A subnet with `n` host bits has 2ⁿ possible addresses. The naive approach — a bitmap or sorted list — either wastes memory or requires a linear scan to find the next free address when the preferred one is taken.

This implementation maps the host portion of each IP to a **path in a binary trie**: each bit in the host address is a LEFT (0) or RIGHT (1) branch. Allocation traverses the trie following the requested bits; if a node is already full, the algorithm backtracks one level and tries the right sibling — effectively finding the next available address in O(log n) without scanning.

```
Host bits of a /24 subnet (8 bits → trie height 9):

              root
             /    \
           0        1          ← bit 7 (MSB)
          / \      / \
         0   1    0   1        ← bit 6
        ...                   ...
       /   \
    leaf   leaf               ← bit 0 (LSB)
  (used) (free)

Requesting 192.168.1.5  (host = 00000101):
  If .5 is taken → backtrack → try 00000110 (.6) → try 00000111 (.7) → …
  Always O(log n) — no linear scan.
```

---

## Features

- Allocate a specific IP; fall back to the next available if taken
- Free an IP and return it to the pool
- Detect double-free attempts (returns a distinct error code)
- Network and broadcast addresses reserved automatically at creation
- Count of available addresses at any time

---

## API

Declared in `include/dhcp.h`. Link `src/dhcp.c` and `src/trie.c`.

```c
/* Create a DHCP pool for the given subnet.
 *
 * subnet_ip    — network address as 4 bytes, e.g. {192, 168, 1, 0}
 * num_of_bits  — number of HOST bits (e.g. 8 for a /24)
 *
 * Automatically reserves: network address (all-zeros host),
 * broadcast address (all-ones host), and one additional
 * infrastructure address.
 *
 * Returns NULL on allocation failure.
 */
dhcp_t *DHCPCreate(const unsigned char subnet_ip[4], size_t num_of_bits);

/* Release all resources. */
void DHCPDestroy(dhcp_t *dhcp);

/* Allocate an IP address.
 *
 * ip      — preferred address (4 bytes); must belong to the subnet.
 * result  — receives the assigned address (may differ from ip if taken).
 *
 * Returns DHCP_SUCCESS on allocation, DHCP_FAILURE if the pool is full.
 */
dhcp_status_t DHCPAllocateIp(dhcp_t *dhcp,
                              const unsigned char ip[4],
                              unsigned char result[4]);

/* Free a previously allocated address.
 *
 * Returns DHCP_SUCCESS on success, DHCP_DOUBLE_FREE if the address
 * was not allocated (double-free guard).
 */
dhcp_status_t DHCPFree(dhcp_t *dhcp, const unsigned char ip[4]);

/* Return the number of currently available (unallocated) addresses.
 * Reserved addresses are not counted.
 */
size_t DHCPCountFree(const dhcp_t *dhcp);
```

### Return codes

| Code | Value | Meaning |
|---|---|---|
| `DHCP_SUCCESS` | 0 | Operation succeeded |
| `DHCP_FAILURE` | 1 | Pool is full (allocation) or other error |
| `DHCP_DOUBLE_FREE` | 2 | Address was already free |

---

## Allocation algorithm (step by step)

Given requested IP `192.168.1.5` in a `/24` subnet:

1. Strip the network prefix (24 bits) — keep only the 8 host bits: `00000101`.
2. Walk the trie bit by bit from MSB to LSB, creating nodes on demand.
3. At each level: follow the requested bit.
   - If the node at that bit is already marked **full** and we arrived from the left (bit = 0), switch to the right sibling (bit = 1) and zero out all remaining requested bits.
   - Recurse downward in the new subtree.
4. Mark the reached leaf as used. Propagate the **full** flag upward if both children are now full.
5. Convert the actual path taken back to a 4-byte IP and write it to `result`.

**Result**: If `.5` was free, `result = 192.168.1.5`. If `.5` was taken, `result` is the lowest available address above it — e.g. `.6`, `.7`, … — without scanning.

---

## Reserved addresses

Three addresses are marked unavailable at `DHCPCreate` time and can never be allocated:

| Address | Host bits | Example (/24) |
|---|---|---|
| Network address | all zeros | `192.168.1.0` |
| Broadcast address | all ones | `192.168.1.255` |
| Infrastructure address | MSB = 1, rest zero | `192.168.1.128` |

A `/24` subnet therefore has 256 − 3 = **253 assignable addresses**.

---

## Trie internals

```c
/* trie.h / trie.c */

typedef struct trie_node {
    struct trie_node *children[2]; /* [0] = LEFT, [1] = RIGHT */
    struct trie_node *parent;
    int is_used;  /* leaf: 1 = allocated; internal: 1 = subtree full */
} trie_node_t;

struct trie {
    size_t height;    /* num_of_bits + 1 */
    trie_node_t root; /* embedded root, not heap-allocated */
};
```

**`is_used` semantics:**
- On a **leaf**: `1` means the address is allocated.
- On an **internal node**: `1` means the entire subtree is full — no need to descend; backtrack immediately.

This single flag is what makes the fallback search O(log n) rather than O(n).

**Nodes are created lazily** — only the paths that have actually been allocated exist in memory. A mostly-empty /24 pool has far fewer than 511 nodes.

---

## Complexity

| Operation | Time | Space |
|---|---|---|
| `DHCPAllocateIp` | O(b) | O(b) nodes created |
| `DHCPFree` | O(b) | O(1) |
| `DHCPCountFree` | O(2ᵇ) | O(1) |
| `DHCPCreate` | O(b) | O(b) |
| `DHCPDestroy` | O(k) | — |

`b` = host bits (e.g. 8 for /24). `k` = number of nodes currently in the trie.

`DHCPCountFree` traverses the whole trie; for a /24 it visits at most 511 nodes.

---

## Project structure

```
DHCP-server/
├── include/
│   ├── dhcp.h          — Public DHCP API and status codes
│   └── trie.h          — Public Trie API
├── src/
│   ├── dhcp.c          — DHCP implementation (allocation, free, count, IP ↔ bit conversion)
│   └── trie.c          — Binary trie (create, insert path, remove path, count, destroy)
└── test/
    ├── dhcp_test.c     — DHCP unit tests (create, allocate, free, count)
    └── trie_test.c     — Trie unit tests (path creation, removal, count)
```

---

## Building & running tests

No external dependencies — standard C library only.

```bash
# Compile and run DHCP tests
gcc -Wall -o dhcp_test src/dhcp.c src/trie.c test/dhcp_test.c -Iinclude
./dhcp_test

# Compile and run trie tests
gcc -Wall -o trie_test src/trie.c test/trie_test.c -Iinclude
./trie_test
```

### What the tests cover

**Trie tests (`trie_test.c`):**
- Create a trie of height 9 (8 host bits)
- Allocate a specific path; verify the returned path matches
- Allocate a second path; verify fallback when first is taken
- Remove a path; verify it can be reallocated
- Count allocated paths

**DHCP tests (`dhcp_test.c`):**
- Create a /24 pool; verify 253 addresses available (256 − 3 reserved)
- Allocate 5 consecutive addresses; verify count decrements
- Request an already-allocated address; verify next-available is returned
- Free an address; verify it becomes allocatable again
- Stress: request the same IP repeatedly until pool fills; verify all unique

---

## Design decisions

**Why a binary trie over a bitmap or hash table?**

A bitmap gives O(1) mark/unmark but finding the *next* free address after a collision requires a linear scan. A hash table has O(1) average lookup but the same problem. The trie's `is_used` flag on internal nodes lets the fallback search skip entire full subtrees in O(log n), with no scan.

**Why lazy node creation?**

Allocating all 2ⁿ leaf nodes upfront for a /16 (65 536 addresses) would be expensive. Lazy creation means memory is proportional to the number of *active* allocations, not the subnet size.

**Why bit-level representation during traversal?**

Converting the IP to a 32-element boolean array before entering the trie means each array index maps directly to one trie level. The alternative — bit-shifting inside the recursive call — would be correct but harder to follow.

**Why parent pointers?**

Stored for each node to support future bottom-up pruning (removing empty internal nodes after deallocation to keep the trie compact). The current implementation does not prune on free but the pointer is there when needed.

---

## License

MIT
