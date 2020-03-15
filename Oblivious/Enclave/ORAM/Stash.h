#ifndef MEASUREMENT_STASH_H
#define MEASUREMENT_STASH_H

#include "../../../Common/CommonUtil.h"

struct __attribute__((packed)) Block {
    uint32_t id;
    uint8_t block[ORAM_DATA_SIZE];
};

struct __attribute__((packed)) Node {
    Block b;
    Node *prev;
    Node *next;
};

class Stash {
private:
    Node *top;
    Node *bottom;
    uint32_t stash_size;        // the upper bound of the stash size
    uint32_t current_size;
public:
    Stash(uint32_t size) {
        stash_size = size;
        current_size = 0;

        top = nullptr;
        bottom = nullptr;
    }

    ~Stash() {
        clear();
    }

    void clear() {
        Node *iter = get_start();
        while(iter != nullptr) {
            Node *cur = iter;
            iter = iter->prev;
            free(cur);
        }

        top = nullptr;
        bottom = nullptr;
    }

    Node* get_start() const {
        return top;
    }

    void insert(Block &new_block) {
        // scan the stash to rewrite the existing blocks
        Node *iter = get_start();
        while(iter != nullptr) {
            if(new_block.id == iter->b.id) {
                memcpy(iter->b.block, new_block.block, ORAM_DATA_SIZE);
                return;
            }
            iter = iter->prev;
        }

        Node *new_node = (Node*) malloc(sizeof(Node));
        // reset the pointer
        new_node->prev = nullptr;
        new_node->next = nullptr;
        // attach at the end of the stash
        if(current_size == stash_size) {
            return;     // stash is full, ignore this block
        } else {
            memcpy(&new_node->b, &new_block, sizeof(Block));
            if(bottom == nullptr) {     // empty stash
                bottom = new_node;
                top = new_node;
            } else {
                bottom->prev = new_node;
                new_node->next = bottom;
                bottom = new_node;
            }
            current_size++;
        }
    }

    void erase(Node *del_block) {
        if(current_size == 1) {     // only one node
            top = nullptr;
            bottom = nullptr;
        } else if(del_block->next == nullptr) { // del the head node
            top = del_block->prev;
            top->next = nullptr;
        } else if(del_block->prev == nullptr) { // del the bottom node
            bottom = del_block->next;
            bottom->prev = nullptr;
        } else {                                // del the node in the middle
            del_block->prev->next = del_block->next;
            del_block->next->prev = del_block->prev;
        }
        // release the memory
        free(del_block);
        current_size--;
    }

    Node* search(uint32_t bid) {
        Node *iter = top;
        while(iter != nullptr) {
            if(iter->b.id == bid) {
                return iter;
            }
            iter = iter->prev;
        }
        // not found return empty
        return iter;
    }
};

#endif //MEASUREMENT_STASH_H
