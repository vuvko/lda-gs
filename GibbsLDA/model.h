/*
 * Copyright (C) 2007 by
 * 
 *         Xuan-Hieu Phan
 *        hieuxuan@ecei.tohoku.ac.jp or pxhieu@gmail.com
 *         Graduate School of Information Sciences
 *         Tohoku University
 *
 * GibbsLDA++ is a free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * GibbsLDA++ is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GibbsLDA++; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

/* 
 * References:
 * + The Java code of Gregor Heinrich (gregor@arbylon.net)
 *   http://www.arbylon.net/projects/LdaGibbsSampler.java
 * + "Parameter estimation for text analysis" by Gregor Heinrich
 *   http://www.arbylon.net/publications/text-est.pdf
 */

#ifndef        _MODEL_H
#define        _MODEL_H

#include "constants.h"
#include "dataset.h"
#include "matrix.h"
#include "munkres.h"

using namespace std;

const double eps = 0.000001;

typedef void (*PCALLBACK)(int percent, void *parent, int experiment);
typedef void (*DCALLBACK)(double value, int iter, void *parent, int experiment);

// LDA model
class model {
    // fixed options
    string wordmapfile;                // file that contains word map [string -> integer id]
    string trainlogfile;        // training log file
    string tassign_suffix;        // suffix for topic assignment file
    string theta_suffix;        // suffix for theta file
    string phi_suffix;                // suffix for phi file
    string others_suffix;        // suffix for file containing other parameters
    string twords_suffix;        // suffix for file containing words-per-topics

    string dir;                        // model directory
    string dfile;                // data file    
    string model_name;                // model name
    int model_status;                // model status:
                                // MODEL_STATUS_UNKNOWN: unknown status
                                // MODEL_STATUS_EST: estimating from scratch
                                // MODEL_STATUS_ESTC: continue to estimate the model from a previous one
                                // MODEL_STATUS_INF: do inference

    dataset * ptrndata;        // pointer to training dataset object
    dataset * pnewdata; // pointer to new dataset object

    mapid2word id2word; // word map [int => string]

    int experiment;
    bool use_soft;
    // for hungarian algorithm
    bool use_hungarian;
    double gamma;
    Matrix<double> distance;
    int K_real;
    double **phi_real;
    double **theta_real;
    double **inv_theta_real;

    // for robast model
    int phi_perc;
    int theta_perc;
    int r_iter;
    
    // --- model parameters and variables ---    
    int M; // dataset size (i.e., number of docs)
    int V; // vocabulary size
    int K; // number of topics
    double alpha, beta; // LDA hyperparameters 
    int niters; // number of Gibbs sampling iterations
    int liter; // the iteration at which the model was saved
    int savestep; // saving period
    int twords; // print out top words per each topic
    int withrawstrs;
    int piter; // calculation perplexity period

    double * p; // temp variable for sampling
    int ** z; // topic assignments for words, size M x doc.size()
    int ** nw; // cwt[i][j]: number of instances of word/term i assigned to topic j, size V x K
    int ** nd; // na[i][j]: number of words in document i assigned to topic j, size M x K
    int * nwsum; // nwsum[j]: total number of words assigned to topic j, size K
    int * ndsum; // nasum[i]: total number of words in document i, size M
    double ** theta; // theta: document-topic distributions, size M x K
    double ** phi; // phi: topic-word distributions, size K x V
    double ** inv_theta; // p(d|t) = p(t|d) * p(d) / sum_d (p(t|d) * p(d))
    
    // for inference only
    int inf_liter;
    int newM;
    int newV;
    int ** newz;
    int ** newnw;
    int ** newnd;
    int * newnwsum;
    int * newndsum;
    double ** newtheta;
    double ** newphi;
    // --------------------------------------
    PCALLBACK progress_callback;
    DCALLBACK perplexity_callback;
    DCALLBACK distance_callback;
    void *parent;

    int sampling(int m, int n);
    void sparse_theta();
    void sparse_phi();
    void compute_distance();

    int inf_sampling(int m, int n);

public:
    model();
    ~model();

    void set_callbacks(PCALLBACK progress, DCALLBACK perplexity, DCALLBACK distance, void *parent_);
    void set_topics_number(int n_topics);
    void set_document_number(int n_docs);
    void set_words_number(int n_words);
    void set_alpha(double new_alpha);
    void set_beta(double new_beta);
    void set_hyperparameters(double new_alpha, double new_beta);
    void set_model_name(const string &new_name);
    void set_model_status(int i);
    void set_experiment_number(int n_experiment);
    void set_iterations(int n_iters);
    void set_last_iteration(int new_liter);
    void set_save_iteration(int n_save_iters);
    void set_twords(int new_twords);
    void set_perplexity_iter(int new_piter);
    void set_gamma(double new_gamma);
    void use_half_smoothing(bool use_half_ = true);
    void use_hungarian_algorithm(bool use_hungarian_ = true);
    void set_real_topics_number(int n_topics_real);
    void set_robast_iter(int r_iter_);
    void set_phi_percentage(int phi_perc_);
    void set_theta_percentage(int theta_perc_);
    void set_data_file(const string &data_file);
    void set_work_dir(const string &work_dir);
    void set_withrawstrs(int withrawstrs_);

    int get_topics_number(void) const;
    int get_document_number(void) const;
    int get_words_number(void) const;
    double get_alpha(void) const;
    double get_beta(void) const;
    string get_model_name(void) const;
    string get_work_dir(void) const;
    string get_data_file(void) const;
    string get_others_suffix(void) const;
    int get_doc_length(int document) const;
    int get_doc_real_length(int document) const;
    int get_word_counts(int document, int word) const;
    int get_word(int document, int word_number) const;
    double get_phi(int word, int topic) const;
    double get_theta(int topic, int document) const;

    // parse command line to get options
    int parse_args(int argc, char ** argv);
    
    // initialize the model
    int init(int argc, char ** argv);
    
    // load LDA model to continue estimating or to do inference
    int load_model(string model_name);

    // load distributions
    int load_phi(string filename);
    int load_theta(string filename);
    
    // save LDA model to files
    // model_name.tassign: topic assignments for words in docs
    // model_name.theta: document-topic distributions
    // model_name.phi: topic-word distributions
    // model_name.others: containing other parameters of the model (alpha, beta, M, V, K)
    // do in utils?
    int save_model(string model_name);
    int save_model_tassign(string filename);
    int save_model_theta(string filename);
    int save_model_phi(string filename);
    int save_model_others(string filename);
    int save_model_twords(string filename);
    
    // saving inference outputs
    // do in utils?
    int save_inf_model(string model_name);
    int save_inf_model_tassign(string filename);
    int save_inf_model_newtheta(string filename);
    int save_inf_model_newphi(string filename);
    int save_inf_model_others(string filename);
    int save_inf_model_twords(string filename);
    
    // init for estimation
    int init_est();
    int init_estc();
        
    // estimate LDA model using Gibbs sampling
    void estimate();
    void compute_theta();
    void compute_phi();
    
    // init for inference
    int init_inf();
    // inference for new (unseen) data based on the estimated LDA model
    void inference();
    void compute_newtheta();
    void compute_newphi();

    // generate document collection
    void generate_collection(string filename, int doc_len);
};

#endif

