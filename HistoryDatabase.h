#ifndef _HISTORY_DATABASE_H_
#define _HISTORY_DATABASE_H_

typedef struct {
    void * _secret_data;
} HistoryDatabase;

HistoryDatabase * HistoryDatabase_create(char is_case_senssitive);
void HistoryDatabase_delete(HistoryDatabase * database);
void HistoryDatabase_add(HistoryDatabase * database, char * word);
char ** HistoryDatabase_prefix_matches(HistoryDatabase * database, char * prefix);

#endif
