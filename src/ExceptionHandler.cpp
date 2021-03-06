#include <iostream>
#include <ctime>
#include <fstream>
#include <exception>
#include "ExceptionHandler.h"
#include "EntapGlobals.h"


ExceptionHandler::ExceptionHandler(const std::string& msg, int err) {
    this->err_code = err;
    this->message = msg;
}

void ExceptionHandler::print_msg() {
    time_t rawtime;
    time(&rawtime);
    std::string date_time = ctime(&rawtime);
    std::ofstream log_file(DEBUG_FILE_PATH, std::ios_base::out | std::ios_base::app );
    std::string added_msg;

    switch (err_code) {
        case ENTAP_ERR::E_INPUT_PARSE:
            added_msg = "Error in parsing input data, please consult -h for more "
                    "information.";
            break;
        case ENTAP_ERR::E_INIT_TAX_DOWN:
            added_msg = "Error in downloading the taxonomic database";
        default:
            added_msg = "Error code not recognized.";
    // TODO uniprot db not found : 'Please rerun config and download the database"
    }
    log_file << date_time.substr(0, date_time.size() - 2)
                 + ": \n" + added_msg << std::endl ;
    log_file <<what()<<std::endl;
    std::cerr << added_msg << std::endl << what() <<std::endl;
}

const char* ExceptionHandler::what() {
    return message.c_str();
}

int ExceptionHandler::getErr_code() const {
    return err_code;
}




