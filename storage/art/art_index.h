/*
  Art_index.h
 
  This header file defines a simple index class that can
  be used to store file pointer indexes (long long). The 
  class keeps the entire index in memory for fast access.
  The internal memory structure is a linked list. While
  not as efficient as a btree, it should be usable for
  most testing environments. The constructor accepts the 
  max key length. This is used for all nodes in the index.

  File Layout:
    SOF                              max_key_len (int)
    SOF + sizeof(int)                crashed (bool)
    SOF + sizeof(int) + sizeof(bool) DATA BEGINS HERE
*/
#include "my_sys.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NODE4   1
#define NODE16  2
#define NODE48  3
#define NODE256 4

#define MAX_PREFIX_LEN 10


#if defined(__GNUC__) && !defined(__clang__)
# if __STDC_VERSION__ >= 199901L && 402 == (__GNUC__ * 100 + __GNUC_MINOR__)
/*
 * GCC 4.2.2's C99 inline keyword support is pretty broken; avoid. Introduced in
 * GCC 4.2.something, fixed in 4.3.0. So checking for specific major.minor of
 * 4.2 is fine.
 */
#  define BROKEN_GCC_C99_INLINE
# endif
#endif


const long METADATA_SIZE = sizeof(int) + sizeof(bool);

/**
 * This struct is included as part
 * of all the various node sizes
 */
typedef struct {
    uint32_t partial_len;
    uint8_t type;
    uint8_t num_children;
    unsigned char partial[MAX_PREFIX_LEN];
} art_node;

/**
 * Small node with only 4 children
 */
typedef struct {
    art_node n;
    unsigned char keys[4];
    art_node *children[4];
} art_node4;

/**
 * Node with 16 children
 */
typedef struct {
    art_node n;
    unsigned char keys[16];
    art_node *children[16];
} art_node16;

/**
 * Node with 48 children, but
 * a full 256 byte field.
 */
typedef struct {
    art_node n;
    unsigned char keys[256];
    art_node *children[48];
} art_node48;

/**
 * Full node with 256 children
 */
typedef struct {
    art_node n;
    art_node *children[256];
} art_node256;

/**
 * Represents a leaf. These are
 * of arbitrary size, as they include the key.
 */
typedef struct {
    long long pos;
    int key_len;
    uchar key[];
} art_leaf;

/**
 * Main struct, points to root.
 */
typedef struct {
    art_node *root;
    uint64_t size;
} art_tree;

#ifdef __cplusplus
}
#endif

class Art_index
{
public:
  //Initializes an ART tree
  Art_index(art_tree *t,int keylen);
  Art_index();
  ~Art_index(void);
  // Open an ART tree
  int open_index(char *path);
  // Create an ART tree
  int create_index(char *path, int keylen);
  art_node* alloc_node(uint8_t type);
  // Destroys an ART node
  void destroy_node(art_node *n);
  // unsigned long int art_size(art_tree *t);
  // find the child node
  art_node** find_child(art_node *n, uchar c);
  // output the min number
  int min(int a, int b);
  // Returns the number of prefix characters shared between the key and node.
  int check_prefix(const art_node *n, const uchar *key, int key_len, int depth);
  // Checks if a leaf matches
  int leaf_matches(const art_leaf *n, const uchar *key, int key_len, int depth);
  // Find the minimum leaf under a node
  art_leaf* minimum(const art_node *n);
  // Find the maximum leaf under a node
  art_leaf* maximum(const art_node *n);
  // Returns the minimum valued leaf
  art_leaf* art_minimum(const art_tree *t);
  // Returns the maximum valued leaf
  art_leaf* art_maximum(const art_tree *t);
  // create the leaf
  art_leaf* make_leaf(const uchar *key, int key_len, long long pos);
  // get longest common prefix
  int longest_common_prefix(art_leaf *l1, art_leaf *l2, int depth);
  void copy_header(art_node *dest, art_node *src);
  void add_child256(art_node256 *n, art_node **ref, unsigned char c, void *child);
  void add_child48(art_node48 *n, art_node **ref, unsigned char c, void *child);
  void add_child16(art_node16 *n, art_node **ref, unsigned char c, void *child);
  void add_child4(art_node4 *n, art_node **ref, unsigned char c, void *child);
  void add_child(art_node *n, art_node **ref, unsigned char c, void *child);
  // Calculates the index at which the prefixes mismatch
  int prefix_mismatch(const art_node *n, const uchar *key, int key_len, int depth);
  long long recursive_insert(art_node *n, art_node **ref, const uchar *key, int key_len, long long pos, int depth, int *old, int replace);
  //inserts a new key with pos into the art tree
  long long insert_key(art_tree *t, const uchar *key, long long pos , int length);
  void remove_child256(art_node256 *n, art_node **ref, unsigned char c);
  void remove_child48(art_node48 *n, art_node **ref, unsigned char c);
  void remove_child16(art_node16 *n, art_node **ref, art_node **l);
  void remove_child4(art_node4 *n, art_node **ref, art_node **l);
  void remove_child(art_node *n, art_node **ref, unsigned char c, art_node **l);
  art_leaf* recursive_delete(art_node *n, art_node **ref, const uchar *key, int key_len, int depth);
  // Delete the key
  long long delete_key(art_tree *t, const uchar *key, int key_len);
  // Find the first leaf key
  uchar* get_first_key(const art_tree *t);
  // Find the first leaf pos
  long long get_first_pos(const art_tree *t);
  // Find the last leaf key
  uchar* get_last_key(const art_tree *t);
  // Find the last leaf pos
  long long get_last_pos(const art_tree *t);
  // seek the leaf
  art_leaf* seek_index(const art_tree *t, const uchar *key, int key_len);
  // Get the pos
  long long get_index_pos(const art_tree *t, const uchar *key, int key_len);
  // Update an ART key and pos
  long long update_key(art_tree *t, uchar *key, int key_len, long long pos);
  // Destroys an ART tree
  int destroy_index(art_tree *t);
  // Save index to file 
  int save_index(art_tree *t);
  // Load index from file
  int load_index(art_tree *t);
  // Close index file
  int close_index();

  // Range scan support - Redesigned simple iterator
  int init_iterator(const art_tree *t);
  art_leaf* get_next_leaf();
  art_leaf* get_prev_leaf();
  int reset_to_first();
  int reset_to_last();
  int seek_iterator(const uchar *key, int key_len);
  void cleanup_iterator();

  //art_tree *index_tree;
private:
  File index_file;
  int max_key_len;
  int block_size;
  
  // Simplified iterator using linear leaf collection with caching
  struct simple_iterator {
    art_leaf **leaves;          // Array of all leaves (sorted)
    int total_leaves;           // Total number of leaves
    int current_index;          // Current position in array
    int capacity;               // Array capacity
    bool valid;                 // Iterator validity
    unsigned long cache_version; // Version for cache invalidation
    int max_capacity_used;      // Track historical peak usage
    int resize_count;           // Count of resize operations
  };
  
  // Iterator state
  const art_tree *iter_tree;           // Tree being iterated
  simple_iterator iterator;            // Simple linear iterator
  unsigned long tree_version;          // Global version counter for cache invalidation
  
  // Helper methods for leaf collection
  void collect_all_leaves(art_node *node, art_leaf ***leaves_ptr, int *count, int *capacity);
  void resize_leaf_array(art_leaf ***leaves_ptr, int *capacity);
  static int compare_leaf_keys(const void *a, const void *b);
  
  // Dynamic capacity management
  int calculate_initial_capacity(const art_tree *t);
  int calculate_resize_capacity(int current_capacity, int current_count);
  int get_resize_threshold(int capacity);
};


