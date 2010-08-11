struct _Trie {
    char state;
    char * value;
    struct _Trie * sibling;
    struct _Trie * children;
};
typedef struct _Trie Trie;

Trie * Trie_new();
void Trie_free(Trie *);
Trie * Trie_node(char);
void Trie_add(Trie *, char *, int, char *);
char * Trie_get(Trie *, char *, int);
