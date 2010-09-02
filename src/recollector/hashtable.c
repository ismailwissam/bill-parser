#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "common.h"

#define HASH_TABLE_SIZE     1000

//hash table object
static NList *hashtab[HASH_TABLE_SIZE];

//internal fuction
static unsigned int hash(const char *s);
static void nlist_free(NList *nlist);
static void node_free(NList *node);

NList *hashtable_insert(const char *key, const char *value)
{
    NList *np = NULL;
    unsigned int hashval;

    if((np = hashtable_search(key)) == NULL)
    {
        np = (NList *)malloc(sizeof(NList));
        if(np == NULL)
            return NULL;
        np->key = strdup(key);
        np->prev = NULL;
        np->next = NULL;

        hashval = hash(key);

        np->next= hashtab[hashval];
        if(hashtab[hashval] != NULL)
            hashtab[hashval]->prev = np;
        hashtab[hashval] = np;
    }
    else
    {
        if(np->value != NULL)
        {
            free(np->value);
        }
    }

    np->value = strdup(value);

    return np;
}

void hashtable_remove(const char *key)
{
    NList *np;
    unsigned int hashval;

    if((np = hashtable_search(key)) != NULL) 
    {
        if(np->prev != NULL && np->next != NULL)
        {
            np->prev->next = np->next;
            np->next->prev = np->prev;
        }
        else if(np->prev != NULL)
        {
            np->prev->next = NULL;
        }
        else if(np->next != NULL)
        {
            np->next->prev = NULL;
            hashval = hash(key);
            hashtab[hashval] = np->next;
        }
        else
        {
            hashval = hash(key);
            hashtab[hashval] = NULL;
        }

        node_free(np);
    }
}

NList *hashtable_search(const char *key)
{
    NList *np;

    for(np = hashtab[hash(key)];
        np != NULL;
        np = np->next)
            if(strcmp(key, np->key) == 0)
                    return np;
    return NULL;
}

const char * hashtable_value(const char *key)
{   
    NList *np;

    for(np = hashtab[hash(key)];
        np != NULL;
        np = np->next)
    {
        err_log("hashtable: key: %s\n", key);
        err_log("hashtable: np->key: %s\n", np->key);
        err_log("hashtable: np->value: %s\n", np->value);

            if(strcmp(key, np->key) == 0)
                    return np->value;
    }

    return NULL;
}

void hashtable_init(void)
{
    int i;
    for(i = 0; i < HASH_TABLE_SIZE; ++i)
    {
        hashtab[i] = NULL;
    }
}

void hashtable_destroy(void)
{
    int i;
    for(i = 0; i < HASH_TABLE_SIZE; ++i)
    {
        if(hashtab[i] != NULL)
            nlist_free(hashtab[i]);
    }
}

static unsigned int hash(const char *s)
{
    unsigned int hashval;

    for(hashval = 0; *s != '\0';s++)
            hashval = *s + 31 * hashval;
    return hashval % HASH_TABLE_SIZE;
}

static void nlist_free(NList *nlist)
{
    NList *np = NULL;
    while(nlist != NULL)
    {
        np = nlist;
        nlist = np->next;
        np->next = NULL;
        node_free(np);
    }
}

static void node_free(NList *node)
{
    if(node != NULL)
    {
        if(node->key != NULL)
            free(node->key);
        if(node->value != NULL)
            free(node->value);
        free(node);
    }
}

