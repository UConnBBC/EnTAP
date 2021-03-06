//
// Created by harta on 8/12/17.
//

#include <boost/archive/binary_iarchive.hpp>
#include <iomanip>
#include <csv.h>
#include "ModEggnog.h"
#include "../ExceptionHandler.h"

std::pair<bool, std::string> ModEggnog::verify_files() {
    std::string                        annotation_base_flag;
    std::string                        annotation_no_flag;
    bool                               verified;

    annotation_base_flag = (boostFS::path(_ontology_dir) / boostFS::path("annotation_results")).string();
    annotation_no_flag   = (boostFS::path(_ontology_dir) / boostFS::path("annotation_results_no_hits")).string();
    _out_hits = annotation_base_flag  +".emapper.annotations";
    _out_no_hits = annotation_no_flag +".emapper.annotations";

    verified = false;
    print_debug("Overwrite was unselected, verifying output files...");
    if (file_exists(_out_hits)) {
        print_debug("File located at: " + _out_hits + " found");
        verified = true;
    } else print_debug("File located at: " + _out_hits + " NOT found");
    if (file_exists(_out_no_hits)) {
        print_debug("File located at: " + _out_no_hits + " found");
        verified = true;
    } else print_debug("File located at: " + _out_no_hits + " NOT found");
    if (verified) {
        print_debug("One or more ontology files were found, skipping ontology execution");
        return std::make_pair(true, "");
    } else {
        print_debug("No ontology files were found, continuing with execution");
        return std::make_pair(false,"");
    }
}


/**
 *
 * @param SEQUENCES
 * @param out - <egg results of sequences that hit databases, results of no hits>
 */
void ModEggnog::parse(std::map<std::string, QuerySequence> &SEQUENCES) {

    print_debug("Beginning to parse eggnog results...");

    typedef std::map<std::string,std::map<std::string, unsigned int>> GO_top_map_t;

    std::stringstream                        ss;
    std::string                              out_msg;
    std::string                              out_no_hits_nucl;
    std::string                              out_no_hits_prot;
    std::string                              out_hit_nucl;
    std::string                              out_hit_prot;
    std::string                              path;
    std::string                              fig_txt_bar_go_overall;
    std::string                              fig_png_bar_go_overall;
    std::string                              fig_txt_bar_ortho;
    std::string                              fig_png_bar_ortho;
    std::string                              tax_scope_readable;
    std::map<std::string, struct_go_term>    GO_DATABASE;
    std::map<std::string, int>               eggnog_map;
    unsigned int                             count_total_go_hits=0;
    unsigned int                             count_total_go_terms=0;
    unsigned int                             count_go_bio=0;
    unsigned int                             count_go_cell=0;
    unsigned int                             count_go_mole=0;
    unsigned int                             count_no_go=0;
    unsigned int                             count_no_kegg=0;
    unsigned int                             count_TOTAL_hits=0;         // All ortho matches
    unsigned int                             count_total_kegg_terms=0;
    unsigned int                             count_total_kegg_hits=0;
    unsigned int                             count_no_hits=0;            // Unannotated OGs
    unsigned int                             count_tax_scope=0;
    unsigned int                             ct = 0;
    float                                    percent;
    DatabaseHelper                           EGGNOG_DATABASE;
    std::map<std::string, unsigned int>      tax_scope_ct_map;
    GO_top_map_t                             go_combined_map;     // Just for convenience
    go_struct                                go_parsed;
    GraphingStruct                           graphingStruct;

    ss<<std::fixed<<std::setprecision(2);
    boostFS::remove_all(_processed_path);
    boostFS::create_directories(_processed_path);
    boostFS::create_directories(_figure_path);
    try {
        GO_DATABASE = read_go_map();
    } catch (ExceptionHandler const &e) {throw e;}

    if (!EGGNOG_DATABASE.open(_eggnog_db_path))
        throw ExceptionHandler("Unable to open GO database",ENTAP_ERR::E_PARSE_EGGNOG);

    for (int i=0; i<2;i++) {
        i == 0 ? path=_out_hits : path=_out_no_hits;
        print_debug("Eggnog file located at " + path + " being filtered");
        if (!file_exists(path)) {
            print_debug("File not found, skipping...");continue;
        }
        path = eggnog_format(path);
        std::string qseqid, seed_ortho, seed_e, seed_score, predicted_gene, go_terms, kegg, tax_scope, ogs,
                best_og, cog_cat, eggnog_annot;
        io::CSVReader<EGGNOG_COL_NUM, io::trim_chars<' '>, io::no_quote_escape<'\t'>> in(path);
        while (in.read_row(qseqid, seed_ortho, seed_e, seed_score, predicted_gene, go_terms, kegg, tax_scope, ogs,
                           best_og, cog_cat, eggnog_annot)) {
            query_map_struct::iterator it = SEQUENCES.find(qseqid);
            if (it != SEQUENCES.end()) {
                count_TOTAL_hits++;
                it->second.set_eggnog_results(seed_ortho,seed_e,seed_score,predicted_gene,go_terms,
                                              kegg,tax_scope,ogs, EGGNOG_DATABASE);
                go_parsed = parse_go_list(go_terms,GO_DATABASE,',');
                it->second.set_go_parsed(go_parsed, ENTAP_EXECUTE::EGGNOG_INT_FLAG);
                it->second.set_is_family_assigned(true);
                eggnog_map[qseqid] = 1;
                if (!go_parsed.empty()) {
                    count_total_go_hits++;
                    it->second.set_is_one_go(true);
                    for (auto &pair : go_parsed) {
                        for (std::string &term : pair.second) {
                            count_total_go_terms++;
                            if (pair.first == GO_MOLECULAR_FLAG) {
                                count_go_mole++;
                            } else if (pair.first == GO_CELLULAR_FLAG) {
                                count_go_cell++;
                            } else if (pair.first == GO_BIOLOGICAL_FLAG) {
                                count_go_bio++;
                            }
                            if (go_combined_map[pair.first].count(term)) {
                                go_combined_map[pair.first][term]++;
                            } else go_combined_map[pair.first][term] = 1;
                            if (go_combined_map[GO_OVERALL_FLAG].count(term)) {
                                go_combined_map[GO_OVERALL_FLAG][term]++;
                            } else go_combined_map[GO_OVERALL_FLAG][term]=1;
                        }
                    }
                } else {
                    count_no_go++;
                }

                // Compile KEGG information
                if (!kegg.empty()) {
                    count_total_kegg_hits++;
                    ct = (unsigned int) std::count(kegg.begin(), kegg.end(), ',');
                    count_total_kegg_terms += ct + 1;
                    it->second.set_is_one_kegg(true);
                } else {
                    count_no_kegg++;
                }

                // Compile Taxonomic Orthogroup stats
                tax_scope_readable = it->second.get_tax_scope();
                if (!tax_scope_readable.empty()) {
                    count_tax_scope++;
                    if (tax_scope_ct_map.count(tax_scope_readable)) {
                        tax_scope_ct_map[tax_scope_readable]++;
                    } else tax_scope_ct_map[tax_scope_readable] = 1;

                }
            }
        }
        boostFS::remove(path);
    }

    EGGNOG_DATABASE.close();
    out_no_hits_nucl = (boostFS::path(_processed_path) / boostFS::path(OUT_UNANNOTATED_NUCL)).string();
    out_no_hits_prot = (boostFS::path(_processed_path) / boostFS::path(OUT_UNANNOTATED_PROT)).string();
    out_hit_nucl     = (boostFS::path(_processed_path) / boostFS::path(OUT_ANNOTATED_NUCL)).string();
    out_hit_prot     = (boostFS::path(_processed_path) / boostFS::path(OUT_ANNOTATED_PROT)).string();
    std::ofstream file_no_hits_nucl(out_no_hits_nucl, std::ios::out | std::ios::app);
    std::ofstream file_no_hits_prot(out_no_hits_prot, std::ios::out | std::ios::app);
    std::ofstream file_hits_nucl(out_hit_nucl, std::ios::out | std::ios::app);
    std::ofstream file_hits_prot(out_hit_prot, std::ios::out | std::ios::app);

    print_debug("Success! Computing overall statistics...");
    for (auto &pair : SEQUENCES) {
        if (eggnog_map.find(pair.first) == eggnog_map.end()) {
            // Unannotated sequence
            if (!pair.second.get_sequence_n().empty()) file_no_hits_nucl<<pair.second.get_sequence_n()<<std::endl;
            file_no_hits_prot << pair.second.get_sequence_p() << std::endl;
            count_no_hits++;
        } else {
            // Annotated sequence
            if (!pair.second.get_sequence_n().empty()) file_hits_nucl<<pair.second.get_sequence_n()<<std::endl;
            file_hits_prot << pair.second.get_sequence_p() << std::endl;
        }
    }

    file_hits_nucl.close();
    file_hits_prot.close();
    file_no_hits_nucl.close();
    file_no_hits_prot.close();

    ss << ENTAP_STATS::SOFTWARE_BREAK + "Ontology - Eggnog\n" +
          ENTAP_STATS::SOFTWARE_BREAK            <<
       "Statistics for overall Eggnog results: " <<
       "\nTotal sequences with family assignment: " << count_TOTAL_hits <<
       "\nTotal sequences without family assignment: " <<count_no_hits;

    // -------- Top Ten Taxonomic Scopes ------- //
    if (!tax_scope_ct_map.empty()) {
        std::string fig_txt_tax_bar = (boostFS::path(_figure_path) / GRAPH_EGG_TAX_BAR_TXT).string();
        std::string fig_png_tax_bar = (boostFS::path(_figure_path) / GRAPH_EGG_TAX_BAR_PNG).string();
        std::ofstream file_tax_bar(fig_txt_tax_bar, std::ios::out | std::ios::app);
        file_tax_bar << "Taxonomic Scope\tCount" << std::endl;

        ss << "\nTop 10 Taxonomic Scopes Assigned:";
        ct = 1;
        std::vector<count_pair> tax_scope_vect(tax_scope_ct_map.begin(),tax_scope_ct_map.end());
        std::sort(tax_scope_vect.begin(),tax_scope_vect.end(),compair());
        for (count_pair &pair : tax_scope_vect) {
            if (ct > 10) break;
            percent = ((float)pair.second / count_tax_scope) * 100;
            ss <<
               "\n\t" << ct << ")" << pair.first << ": " << pair.second <<
               "(" << percent << "%)";
            file_tax_bar << pair.first << '\t' << std::to_string(pair.second) << std::endl;
            ct++;
        }
        file_tax_bar.close();
        graphingStruct.fig_out_path = fig_png_tax_bar;
        graphingStruct.text_file_path = fig_txt_tax_bar;
        graphingStruct.graph_title = GRAPH_EGG_TAX_BAR_TITLE;
        graphingStruct.software_flag = GRAPH_ONTOLOGY_FLAG;
        graphingStruct.graph_type = GRAPH_TOP_BAR_FLAG;
        pGraphingManager->graph(graphingStruct);
    }
    // --------------------------------------- //

    ss<<
      "\nTotal sequences with at least one GO term: " << count_total_go_hits <<
      "\nTotal sequences without GO terms: " << count_no_go <<
      "\nTotal GO terms assigned: " << count_total_go_terms;

    if (count_total_go_hits > 0) {
        for (int lvl : _go_levels) {
            for (auto &pair : go_combined_map) {
                if (pair.first.empty()) continue;
                // Count maps (biological/molecular/cellular/overall)
                std::string fig_txt_go_bar = (boostFS::path(_figure_path) / pair.first).string() + std::to_string(lvl)+GRAPH_GO_END_TXT;
                std::string fig_png_go_bar = (boostFS::path(_figure_path) / pair.first).string() + std::to_string(lvl)+GRAPH_GO_END_PNG;
                std::ofstream file_go_bar(fig_txt_go_bar, std::ios::out | std::ios::app);
                std::vector<count_pair> go_vect(pair.second.begin(),pair.second.end());
                std::sort(go_vect.begin(),go_vect.end(),compair());
                file_go_bar << "Gene Ontology Term\tCount" << std::endl;

                // get total count for each level...change, didn't feel like making another
                unsigned int lvl_ct = 0;   // Use for percentages, total terms for each lvl
                ct = 0;                    // Use for unique
                for (count_pair &pair2 : go_vect) {
                    if (pair2.first.find("(L=" + std::to_string(lvl))!=std::string::npos || lvl == 0) {
                        ct++;
                        lvl_ct += pair2.second;
                    }
                }
                ss << "\nTotal " << pair.first <<" terms (lvl=" << lvl << "): " << lvl_ct;
                ss << "\nTotal unique " << pair.first <<" terms (lvl=" << lvl << "): " << ct;
                ss << "\nTop 10 " << pair.first << " terms assigned (lvl=" << lvl << "): ";

                ct = 1;
                for (count_pair &pair2 : go_vect) {
                    if (ct > 10) break;
                    if (pair2.first.find("(L=" + std::to_string(lvl))!=std::string::npos || lvl == 0) {
                        ct++;
                        percent = ((float)pair2.second / lvl_ct) * 100;
                        ss <<
                           "\n\t" << ct << ")" << pair2.first << ": " << pair2.second <<
                           "(" << percent << "%)";
                        file_go_bar << pair2.first << '\t' << std::to_string(pair2.second) << std::endl;
                    }
                }
                file_go_bar.close();
                graphingStruct.fig_out_path = fig_png_go_bar;
                graphingStruct.text_file_path = fig_txt_go_bar;
                if (pair.first == GO_BIOLOGICAL_FLAG) graphingStruct.graph_title = GRAPH_GO_BAR_BIO_TITLE + "_Level:_"+std::to_string(lvl);
                if (pair.first == GO_CELLULAR_FLAG) graphingStruct.graph_title = GRAPH_GO_BAR_CELL_TITLE+ "_Level:_"+std::to_string(lvl);
                if (pair.first == GO_MOLECULAR_FLAG) graphingStruct.graph_title = GRAPH_GO_BAR_MOLE_TITLE+ "_Level:_"+std::to_string(lvl);
                if (pair.first == GO_OVERALL_FLAG) graphingStruct.graph_title = GRAPH_GO_BAR_ALL_TITLE+ "_Level:_"+std::to_string(lvl);
                // Other params can stay the same
                pGraphingManager->graph(graphingStruct);
            }
        }

    }
    ss<<
      "\nTotal sequences with at least one pathway (KEGG) assignment: " << count_total_kegg_hits<<
      "\nTotal sequences without pathways (KEGG): " << count_no_kegg<<
      "\nTotal pathways (KEGG) assigned: " << count_total_kegg_terms;
    out_msg = ss.str();
    print_statistics(out_msg);
    GO_DATABASE.clear();
    print_debug("Success!");
}

void ModEggnog::execute(std::map<std::string, QuerySequence> &) {
    print_debug("Running eggnog...");

    std::string                        annotation_base_flag;
    std::string                        annotation_no_flag;
    std::string                        annotation_std;
    std::string                        eggnog_command;
    std::string                        hit_out;
    std::string                        no_hit_out;


    annotation_base_flag = (boostFS::path(_ontology_dir) / boostFS::path("annotation_results")).string();
    annotation_no_flag   = (boostFS::path(_ontology_dir) / boostFS::path("annotation_results_no_hits")).string();
    annotation_std       = (boostFS::path(_ontology_dir) / boostFS::path("annotation_std")).string();
    eggnog_command       = "python " + _exe_path + " ";

    std::unordered_map<std::string,std::string> eggnog_command_map = {
            {"-i",_inpath},
            {"--output",annotation_base_flag},
            {"--cpu",std::to_string(_threads)},
            {"-m", "diamond"}
    };
    if (file_exists(_inpath)) {
        for (auto &pair : eggnog_command_map)eggnog_command += pair.first + " " + pair.second + " ";
        print_debug("\nExecuting eggnog mapper against protein sequences that hit databases...\n"
                    + eggnog_command);
        if (execute_cmd(eggnog_command, annotation_std) !=0) {
            throw ExceptionHandler("Error executing eggnog mapper", ENTAP_ERR::E_RUN_ANNOTATION);
        }
        print_debug("Success! Results written to: " + annotation_base_flag);
    } else {
        throw ExceptionHandler("No input file found at: " + _inpath,
                               ENTAP_ERR::E_RUN_EGGNOG);
    }
    if (file_exists(_in_no_hits)) {
        std::ifstream inFile(_in_no_hits);
        long line_num = std::count(std::istreambuf_iterator<char>(inFile),
                                   std::istreambuf_iterator<char>(), '\n');
        inFile.close();
        if (line_num >1) {
            eggnog_command_map["-i"] = _in_no_hits;
            eggnog_command_map["--output"] = annotation_no_flag;
            eggnog_command = "python " + _exe_path + " ";
            for (auto &pair : eggnog_command_map) eggnog_command += pair.first + " " + pair.second + " ";
            print_debug("\nExecuting eggnog mapper against protein sequences that did not hit databases...\n"
                        + eggnog_command);
            if (execute_cmd(eggnog_command, annotation_std) !=0) {
                throw ExceptionHandler("Error executing eggnog mapper", ENTAP_ERR::E_RUN_ANNOTATION);
            }
        }
    }
    print_debug("Success!");
}

void ModEggnog::set_data(std::vector<short> & lvls, std::string & eggnog_databse, int t) {

    _go_levels = lvls;
    _eggnog_db_path = eggnog_databse;
    _threads = t;
}


// TODO remove
std::string ModEggnog::eggnog_format(std::string file) {

    std::string out_path;
    std::string line;

    out_path = file + "_alt";
    boostFS::remove(out_path);
    std::ifstream in_file(file);
    std::ofstream out_file(out_path);
    while (getline(in_file,line)) {
        if (line.at(0) == '#' || line.empty()) continue;
        out_file << line << std::endl;
    }
    in_file.close();
    out_file.close();
    return out_path;
}
