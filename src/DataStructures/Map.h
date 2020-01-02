//
// Created by s152717 on 24-12-2019.
//

#ifndef YADF_MAP_H
#define YADF_MAP_H

//
// Created by ieperen3039 on 28-11-19.
//

#include <stdlib.h>
#include <stdbool.h>
#include "global.h"

// multiplier of map size upon reallocation
#define MAP_SIZE_INCREASE 4.0
// reallocate to the same size if it gives at most this compaction
#define MAP_REQ_COMPACTION 0.75
// more bins gives a more efficient map, but less room until it is full
#define MAP_DEFAULT_BINS 12
// initial number of elements to be stored in the map
#define MAP_DEFAULT_SIZE 16

struct _MapNodeHeader {
    unsigned int _key;
    struct _MapNodeHeader* _next;
};

/// By storing pointers, one can use this to make a map that allows null elements
typedef struct {
    struct _MapNodeHeader** _bins;
    void* _data; // array with all elements
    size_t _element_size;
    int _size; // in number of elements
    int _num_bins; // in number of elements
    int _max_size; // in number of elements
    int _empty_index; // in number of elements FROM _data
    float _bin_factor;
} ElementMap;

PURE static inline size_t _map_get_data_block_size(size_t element_size, int max_size, int num_bins) {
    size_t bin_table_elt_size = sizeof(struct _MapNodeHeader*);
    size_t header_size = sizeof(struct _MapNodeHeader);
    return
            bin_table_elt_size * num_bins + // pointer array, one element for each bin
            (header_size + element_size) * max_size; // (header + element) array
}

/**
 * Initializes a map, assuming it is not initialized before.
 *
 * A map stores key - element pairs, allowing approximately O(1) element access given a key.
 * Elements are stored by copying them to its internal structure.
 *
 * @param map an uninitialized map
 * @param element_size the size of a single element added to the map
 * @param max_size
 * @param backing_factor
 * @return map
 */
static inline ElementMap* map_init_sized(ElementMap* map, size_t element_size, int max_size, float backing_factor) {
    int num_bins = (int) ((float) max_size * backing_factor);
    map->_element_size = element_size;
    if (max_size > 0) {
        map->_bins = malloc(_map_get_data_block_size(element_size, max_size, num_bins));
        map->_data = &map->_bins[num_bins];
    } else {
        map->_bins = NULL;
        map->_data = NULL;
    }
    map->_size = 0;
    map->_num_bins = num_bins;
    map->_max_size = max_size;
    map->_bin_factor = backing_factor;
    map->_empty_index = 0;

    for (int i = 0; i < num_bins; ++i) {
        map->_bins[i] = NULL;
    }

    return map;
}

static inline ElementMap* map_init(ElementMap* map, size_t element_size) {
    return map_init_sized(map, element_size, MAP_DEFAULT_SIZE, ((float) MAP_DEFAULT_BINS / MAP_DEFAULT_SIZE));
}

PURE static inline unsigned int hash(int key) {
    const unsigned int INT_PHI = 0x9E3779B9;
    unsigned int hash = key * INT_PHI;
    return (hash ^ (hash >> 16));
}

static bool map_remove(ElementMap* map, int key) {
    unsigned int bin = hash(key) % map->_num_bins;
    struct _MapNodeHeader* map_node = map->_bins[bin];
    if (map_node == NULL) return false;

    if (map_node->_key == key) {
        map->_bins[bin] = map_node->_next;
        return true;
    }

    struct _MapNodeHeader* prev_node = map_node;
    map_node = map_node->_next;

    while (map_node != NULL) {
        if (map_node->_key == key) {
            prev_node->_next = map_node->_next; // may be NULL
            return true;
        }
        prev_node = map_node;
        map_node = map_node->_next;
    };

    return false;
}

/** reallocates the map, increasing size if appropriate */
PURE static void map_realloc(ElementMap* map) {
    int new_size;

    if (map->_max_size == 0) {
        new_size = MAP_DEFAULT_SIZE;

    } else if ((map->_size / map->_max_size) < MAP_REQ_COMPACTION) {
        new_size = map->_max_size;

    } else {
        new_size = (int) (map->_max_size * MAP_SIZE_INCREASE);
    }

    int new_num_bins = (int) ((float) new_size * map->_bin_factor);

    size_t new_data_block_size = _map_get_data_block_size(map->_element_size, new_size, new_num_bins);
    struct _MapNodeHeader** new_bins = malloc(new_data_block_size);
    for (int i = 0; i < new_num_bins; ++i) new_bins[i] = NULL;

    void* new_data = &new_bins[new_num_bins];

    // transfer each element to new map
    int new_empty_index = 0;
    for (int i = 0; i < map->_num_bins; ++i) {
        struct _MapNodeHeader* old_elt = map->_bins[i];

        while (old_elt != NULL) {
            unsigned int bin = hash(old_elt->_key) % new_num_bins;
            struct _MapNodeHeader* predecessor = new_bins[bin];

            size_t header_elt_size = map->_element_size + sizeof(struct _MapNodeHeader);
            struct _MapNodeHeader* new_node = new_data + header_elt_size * new_empty_index;
            new_empty_index++;

            // find the logical position, override is impossible
            if (predecessor != NULL) {
                while (predecessor->_next != NULL) {
                    predecessor = predecessor->_next;
                }
                // set last node pointer to empty element
                predecessor->_next = new_node;

            } else {
                new_bins[bin] = new_node;
            }

            new_node->_key = old_elt->_key;
            new_node->_next = NULL;
            // node + 1, as node is of type NodeHeader*
            memcpy(new_node + 1, old_elt + 1, map->_element_size);

            old_elt = old_elt->_next;
        }
    }

    free(map->_bins);
    map->_bins = new_bins;
    map->_data = new_data;
    map->_max_size = new_size;
    map->_num_bins = new_num_bins;
    map->_empty_index = new_empty_index;
}

/** returns the next empty element. If no space is available, the map is resized */
static inline void* _map_get_next_empty(ElementMap* map) {
    if (map->_empty_index == map->_max_size) {
        // resize and re-allocate map
        map_realloc(map);
    }

    size_t header_elt_size = map->_element_size + sizeof(struct _MapNodeHeader);
    return map->_data + (header_elt_size * map->_empty_index++);
}

static void map_insert(ElementMap* map, int key, const void* element) {
    // set pointer
    unsigned int bin = hash(key) % map->_num_bins;
    struct _MapNodeHeader* node = map->_bins[bin];

    // find either the logical position, or an override
    if (node != NULL) {
        // while no override is found
        while (node->_key != key) {
            if (node->_next == NULL) { // if the logical position is found
                // set node pointer to empty element
                node->_next = _map_get_next_empty(map);
                // no override found, focus empty element
                node = node->_next;
                node->_next = NULL;
                break;
            }
            node = node->_next;
        }
    } else {
        node = _map_get_next_empty(map);
        node->_next = NULL;
        map->_bins[bin] = node;
    }

    // emplace element, preserve value of node.next
    node->_key = key;
    // node + 1 is right after the header
    memcpy(node + 1, element, map->_element_size);
    map->_size++;
}

/** 
 * @return the element associated with the given key, or NULL if the key does not exist 
 */
PURE static inline void* map_get(const ElementMap* map, int key) {
    unsigned int bin = hash(key) % map->_num_bins;
    struct _MapNodeHeader* map_node = map->_bins[bin];

    while (map_node != NULL) {
        if (map_node->_key == key) {
            return map_node + 1; // the element
        }
        map_node = map_node->_next;
    };

    return NULL;
}

/** return true iff an element with the given key exists. This is likely not faster than map_get() != NULL */
PURE static inline bool map_contains(const ElementMap* map, int key) {
    return map_get(map, key) != NULL;
}

PURE static inline void* map_get_or_default(const ElementMap* map, int key, void* default_value) {
    void* value = map_get(map, key);
    return value != NULL ? value : default_value;
}

PURE static inline int map_get_size(const ElementMap* map) {
    return map->_size;
}

static inline void map_free(const ElementMap* map) {
    free(map->_bins);
}

/**
 * returns the element with the given key.
 * If no such element exist, create an element using the given supplier, place it in the map, and return it.
 * Can be useful for nested data structures;
 *
 * @code
 * List* get_new_list(int){
 *      List* l = malloc(sizeof(List))
 *      return list_init(l, sizeof(int), 0);
 * }
 * [...]
 * List* elt = map_compute_if_absent(map, key, get_new_list);
 */
PURE static inline void* map_compute_if_absent(ElementMap* map, int key, void* (* supplier)()) {
    unsigned int bin = hash(key) % map->_num_bins;
    struct _MapNodeHeader* node = map->_bins[bin];

    if (node == NULL) {
        node = _map_get_next_empty(map);
        map->_bins[bin] = node;

    } else {
        while (1) {
            if (node->_key == key) {
                // element found
                // skip header
                return node + 1;
            }

            if (node->_next == NULL) {
                // element is absent
                node->_next = _map_get_next_empty(map);
                node = node->_next;
                break;
            }

            node = node->_next;
        }
    }

    // element not found; use supplier
    node->_key = key;
    node->_next = NULL; // if it was found, control would not be here
    memcpy(node + 1, supplier(), map->_element_size);

    return node + 1;
}

#endif //YADF_MAP_H
