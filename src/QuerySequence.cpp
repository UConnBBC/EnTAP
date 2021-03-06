//
// Created by harta on 3/29/17.
//

#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include "QuerySequence.h"
#include "EntapGlobals.h"
#include "EggnogLevels.h"

// best hit selection
bool QuerySequence::operator>(const QuerySequence &querySequence) {
    if (this->is_better_hit) {
        // For hits of the same database "better hit"
        double eval1 = this->_e_val, eval2 = querySequence._e_val;
        if (eval1 == 0) eval1 = 1E-180;
        if (eval2 == 0) eval2 = 1E-180;
        if (fabs(log10(eval1) - log10(eval2)) < E_VAL_DIF) {
            double coverage_dif = fabs(this->_coverage - querySequence._coverage);
            if (coverage_dif > COV_DIF) {
                return this->_coverage > querySequence._coverage;
            }
            if (this->_sim_search_results.contaminant && !querySequence._sim_search_results.contaminant) return false;
            if (!this->_sim_search_results.contaminant && querySequence._sim_search_results.contaminant) return true;
            if (this->_tax_score == querySequence._tax_score)
                return this->_e_val<querySequence._e_val;
            return this->_tax_score > querySequence._tax_score;
        } else {
            return eval1 < eval2;
        }
    }else {
        // For overall best hits between databases "best hit"
        double coverage_dif = fabs(this->_coverage - querySequence._coverage);
        if (coverage_dif > COV_DIF) {
            return this->_coverage > querySequence._coverage;
        }
        if (this->_sim_search_results.contaminant && !querySequence._sim_search_results.contaminant) return false;
        if (!this->_sim_search_results.contaminant && querySequence._sim_search_results.contaminant) return true;
        return this->_tax_score > querySequence._tax_score;
    }
}

void operator+(const QuerySequence &querySequence) {


}

// TODO switch to set_sim_search
void QuerySequence::set_sim_search_results(std::string database,std::string qseqid,std::string sseqid,
                             std::string pident,std::string length, std::string mismatch, std::string gap, std::string qstart,
                             std::string qend , std::string sstart, std::string send, std::string title, std::string bit,
                             double evalue,  double cover) {
    _sim_search_results.database_path = database;
    _sim_search_results.qseqid = qseqid;
    _sim_search_results.sseqid = sseqid;
    _sim_search_results.pident = pident;
    _sim_search_results.length = length;
    _sim_search_results.mismatch = mismatch;
    _sim_search_results.gapopen = gap;
    _sim_search_results.qstart = qstart;
    _sim_search_results.qend = qend;
    _sim_search_results.sstart = sstart;
    _sim_search_results.send = send;
    _sim_search_results.stitle = title;
    _sim_search_results.bit_score = bit;
    std::ostringstream ostringstream;
    ostringstream<<evalue;
    _sim_search_results.e_val = ostringstream.str();
    _sim_search_results.coverage = std::to_string(cover);

    _e_val = evalue;
    _coverage = cover;
}


unsigned long QuerySequence::getSeq_length() const {
    return _seq_length;
}

QuerySequence::QuerySequence() {
    init_sequence();

}

void QuerySequence::setSequence( std::string &seq) {
    is_protein = true;
    _seq_length = calc_seq_length(seq,true);
    if (!seq.empty() && seq[seq.length()-1] == '\n') {
        seq.pop_back();
    }
    _sequence_p = seq;
}

const std::string &QuerySequence::get_sequence_p() const {
    return _sequence_p;
}

void QuerySequence::set_sequence_p(const std::string &_sequence_p) {
    QuerySequence::_sequence_p = _sequence_p;
}

const std::string &QuerySequence::get_sequence_n() const {
    return _sequence_n;
}

void QuerySequence::set_sequence_n(const std::string &_sequence_n) {
    QuerySequence::_sequence_n = _sequence_n;
}

bool QuerySequence::isIs_protein() const {
    return is_protein;
}

QuerySequence::QuerySequence(bool is_protein, std::string seq){
    init_sequence();
    this->_is_database_hit = false;
    this->is_protein = is_protein;
    _seq_length = calc_seq_length(seq,is_protein);
    if (!seq.empty() && seq[seq.length()-1] == '\n') {
        seq.pop_back();
    }
    is_protein ? _sequence_p = seq : _sequence_n = seq;
}

unsigned long QuerySequence::calc_seq_length(std::string &seq,bool protein) {
    std::string sub = seq.substr(seq.find("\n")+1);
    long line_chars = std::count(sub.begin(),sub.end(),'\n');
    unsigned long seq_len = sub.length() - line_chars;
    if (protein) seq_len *= 3;
    return seq_len;
}


void QuerySequence::setQseqid(const std::string &qseqid) {
    _sim_search_results.qseqid = qseqid;
}

void QuerySequence::setSpecies(const std::string &species) {
    _sim_search_results.species = species;
}

bool QuerySequence::isContaminant() const {
    return _sim_search_results.contaminant;
}

void QuerySequence::setContaminant(bool contaminant) {
    QuerySequence::_sim_search_results.contaminant = contaminant;
    contaminant ? _sim_search_results.yes_no_contam = "Yes" : _sim_search_results.yes_no_contam  = "No";
}

std::ostream& operator<<(std::ostream &ostream, const QuerySequence &query) {
//    return ostream<<query._qseqid<<'\t' <<query._sseqid<<'\t'  <<query._pident<<'\t'<<
//                    query._length<<'\t' <<query._mismatch<<'\t'<<
//                    query._gapopen<<'\t'<<query._qstart<<'\t'  <<query._qend<<'\t'<<
//                    query._sstart<<'\t' <<query._send<<'\t'    <<query._e_val<<'\t'<< query._coverage<<"\t"<<
//                    query._stitle<<'\t' <<query._species<<'\t' <<query._database_path<<'\t'<<
//                    query._frame;
}

const std::string &QuerySequence::getFrame() const {
    return _frame;
}

void QuerySequence::setFrame(const std::string &frame) {
    QuerySequence::_frame = frame;
}

void QuerySequence::setSeq_length(unsigned long seq_length) {
    QuerySequence::_seq_length = seq_length;
}

// This is reserved for individual sim search file filtering
// Best hit for each database
void QuerySequence::setIs_better_hit(bool is_better_hit) {
    QuerySequence::is_better_hit = is_better_hit;
}

void QuerySequence::set_is_informative(bool _is_informative) {
    QuerySequence::_is_informative = _is_informative;
}

const std::string &QuerySequence::get_species() const {
    return _sim_search_results.species;
}

const std::string &QuerySequence::get_tax_scope() const {
    return _eggnog_results.tax_scope_readable;
}

const std::string &QuerySequence::get_contam_type() const {
    return _sim_search_results.contam_type;
}

bool QuerySequence::is_informative() const {
    return _is_informative;
}

void QuerySequence::set_contam_type(const std::string &_contam_type) {
    QuerySequence::_sim_search_results.contam_type = _contam_type;
}

void QuerySequence::set_is_database_hit(bool _is_database_hit) {
    QuerySequence::_is_database_hit = _is_database_hit;
}

void QuerySequence::set_eggnog_results(std::string seed_o, std::string seed_o_eval,
                                       std::string seed_score, std::string predicted,
                                       std::string go_terms, std::string kegg,
                                       std::string annotation_tax, std::string ogs,
                                       DatabaseHelper &database) {
    this->_eggnog_results.seed_ortholog = seed_o;
    this->_eggnog_results.seed_evalue = seed_o_eval;
    this->_eggnog_results.seed_score = seed_score;
    this->_eggnog_results.predicted_gene = predicted;
    this->_eggnog_results.ogs = ogs;

    // Lookup/Assign Tax Scope
    if (!annotation_tax.empty()) {
        unsigned short p = (unsigned short) (annotation_tax.find("NOG"));
        if (p != std::string::npos) {
            this->_eggnog_results.tax_scope = annotation_tax.substr(0,p+3);
            this->_eggnog_results.tax_scope_readable =
                    EGGNOG_LEVELS[this->_eggnog_results.tax_scope];
        } else {
            this->_eggnog_results.tax_scope = annotation_tax;
            this->_eggnog_results.tax_scope_readable = "";
        }
    } else {
        this->_eggnog_results.tax_scope = "";
        this->_eggnog_results.tax_scope_readable = "";
    }

    // Push each GO term to a new position in vector array
    std::string temp;
    if (!go_terms.empty()) {
        std::istringstream ss(go_terms);
        while (std::getline(ss,temp,',')) {
            this->_eggnog_results.raw_go.push_back(temp);
        }
    }

    // Push each KEGG term to a new position in vector array
    if (!kegg.empty()) {
        std::istringstream keggs(kegg);
        while (std::getline(keggs,temp,',')) {
            this->_eggnog_results.raw_kegg.push_back(temp);
        }
    }

    // Find OG query was assigned to
    if (!ogs.empty()) {
        std::istringstream ss(ogs);
        std::unordered_map<std::string,std::string> og_map; // Not fully used right now
        while (std::getline(ss,temp,',')) {
            unsigned short p = (unsigned short) temp.find("@");
            og_map[temp.substr(p+1)] = temp.substr(0,p);
        }
        _eggnog_results.og_key = "";
        if (og_map.find(_eggnog_results.tax_scope) != og_map.end()) {
            _eggnog_results.og_key = og_map[_eggnog_results.tax_scope];
        }
    }

    // Lookup description, KEGG, protein domain from SQL database
    _eggnog_results.sql_kegg = "";
    _eggnog_results.description = "";
    _eggnog_results.protein_domains = "";
    if (!_eggnog_results.og_key.empty()) {
        std::vector<std::vector<std::string>>results;
        std::string sql_kegg;
        std::string sql_desc;
        std::string sql_protein;

        char *query = sqlite3_mprintf(
                "SELECT description, KEGG_freq, SMART_freq FROM og WHERE og=%Q",
                _eggnog_results.og_key.c_str());
        try {
            results = database.query(query);
            sql_desc = results[0][0];
            sql_kegg = results[0][1];
            sql_protein = results[0][2];
            if (!sql_desc.empty() && sql_desc.find("[]") != 0) _eggnog_results.description = sql_desc;
            if (!sql_kegg.empty() && sql_kegg.find("[]") != 0) _eggnog_results.sql_kegg = sql_kegg;
            if (!sql_protein.empty() && sql_protein.find("[]") != 0){
                _eggnog_results.protein_domains = sql_protein;
            }
        } catch (std::exception &e) {
            // Do not fatal error
            print_debug(e.what());
        }
    }
}


void QuerySequence::set_go_parsed(const QuerySequence::go_struct &_go_parsed, short i) {
    switch (i) {
        case ENTAP_EXECUTE::EGGNOG_INT_FLAG:
            _eggnog_results.parsed_go = _go_parsed;
            break;
        case ENTAP_EXECUTE::INTERPRO_INT_FLAG:
            break;
        default:
            break;
    }
}

void QuerySequence::init_sequence() {
    _seq_length = 0;
    _e_val = 0;
    _coverage = 0;

    _sim_search_results.length      = "";
    _sim_search_results.mismatch    = "";
    _sim_search_results.gapopen     = "";
    _sim_search_results.qstart      = "";
    _sim_search_results.qend        = "";
    _sim_search_results.sstart      = "";
    _sim_search_results.send        = "";
    _sim_search_results.pident      = "";
    _sim_search_results.bit_score   = "";
    _sim_search_results.e_val       = "";
    _sim_search_results.coverage    = "";
    _sim_search_results.database_path="";
    _sim_search_results.qseqid = "";
    _sim_search_results.sseqid = "";
    _sim_search_results.stitle = "";
    _sim_search_results.species = "";
    _sim_search_results.yes_no_contam = "";


    _eggnog_results.seed_ortholog="";
    _eggnog_results.seed_evalue="";
    _eggnog_results.seed_score="";
    _eggnog_results.predicted_gene="";
    _eggnog_results.tax_scope="";
    _eggnog_results.tax_scope_readable="";
    _eggnog_results.ogs="";
    _eggnog_results.og_key="";
    _eggnog_results.sql_kegg="";
    _eggnog_results.description="";
    _eggnog_results.protein_domains="";

    _frame = "";
    _sequence_p = "";
    _sequence_n = "";
    _is_family_assigned = false;
    _is_one_go = false;
    _is_one_kegg = false;
    _is_database_hit = false;
    _is_expression_kept = false;
}

void QuerySequence::set_ontology_results(std::map<std::string, std::string> map) {
//    this->_ontology_results = map;
}


void QuerySequence::set_lineage(const std::string &_lineage) {
    QuerySequence::_sim_search_results.lineage = _lineage;
}

void QuerySequence::set_tax_score(std::string input_lineage) {
    float tax_score = 0;
    std::string lineage = _sim_search_results.lineage;
    std::remove_if(lineage.begin(),lineage.end(), ::isspace);
    std::remove_if(input_lineage.begin(),input_lineage.end(), ::isspace);

    std::string temp;
    size_t p = 0;std::string del = ";";
    while ((p = lineage.find(";"))!=std::string::npos) {
        temp = lineage.substr(0,p);
        if (input_lineage.find(temp)!=std::string::npos) tax_score++;
        lineage.erase(0,p+del.length());
    }
    if (tax_score == 0) {
        if(_is_informative) tax_score += INFORM_ADD;
    } else {
        tax_score *= INFORM_FACTOR;
    }
    _tax_score = tax_score;
}

const std::string &QuerySequence::get_sequence() const {
    if (_sequence_n.empty()) return _sequence_p;
    return _sequence_n;
}

const QuerySequence::SimSearchResults &QuerySequence::get_sim_struct() const {
    return _sim_search_results;
}

void QuerySequence::set_sim_struct(const SimSearchResults &sim) {
    _sim_search_results = sim;
}

void QuerySequence::setIs_protein(bool is_protein) {
    QuerySequence::is_protein = is_protein;
}

bool QuerySequence::is_is_database_hit() const {
    return _is_database_hit;
}

bool QuerySequence::is_is_family_assigned() const {
    return _is_family_assigned;
}

void QuerySequence::set_is_family_assigned(bool _is_family_assigned) {
    QuerySequence::_is_family_assigned = _is_family_assigned;
}

bool QuerySequence::is_is_one_go() const {
    return _is_one_go;
}

void QuerySequence::set_is_one_go(bool _is_one_go) {
    QuerySequence::_is_one_go = _is_one_go;
}

bool QuerySequence::is_is_one_kegg() const {
    return _is_one_kegg;
}

void QuerySequence::set_is_one_kegg(bool _is_one_kegg) {
    QuerySequence::_is_one_kegg = _is_one_kegg;
}

bool QuerySequence::is_is_expression_kept() const {
    return _is_expression_kept;
}

void QuerySequence::set_is_expression_kept(bool _is_expression_kept) {
    QuerySequence::_is_expression_kept = _is_expression_kept;
};

std::string QuerySequence::print_tsv(const std::vector<const std::string*>& headers) {
    std::stringstream ss;

    // Fix, shouldn't be initialized here
    init_header();

    for (const std::string *header : headers) {
        ss << *OUTPUT_MAP[header] << "\t";
    }
    return ss.str();
}

std::string QuerySequence::print_tsv(short software, std::vector<const std::string*>& headers,
                                     short lvl) {
    init_header();
    std::stringstream stream;
    go_struct go_terms;

    switch (software) {
        case ENTAP_EXECUTE::EGGNOG_INT_FLAG:
//            stream << *this << '\t';
            go_terms = _eggnog_results.parsed_go;
            break;
        case ENTAP_EXECUTE::INTERPRO_INT_FLAG:
//            stream << *this <<'\t';
//            for (const std::string &val : headers) {
//                stream << _ontology_results[val] << '\t'; TODO interpro udpate
//            }
            // Set go terms
            break;
        default:
            break;
    }

    for (const std::string *header : headers) {
        if (*header == ENTAP_EXECUTE::HEADER_EGG_GO_BIO) {
            if (go_terms.empty()) {
                stream <<'\t';continue;
            }
            for (std::string val : go_terms[ENTAP_EXECUTE::GO_BIOLOGICAL_FLAG]) {
                if (val.find("(L=" + std::to_string(lvl))!=std::string::npos || lvl == 0) {
                    stream<<val<<",";
                }
            }
            stream<<'\t';
        } else if (*header == ENTAP_EXECUTE::HEADER_EGG_GO_CELL) {
            if (go_terms.empty()) {
                stream <<'\t';continue;
            }
            for (std::string val : go_terms[ENTAP_EXECUTE::GO_CELLULAR_FLAG])  {
                if (val.find("(L=" + std::to_string(lvl))!=std::string::npos || lvl == 0) {
                    stream<<val<<",";
                }
            }
            stream<<'\t';
        } else if (*header == ENTAP_EXECUTE::HEADER_EGG_GO_MOLE) {
            if (go_terms.empty()) {
                stream <<'\t';continue;
            }
            for (std::string val : go_terms[ENTAP_EXECUTE::GO_MOLECULAR_FLAG]) {
                if (val.find("(L=" + std::to_string(lvl))!=std::string::npos || lvl == 0) {
                    stream<<val<<",";
                }
            }
            stream<<'\t';
        } else stream << *OUTPUT_MAP[header] << "\t";
    }
    return stream.str();
}

void QuerySequence::set_fpkm(float _fpkm) {
    QuerySequence::_fpkm = _fpkm;
}


void QuerySequence::init_header() {
    OUTPUT_MAP ={
            {&ENTAP_EXECUTE::HEADER_QUERY     , &_sim_search_results.qseqid},
            {&ENTAP_EXECUTE::HEADER_SUBJECT   , &_sim_search_results.sseqid},
            {&ENTAP_EXECUTE::HEADER_PERCENT   , &_sim_search_results.pident},
            {&ENTAP_EXECUTE::HEADER_ALIGN_LEN , &_sim_search_results.length},
            {&ENTAP_EXECUTE::HEADER_MISMATCH  , &_sim_search_results.mismatch},
            {&ENTAP_EXECUTE::HEADER_GAP_OPEN  , &_sim_search_results.gapopen},
            {&ENTAP_EXECUTE::HEADER_QUERY_E   , &_sim_search_results.qend},
            {&ENTAP_EXECUTE::HEADER_QUERY_S   , &_sim_search_results.qstart},
            {&ENTAP_EXECUTE::HEADER_SUBJ_S    , &_sim_search_results.sstart},
            {&ENTAP_EXECUTE::HEADER_SUBJ_E    , &_sim_search_results.send},
            {&ENTAP_EXECUTE::HEADER_E_VAL     , &_sim_search_results.e_val},
            {&ENTAP_EXECUTE::HEADER_COVERAGE  , &_sim_search_results.coverage},
            {&ENTAP_EXECUTE::HEADER_TITLE     , &_sim_search_results.stitle},
            {&ENTAP_EXECUTE::HEADER_SPECIES   , &_sim_search_results.species},
            {&ENTAP_EXECUTE::HEADER_DATABASE  , &_sim_search_results.database_path},
            {&ENTAP_EXECUTE::HEADER_FRAME     , &_frame},
            {&ENTAP_EXECUTE::HEADER_CONTAM    , &_sim_search_results.yes_no_contam},
            {&ENTAP_EXECUTE::HEADER_SEED_ORTH , &_eggnog_results.seed_ortholog},
            {&ENTAP_EXECUTE::HEADER_SEED_EVAL , &_eggnog_results.seed_evalue},
            {&ENTAP_EXECUTE::HEADER_SEED_SCORE, &_eggnog_results.seed_score},
            {&ENTAP_EXECUTE::HEADER_PRED_GENE , &_eggnog_results.predicted_gene},
            {&ENTAP_EXECUTE::HEADER_TAX_SCOPE , &_eggnog_results.tax_scope_readable},
            {&ENTAP_EXECUTE::HEADER_EGG_OGS   , &_eggnog_results.ogs},
            {&ENTAP_EXECUTE::HEADER_EGG_DESC  , &_eggnog_results.description},
            {&ENTAP_EXECUTE::HEADER_EGG_KEGG  , &_eggnog_results.sql_kegg} ,
            {&ENTAP_EXECUTE::HEADER_EGG_PROTEIN,&_eggnog_results.protein_domains}
    };
}
