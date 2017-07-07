//
// Created by harta on 5/10/17.
//

#ifndef ENTAP_SIMILARITYSEARCH_H
#define ENTAP_SIMILARITYSEARCH_H
#include <iostream>
#include <list>
#include <unordered_map>
#include <map>
#include <boost/program_options/variables_map.hpp>
#include "QuerySequence.h"


class SimilaritySearch {
public:
    std::vector<std::string> execute(std::string, bool);
    SimilaritySearch(std::vector<std::string>&, std::string, int, std::string, std::string,
                     std::string,boost::program_options::variables_map &);
    std::pair<std::string,std::string> parse_files(std::string,std::map<std::string, QuerySequence>&);
private:
    const std::string _NCBI_REGEX                                = "\\[([^]]+)\\](?!.+\\[.+\\])";
    const std::string _UNIPROT_REGEX                             = "OS=(.+?)\\s\\S\\S=";
    const std::string SIM_SEARCH_DATABASE_BEST_TSV               = "_best_hits.tsv";
    const std::string SIM_SEARCH_DATABASE_BEST_TSV_NO_CONTAM     = "_best_hits_no_contam.tsv";
    const std::string SIM_SEARCH_DATABASE_BEST_FA_NUCL           = "_best_hits.fnn";
    const std::string SIM_SEARCH_DATABASE_BEST_FA_PROT           = "_best_hits.faa";
    const std::string SIM_SEARCH_DATABASE_BEST_FA_NUCL_NO_CONTAM = "_best_hits_no_contam.fnn";
    const std::string SIM_SEARCH_DATABASE_BEST_FA_PROT_NO_CONTAM = "_best_hits_no_contam.faa";

    const std::string SIM_SEARCH_DATABASE_CONTAM_TSV             = "_best_hits_contam.tsv";
    const std::string SIM_SEARCH_DATABASE_CONTAM_FA_NUCL         = "_best_hits_contam.fnn";
    const std::string SIM_SEARCH_DATABASE_CONTAM_FA_PROT         = "_best_hits_contam.faa";
    const std::string SIM_SEARCH_DATABASE_NO_HITS_NUCL           = "_no_hits.fnn";
    const std::string SIM_SEARCH_DATABASE_NO_HITS_PROT           = "_no_hits.faa";
    const std::string SIM_SEARCH_DATABASE_UNSELECTED             = "_unselected.tsv";
    const std::string SIM_SEARCH_PARSE_PROCESSED                 = "similarity_search/processed";
    const std::string SIM_SEARCH_RESULTS_DIR                     = "similarity_search/results";
    const std::string SIM_SEARCH_COMPILED_PATH                   = "similarity_search/results/_overall";

    std::vector<std::string>         _database_paths;
    std::vector<std::string>         _sim_search_paths;
    std::string                      _diamond_exe;
    std::string                      _outpath;
    std::string                      _input_path;
    std::string                      _entap_exe;
    std::string                      _input_lineage;
    std::string                      _input_species;
    std::string                      _blast_type;
    int                              _threads;
    bool                             _overwrite;
    bool                             _blastp;
    double                           _e_val;
    double                           _coverage;
    short                            _software_flag;
    std::vector<std::string>         _contaminants;

    std::vector<std::string> diamond();
    void diamond_blast(std::string, std::string, std::string,std::string&,int&, std::string&);
    std::vector<std::string> verify_diamond_files(std::string&, std::string);
    std::pair<std::string,std::string> diamond_parse(std::vector<std::string>&,
                                                     std::map<std::string, QuerySequence>&);
    std::unordered_map<std::string, std::string> read_tax_map();
    std::pair<bool,std::string>  is_contaminant(std::string, std::unordered_map<std::string,
            std::string> &,std::vector<std::string>&);
    std::string get_species(std::string &);
    bool is_informative(std::string);
    std::pair<std::string,std::string> process_best_diamond_hit(std::list<std::map<std::string,
            QuerySequence>>&,std::map<std::string, QuerySequence>&);
    void print_header(std::string);
    std::string get_lineage(std::string, std::unordered_map<std::string, std::string>&);
    int calculate_score(std::string,bool);
    std::pair<std::string,std::string> calculate_best_stats (std::map<std::string, QuerySequence>&,
                                 std::map<std::string, QuerySequence>&,
                                 std::stringstream &, std::string&,bool);
};


#endif //ENTAP_SIMILARITYSEARCH_H
