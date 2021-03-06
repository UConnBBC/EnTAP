//
// Created by harta on 8/12/17.
//

#include <boost/serialization/map.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include "../EntapGlobals.h"
#include "AbstractOntology.h"
#include "../ExceptionHandler.h"


std::map<std::string,std::vector<std::string>> AbstractOntology::parse_go_list
        (std::string list, std::map<std::string,struct_go_term> &GO_DATABASE,char delim) {

    std::map<std::string,std::vector<std::string>> output;
    std::string temp;
    std::vector<std::vector<std::string>>results;

    if (list.empty()) return output;
    std::istringstream ss(list);
    while (std::getline(ss,temp,delim)) {
        struct_go_term term_info = GO_DATABASE[temp];
        output[term_info.category].push_back(temp + "-" + term_info.term +
                                             "(L=" + term_info.level + ")");
    }
    return output;
}



std::map<std::string,struct_go_term> AbstractOntology::read_go_map () {
    std::map<std::string,struct_go_term> new_map;
    std::string go_db_path = _entap_exe + ENTAP_CONFIG::GO_DB_PATH;
    try {
        {
            std::ifstream ifs(go_db_path);
            boost::archive::binary_iarchive ia(ifs);
            ia >> new_map;
        }
    } catch (std::exception &exception) {
        throw ExceptionHandler(exception.what(), ENTAP_ERR::E_INIT_GO_SETUP);
    }
    return new_map;
};
