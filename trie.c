#include <stdlib.h>
#include <stdio.h>
#include <Block.h>

struct _Trie {
    char state;
    char * value;
    struct _Trie * sibling;
    struct _Trie * children;
};
typedef struct _Trie Trie;

Trie * Trie_new() {
    Trie * trie = (Trie *) malloc(sizeof(Trie));
    trie->state = '\0';
    trie->value = NULL;
    trie->sibling = NULL;
    trie->children = NULL;
    return trie;
}

void Trie_free(Trie * trie) {
    if(trie->sibling)
        Trie_free(trie->sibling);
    if(trie->children)
        Trie_free(trie->children);
    free(trie);
}

Trie * Trie_node(char state) {
    Trie * trie = Trie_new();
    trie->state = state;
    return trie;
}

void Trie_add(Trie * trie, char * data, int length, char * value) {
    // look for the child
    Trie * child = trie->children;
    while(child) {
        if(child->state == *data)
            break;
        child = child->sibling;
    }

    // if the child doesn't exist add it
    if(!child) {
        child = Trie_node(*data);
        child->sibling = trie->children;
        trie->children = child;
    }

    if(length == 1) {
        child->value = value;
    } else {
        // oh, tail recursion... how I wish you existed in C.
        Trie_add(child, data + 1, length - 1, value);
    }
}

char * Trie_get(Trie * trie, char * data, int length) {
    Trie * child = trie;

    // look for the child
    while(length > 0) {
        child = child->children;
        while(child) {
            if(child->state == *data)
                break;
            child = child->sibling;
        }
        if(!child)
            return NULL;

        length--;
        data++;
    }
    
    return child->value;
}

/*
int main() {
    Trie * trie = Trie_new();
    Trie_add(trie, "foo", 3, "foo");
    Trie_add(trie, "bar", 3, "bar");
    Trie_add(trie, "food", 4, "food");

    printf("foo: %s\n", Trie_get(trie, "foo", 3));
    printf("food: %s\n", Trie_get(trie, "food", 4));
    printf("bar: %s\n", Trie_get(trie, "bar", 3));
    printf("argh: %s\n", Trie_get(trie, "argh", 4));
}
*/
