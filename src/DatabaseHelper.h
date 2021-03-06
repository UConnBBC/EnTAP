//
// Created by harta on 5/25/17.
//

#ifndef ENTAP_DATABASEHELPER_H
#define ENTAP_DATABASEHELPER_H

#include <iostream>
#include <vector>
#include "sqlite3.h"


class DatabaseHelper {

typedef std::vector<std::vector<std::string>> query_struct;

public:
    DatabaseHelper();
    ~DatabaseHelper();
    bool open(std::string file);
    void close();
    query_struct query(char* query);

private:
    sqlite3 *_database;
};



#endif //ENTAP_DATABASEHELPER_H
