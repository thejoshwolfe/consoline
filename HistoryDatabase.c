#include "HistoryDatabase.h"

#include "RbTree.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    int access_count;
    char is_case_senssitive;
    RbTree * tree;
} InternalHistoryDatabase;

typedef struct {
    char * text;
    int hit_count;
    int last_hit_time;
} WordData;

static int strcmp_with_casting(void * left, void * right)
{
    return strcmp((const char *)left, (const char *)right);
}

HistoryDatabase * HistoryDatabase_create(char is_case_senssitive)
{
    HistoryDatabase * database = (HistoryDatabase *)malloc(sizeof(HistoryDatabase));
    InternalHistoryDatabase * secret_data = (InternalHistoryDatabase *)malloc(sizeof(InternalHistoryDatabase));
    secret_data->access_count = 0;
    secret_data->is_case_senssitive = is_case_senssitive;
    secret_data->tree = RbTree_create(strcmp_with_casting);
    database->_secret_data = secret_data;
    return database;
}

static char delete_visitor(RbTree_Node * node, void * data)
{
    free(node->key);
    WordData * word_data = (WordData *)node->value;
    free(word_data->text);
    free(word_data);
    return 1;
}
void HistoryDatabase_delete(HistoryDatabase * database)
{
    InternalHistoryDatabase * secret_data = (InternalHistoryDatabase *)database->_secret_data;
    RbTree_delete(secret_data->tree, delete_visitor);
    free(secret_data);
    free(database);
}

static char * key_for_word(char is_case_senssitive, char * word)
{
    if (is_case_senssitive)
        return word;
    // to lower case
    char * key = strdup(word);
    int i;
    for (i = 0; key[i] != '\0'; i++)
        key[i] = tolower(key[i]);
    return key;
}

void HistoryDatabase_add(HistoryDatabase * database, char * word)
{
    InternalHistoryDatabase * secret_data = (InternalHistoryDatabase *)database->_secret_data;
    char * key = key_for_word(secret_data->is_case_senssitive, word);
    WordData * word_data = (WordData *)RbTree_get(secret_data->tree, key);
    if (word_data == NULL) {
        word_data = (WordData *)malloc(sizeof(WordData));
        word_data->text = word;
        word_data->hit_count = 0;
        RbTree_put(secret_data->tree, strdup(key), word_data);
    }
    word_data->hit_count++;
    word_data->last_hit_time = secret_data->access_count++;
    if (key != word)
        free(key);
}

typedef struct {
    char * prefix;
    int prefix_len;
    int matches_cap;
    char ** matches;
    int matches_len;
} MatchCollect;

static char match_visitor(RbTree_Node * node, void * data)
{
    MatchCollect * match_collector = (MatchCollect *)data;
    if (strncmp(match_collector->prefix, (char *)node->key, match_collector->prefix_len) != 0)
        return 0;
    // it's a match
    WordData * word_data = (WordData *)node->value;
    match_collector->matches[match_collector->matches_len++] = strdup(word_data->text);
    if (match_collector->matches_len >= match_collector->matches_cap) {
        // expand capacity
        match_collector->matches_cap *= 2;
        match_collector->matches = (char **)realloc(match_collector->matches, match_collector->matches_cap * sizeof(char *));
    }
    return 1;
}

char ** HistoryDatabase_prefix_matches(HistoryDatabase * database, char * prefix)
{
    InternalHistoryDatabase * secret_data = (InternalHistoryDatabase *)database->_secret_data;
    char * key_prefix = key_for_word(secret_data->is_case_senssitive, prefix);
    MatchCollect match_collector;
    match_collector.prefix = key_prefix;
    match_collector.prefix_len = strlen(key_prefix);
    match_collector.matches_cap = 0x10;
    match_collector.matches = (char **)malloc(match_collector.matches_cap * sizeof(char *));
    match_collector.matches_len = 0;
    RbTree_traverse_starting_at(secret_data->tree, key_prefix, match_visitor, &match_collector);
    if (key_prefix != prefix)
        free(key_prefix);
    match_collector.matches[match_collector.matches_len] = NULL;
    return match_collector.matches;
}

