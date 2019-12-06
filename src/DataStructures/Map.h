//
// Created by ieperen3039 on 28-11-19.
//

#ifndef YADF2_MAP_H
#define YADF2_MAP_H

#include <stdlib.h>
#include <stdbool.h>

// in total data size
#define SIZE_INCREASE 4.0

#define DEFAULT_BINS 0.5
#define DEFAULT_SIZE 32

typedef struct _MapNode MapNode;
struct _MapNode {
    void* pointer;
    unsigned int _hash;
    MapNode* _next;
};

typedef struct {
    MapNode* _data; // array with all elements
    int _size; // in number of elements
    int _num_bins; // in number of elements
    int _max_size; // in number of elements
    int _empty_index;
    int _holes;
    float _backing_factor;
} Map;

static inline void map_new_sized(int max_size, float backing_factor) {
    Map* map = malloc(sizeof(Map));
    int num_bins = (int) ((float) max_size * backing_factor);
    map->_data = malloc(max_size * sizeof(MapNode));
    map->_size = 0;
    map->_num_bins = num_bins;
    map->_max_size = max_size;
    map->_backing_factor = backing_factor;
    map->_empty_index = num_bins;
    map->_holes = 0;

    for (int i = 0; i < num_bins; ++i) {
        map->_data[i] = (MapNode) {NULL, 0, NULL};
    }
}

static inline void map_new() {
    map_new_sized(DEFAULT_SIZE, 1 - (float) (DEFAULT_SIZE * DEFAULT_BINS));
}

static inline bool map_remove(Map* map, int hash){
    int bin = hash % map->_num_bins;
    MapNode* map_node = &(map->_data[bin]);
    
    // when first is a hit
    if (map_node->_hash == hash){
        // move `next` if it exists
        if (map_node->_next != NULL) {
            *map_node = *map_node->_next;
            map_node->_next->pointer = NULL;
            map->_holes++;
        } else {
            map_node->pointer = NULL;
        }
        return true;
    }

    MapNode* prev_node = map_node;
    map_node = map_node->_next;

    while (map_node != NULL) {
        if (map_node->_hash == hash){
            prev_node->_next = map_node->_next; // may be NULL
            map_node->pointer = NULL;
            map->_holes++;
            return true;
        }
        map_node = map_node->_next;
    };

    return false;
}

static inline void map_insert(Map* map, int hash, void* element){
    if (element == NULL) {
        map_remove(map, hash);
        return;
    }
    
    map->_size++;
    if (map->_empty_index > map->_max_size){
        // resize and re-allocate map
        // TODO consider filling holes
        int new_size = (int) (map->_size * SIZE_INCREASE);
        int new_num_bins = (int) ((float) new_size * map->_backing_factor);
        int new_empty_index = new_num_bins;
        MapNode* new_data = malloc(new_size * sizeof(MapNode));
        
        // move all data to new map
        for (int i = 0; i < map->_size; i++) {
            MapNode map_node = map->_data[i];
            uint new_bin = map_node._hash % new_num_bins;
            MapNode* map_tgt = &(new_data[new_bin]);
            
            if (map_node.pointer == NULL){
                *map_tgt = (MapNode) {element, hash, NULL};

            } else {
                // add element
                MapNode* new_node = &(new_data[new_empty_index]);
                *new_node = (MapNode) {element, hash, NULL};
                new_empty_index++;

                MapNode* next = map_node._next;
                while (next->_next != NULL) {
                    next = next->_next;
                }

                next->_next = new_node;
            }
        }

        free(map->_data);
        map->_data = new_data;
        map->_max_size = new_size;
        map->_num_bins = new_num_bins;
        map->_empty_index = new_empty_index;
    }

    // set pointer
    int bin = hash % map->_num_bins;
    MapNode* map_node = &(map->_data[bin]);
    if (map_node->pointer == NULL){
        *map_node = (MapNode) {element, hash, NULL};
        
    } else {
        // add element
        MapNode* new_node = &(map->_data[map->_empty_index]);
        *new_node = (MapNode) {element, hash, NULL};
        map->_empty_index++;

        while (map_node->_next != NULL) {
            map_node = map_node->_next;
        }

        map_node->_next = new_node;
    }
}

static inline void* map_get(Map* map, int hash){
    int bin = hash % map->_num_bins;
    MapNode* map_node = &(map->_data[bin]);
    do {
        if (map_node->_hash == hash){
            return map_node->pointer;
        }
        map_node = map_node->_next;
    } while (map_node != NULL);

    return NULL;
}

static inline int map_hash(int key) {
    const unsigned int INT_PHI = 0x9E3779B9;
    unsigned int h = key * INT_PHI;
    int hash = (int) (h ^ (h >> 16));
    return hash;
}

static inline void map_hash_insert(Map* map, int key, void* element) {
    map_insert(map, map_hash(key), element);
}

static inline void* map_hash_get(Map* map, int key){
    return map_get(map, key);
}

#endif //YADF2_MAP_H
