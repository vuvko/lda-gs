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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "constants.h"
#include "strtokenizer.h"
#include "utils.h"
#include "dataset.h"
#include "model.h"

using namespace std;

model::~model() {
    if (p) {
        delete [] p;
    }

    if (ptrndata) {
        delete ptrndata;
    }
    
    if (pnewdata) {
        delete pnewdata;
    }

    if (z) {
        for (int m = 0; m < M; m++) {
            if (z[m]) {
                delete [] z[m];
            }
        }
        delete [] z;
    }
    
    if (nw) {
        for (int w = 0; w < V; w++) {
            if (nw[w]) {
                delete [] nw[w];
            }
        }
        delete [] nw;
    }

    if (nd) {
        for (int m = 0; m < M; m++) {
            if (nd[m]) {
                delete [] nd[m];
            }
        }
        delete [] nd;
    }
    
    if (nwsum) {
        delete [] nwsum;
    }
    
    if (ndsum) {
        delete [] ndsum;
    }
    
    if (theta) {
        for (int m = 0; m < M; m++) {
            if (theta[m]) {
                delete [] theta[m];
            }
        }
        delete [] theta;
    }
    
    if (theta_real) {
        for (int m = 0; m < M; m++) {
            if (theta_real[m]) {
                delete [] theta_real[m];
            }
        }
        delete [] theta_real;
    }

    if (inv_theta) {
        for (int k = 0; k < K; ++k) {
            if (inv_theta[k]) {
                delete [] inv_theta[k];
            }
        }
        delete [] inv_theta;
    }

    if (inv_theta_real) {
        for (int k = 0; k < K; ++k) {
            if (inv_theta_real[k]) {
                delete [] inv_theta_real[k];
            }
        }
        delete [] inv_theta_real;
    }

    if (phi) {
        for (int k = 0; k < K; k++) {
            if (phi[k]) {
                delete [] phi[k];
            }
        }
        delete [] phi;
    }

    if (phi_real) {
        for (int k = 0; k < K; k++) {
            if (phi_real[k]) {
                delete [] phi_real[k];
            }
        }
        delete [] phi_real;
    }

    // only for inference
    if (newz) {
        for (int m = 0; m < newM; m++) {
            if (newz[m]) {
                delete [] newz[m];
            }
        }
        delete [] newz;
    }
    
    if (newnw) {
        for (int w = 0; w < newV; w++) {
            if (newnw[w]) {
                delete [] newnw[w];
            }
        }
        delete [] newnw;
    }

    if (newnd) {
        for (int m = 0; m < newM; m++) {
            if (newnd[m]) {
                delete [] newnd[m];
            }
        }
        delete [] newnd;
    }
    
    if (newnwsum) {
        delete [] newnwsum;
    }
    
    if (newndsum) {
        delete [] newndsum;
    }
    
    if (newtheta) {
        for (int m = 0; m < newM; m++) {
            if (newtheta[m]) {
                delete [] newtheta[m];
            }
        }
        delete [] newtheta;
    }
    
    if (newphi) {
        for (int k = 0; k < K; k++) {
            if (newphi[k]) {
                delete [] newphi[k];
            }
        }
        delete [] newphi;
    }
}

model::model()
{
    wordmapfile = "wordmap.txt";
    trainlogfile = "trainlog.txt";
    tassign_suffix = ".tassign";
    theta_suffix = ".theta";
    phi_suffix = ".phi";
    others_suffix = ".others";
    twords_suffix = ".twords";

    dir = "./";
    dfile = "trndocs.dat";
    model_name = "model-final";
    model_status = MODEL_STATUS_UNKNOWN;

    ptrndata = NULL;
    pnewdata = NULL;
    use_hungarian = false;

    p = NULL;
    z = NULL;
    nw = NULL;
    nd = NULL;
    nwsum = NULL;
    ndsum = NULL;
    theta = NULL;
    theta_real = NULL;
    inv_theta = NULL;
    inv_theta_real = NULL;
    phi = NULL;
    phi_real = NULL;

    newz = NULL;
    newnw = NULL;
    newnd = NULL;
    newnwsum = NULL;
    newndsum = NULL;
    newtheta = NULL;
    newphi = NULL;
    newM = 0;
    newV = 0;
    K = 100;
    M = 0;
    V = 0;
    alpha = 50.0 / K;
    beta = 0.1;
    niters = 30;
    liter = 0;
    savestep = 200;
    twords = 0;
    withrawstrs = 0;
    piter = 0;

    phi_perc = 0;
    theta_perc = 0;
    r_iter = 0;
}

int model::parse_args(int argc, char ** argv) {
    return utils::parse_args(argc, argv, this);
}

int model::init(int argc, char ** argv) {
    // call parse_args
    if (parse_args(argc, argv)) {
        return 1;
    }
    
    if (model_status == MODEL_STATUS_EST) {
        // estimating the model from scratch
        if (use_hungarian) {
            return 0;
        }
        if (init_est()) {
            return 1;
        }
        
    } else if (model_status == MODEL_STATUS_ESTC) {
        // estimating the model from a previously estimated one
        if (init_estc()) {
            return 1;
        }
        
    } else if (model_status == MODEL_STATUS_INF) {
        // do inference
        if (init_inf()) {
            return 1;
        }
    }
    
    return 0;
}

int model::load_model(string model_name) {
    int i, j;
    
    string filename = dir + model_name + tassign_suffix;
    FILE * fin = fopen(filename.c_str(), "r");
    char buff[BUFF_SIZE_LONG];
    if (!fin) {
        printf("Cannot open file %s to load model!\n", filename.c_str());
        return 1;
    }
    
    string line;

    // allocate memory for z and ptrndata
    z = new int*[M];
    ptrndata = new dataset(M);
    ptrndata->V = V;

    for (i = 0; i < M; i++) {
        char * pointer = fgets(buff, BUFF_SIZE_LONG, fin);
        if (!pointer) {
            printf("Invalid word-topic assignment file, check the number of docs!\n");
            return 1;
        }
        
        line = buff;
        strtokenizer strtok(line, " \t\r\n");
        int length = strtok.count_tokens();
        
        vector<pair<int, int> > words;
        vector<int> topics;
        for (j = 0; j < length; j++) {
            /*
            string token = strtok.token(j);

            strtokenizer tok(token, ":");
            if (tok.count_tokens() != 2) {
                printf("Invalid word-topic assignment line!\n");
                return 1;
            }
            */
            words.push_back(pair<int, int>(atoi(strtok.token(j).first.c_str()), 1)); // TODO mb fix it?
            topics.push_back(strtok.token(j).second);
        }
        
        // allocate and add new document to the corpus
        document * pdoc = new document(words);
        ptrndata->add_doc(pdoc, i);
        
        // assign values for z
        z[i] = new int[topics.size()];
        for (j = 0; j < topics.size(); j++) {
            z[i][j] = topics[j];
        }
    }
    
    fclose(fin);
    
    return 0;
}

int model::load_phi(string filename)
{
    FILE *fin = fopen(filename.c_str(), "r");
    if (!fin) {
        printf("Cannot open file %s to load phi!\n", filename.c_str());
        return 1;
    }
    phi_real = new double *[K_real];
    for (int k = 0; k < K_real; ++k) {
        phi_real[k] = new double [V];
        for (int w = 0; w < V; ++w) {
            fscanf(fin, "%lf", &phi_real[k][w]);
        }
    }
    fclose(fin);
    return 0;
}

int model::load_theta(string filename)
{
    FILE *fin = fopen(filename.c_str(), "r");
    if (!fin) {
        printf("Cannot open file %s to load theta!\n", filename.c_str());
        return 1;
    }
    theta_real = new double *[M];
    for (int m = 0; m < M; ++m) {
        theta_real[m] = new double[K_real];
        for (int k = 0; k < K_real; ++k) {
            fscanf(fin, "%lf", &theta_real[m][k]);
        }
    }
    fclose(fin);
    inv_theta_real = new double *[K_real];
    for (int k = 0; k < K_real; ++k) {
        inv_theta_real[k] = new double[M];
        double theta_sum = 0;
        for (int m = 0; m < M; ++m) {
            theta_sum += theta_real[m][k];
        }
        for (int m = 0; m < M; ++m) {
            inv_theta_real[k][m] = theta_real[m][k] / theta_sum;
        }
    }
    return 0;
}

int model::save_model(string model_name) {
    if (save_model_tassign(dir + model_name + tassign_suffix)) {
        return 1;
    }
    
    if (save_model_others(dir + model_name + others_suffix)) {
        return 1;
    }
    
    if (save_model_theta(dir + model_name + theta_suffix)) {
        return 1;
    }
    
    if (save_model_phi(dir + model_name + phi_suffix)) {
        return 1;
    }
    
    if (twords > 0) {
        if (save_model_twords(dir + model_name + twords_suffix)) {
            return 1;
        }
    }
    
    return 0;
}

int model::save_model_tassign(string filename) {
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        printf("Cannot open file %s to save!\n", filename.c_str());
        return 1;
    }

    // wirte docs with topic assignments for words
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < ptrndata->docs[i]->length; j++) {
            if (z[i][j] < eps) {
                z[i][j] = -1;
            }
            fprintf(fout, "%d:%d ", ptrndata->docs[i]->words[j], z[i][j]);
        }
        fprintf(fout, "\n");
    }

    fclose(fout);
    
    return 0;
}

int model::save_model_theta(string filename) {
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        printf("Cannot open file %s to save!\n", filename.c_str());
        return 1;
    }

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < K; j++) {
            fprintf(fout, "%.15f ", theta[i][j]);
        }
        fprintf(fout, "\n");
    }
    
    fclose(fout);
    
    return 0;
}

int model::save_model_phi(string filename) {
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        printf("Cannot open file %s to save!\n", filename.c_str());
        return 1;
    }
    
    for (int i = 0; i < K; i++) {
        for (int j = 0; j < V; j++) {
            fprintf(fout, "%.15f ", phi[i][j]);
        }
        fprintf(fout, "\n");
    }
    
    fclose(fout);
    
    return 0;
}

int model::save_model_others(string filename) {
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        printf("Cannot open file %s to save!\n", filename.c_str());
        return 1;
    }

    fprintf(fout, "alpha=%f\n", alpha);
    fprintf(fout, "beta=%f\n", beta);
    fprintf(fout, "ntopics=%d\n", K);
    fprintf(fout, "ndocs=%d\n", M);
    fprintf(fout, "nwords=%d\n", V);
    fprintf(fout, "liter=%d\n", liter);
    
    fclose(fout);
    
    return 0;
}

int model::save_model_twords(string filename) {
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        printf("Cannot open file %s to save!\n", filename.c_str());
        return 1;
    }
    
    if (twords > V) {
        twords = V;
    }
    mapid2word::iterator it;
    
    for (int k = 0; k < K; k++) {
        vector<pair<int, double> > words_probs;
        pair<int, double> word_prob;
        for (int w = 0; w < V; w++) {
            word_prob.first = w;
            word_prob.second = phi[k][w];
            words_probs.push_back(word_prob);
        }

        // quick sort to sort word-topic probability
        utils::quicksort(words_probs, 0, words_probs.size() - 1);
        
        fprintf(fout, "Topic %dth:\n", k);
        for (int i = 0; i < twords; i++) {
            it = id2word.find(words_probs[i].first);
            if (it != id2word.end()) {
                fprintf(fout, "\t%s   %f\n", (it->second).c_str(), words_probs[i].second);
            }
        }
    }
    
    fclose(fout);
    
    return 0;
}

int model::save_inf_model(string model_name) {
    if (save_inf_model_tassign(dir + model_name + tassign_suffix)) {
        return 1;
    }
    
    if (save_inf_model_others(dir + model_name + others_suffix)) {
        return 1;
    }
    
    if (save_inf_model_newtheta(dir + model_name + theta_suffix)) {
        return 1;
    }
    
    if (save_inf_model_newphi(dir + model_name + phi_suffix)) {
        return 1;
    }

    if (twords > 0) {
        if (save_inf_model_twords(dir + model_name + twords_suffix)) {
            return 1;
        }
    }
    
    return 0;
}

int model::save_inf_model_tassign(string filename) {
    int i, j;
    
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        printf("Cannot open file %s to save!\n", filename.c_str());
        return 1;
    }

    // wirte docs with topic assignments for words
    for (i = 0; i < pnewdata->M; i++) {
        for (j = 0; j < pnewdata->docs[i]->length; j++) {
            fprintf(fout, "%d:%d ", pnewdata->docs[i]->words[j], newz[i][j]);
        }
        fprintf(fout, "\n");
    }

    fclose(fout);
    
    return 0;
}

int model::save_inf_model_newtheta(string filename) {
    int i, j;

    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        printf("Cannot open file %s to save!\n", filename.c_str());
        return 1;
    }
    
    for (i = 0; i < newM; i++) {
        for (j = 0; j < K; j++) {
            fprintf(fout, "%f ", newtheta[i][j]);
        }
        fprintf(fout, "\n");
    }
    
    fclose(fout);
    
    return 0;
}

int model::save_inf_model_newphi(string filename) {
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        printf("Cannot open file %s to save!\n", filename.c_str());
        return 1;
    }
    
    for (int i = 0; i < K; i++) {
        for (int j = 0; j < newV; j++) {
            fprintf(fout, "%f ", newphi[i][j]);
        }
        fprintf(fout, "\n");
    }
    
    fclose(fout);
    
    return 0;
}

int model::save_inf_model_others(string filename) {
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        printf("Cannot open file %s to save!\n", filename.c_str());
        return 1;
    }

    fprintf(fout, "alpha=%f\n", alpha);
    fprintf(fout, "beta=%f\n", beta);
    fprintf(fout, "ntopics=%d\n", K);
    fprintf(fout, "ndocs=%d\n", newM);
    fprintf(fout, "nwords=%d\n", newV);
    fprintf(fout, "liter=%d\n", inf_liter);
    
    fclose(fout);
    
    return 0;
}

int model::save_inf_model_twords(string filename) {
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        printf("Cannot open file %s to save!\n", filename.c_str());
        return 1;
    }
    
    if (twords > newV) {
        twords = newV;
    }
    mapid2word::iterator it;
    map<int, int>::iterator _it;
    
    for (int k = 0; k < K; k++) {
        vector<pair<int, double> > words_probs;
        pair<int, double> word_prob;
        for (int w = 0; w < newV; w++) {
            word_prob.first = w;
            word_prob.second = newphi[k][w];
            words_probs.push_back(word_prob);
        }

        // quick sort to sort word-topic probability
        utils::quicksort(words_probs, 0, words_probs.size() - 1);
        
        fprintf(fout, "Topic %dth:\n", k);
        for (int i = 0; i < twords; i++) {
            _it = pnewdata->_id2id.find(words_probs[i].first);
            if (_it == pnewdata->_id2id.end()) {
                continue;
            }
            it = id2word.find(_it->second);
            if (it != id2word.end()) {
                fprintf(fout, "\t%s   %f\n", (it->second).c_str(), words_probs[i].second);
            }
        }
    }
    
    fclose(fout);
    
    return 0;
}


int model::init_est() {
    int m, n, w, k;

    p = new double[K];

    if (progress_callback) {
        progress_callback(0, parent, experiment);
    }

    // + read training data
    ptrndata = new dataset;
    if (ptrndata->read_trndata(dir + dfile, dir + wordmapfile)) {
        printf("Fail to read training data!\n");
        return 1;
    }

    // + allocate memory and assign values for variables
    M = ptrndata->M;
    V = ptrndata->V;
    // K: from command line or default value
    // alpha, beta: from command line or default values
    // niters, savestep: from command line or default values

    nw = new int*[V];
    for (w = 0; w < V; w++) {
        nw[w] = new int[K];
        for (k = 0; k < K; k++) {
            nw[w][k] = 0;
        }
    }

    nd = new int*[M];
    for (m = 0; m < M; m++) {
        nd[m] = new int[K];
        for (k = 0; k < K; k++) {
            nd[m][k] = 0;
        }
    }

    nwsum = new int[K];
    for (k = 0; k < K; k++) {
        nwsum[k] = 0;
    }
    
    ndsum = new int[M];
    for (m = 0; m < M; m++) {
        ndsum[m] = 0;
    }

    srandom(time(0)); // initialize for random number generation
    z = new int*[M];
    for (m = 0; m < ptrndata->M; m++) {
        int N = ptrndata->docs[m]->length;
        z[m] = new int[N];
        
        // initialize for z
        for (n = 0; n < N; n++) {
            if (ptrndata->docs[m]->word_counts[n] == 0) {
                z[m][n] = -1;
                continue;
            }
            int topic = (int)(((double)random() / RAND_MAX) * K);
            z[m][n] = topic;

            // number of instances of word i assigned to topic j
            nw[ptrndata->docs[m]->words[n]][topic] += ptrndata->docs[m]->word_counts[n];
            // number of words in document i assigned to topic j
            nd[m][topic] += ptrndata->docs[m]->word_counts[n];
            // total number of words assigned to topic j
            nwsum[topic] += ptrndata->docs[m]->word_counts[n];
            // total number of words in document i
            ndsum[m] += ptrndata->docs[m]->word_counts[n];
        }
    }
    
    theta = new double*[M];
    for (m = 0; m < M; m++) {
        theta[m] = new double[K];
    }

    inv_theta = new double *[K];
    phi = new double*[K];
    for (k = 0; k < K; k++) {
        inv_theta[k] = new double[M];
        phi[k] = new double[V];
    }
    if (K > K_real) {
        int cp = (K + K_real - 1) / K_real;
        distance.resize(K, cp * K_real);
    } else {
        distance.resize(K, K_real);
    }
    
    return 0;
}

int model::init_estc() {
    // estimating the model from a previously estimated one
    int m, n, w, k;

    p = new double[K];

    if (progress_callback) {
        progress_callback(0, parent, experiment);
    }

    // load moel, i.e., read z and ptrndata
    if (load_model(model_name)) {
        printf("Fail to load word-topic assignmetn file of the model!\n");
        return 1;
    }

    nw = new int*[V];
    for (w = 0; w < V; w++) {
        nw[w] = new int[K];
        for (k = 0; k < K; k++) {
            nw[w][k] = 0;
        }
    }

    nd = new int*[M];
    for (m = 0; m < M; m++) {
        nd[m] = new int[K];
        for (k = 0; k < K; k++) {
            nd[m][k] = 0;
        }
    }

    nwsum = new int[K];
    for (k = 0; k < K; k++) {
        nwsum[k] = 0;
    }
    
    ndsum = new int[M];
    for (m = 0; m < M; m++) {
        ndsum[m] = 0;
    }

    for (m = 0; m < ptrndata->M; m++) {
        int N = ptrndata->docs[m]->length;

        // assign values for nw, nd, nwsum, and ndsum
        for (n = 0; n < N; n++) {
            if (ptrndata->docs[m]->word_counts[n] == 0) {
                continue;
            }
            int w = ptrndata->docs[m]->words[n];
            int topic = z[m][n];

            // number of instances of word i assigned to topic j
            nw[w][topic] += 1;
            // number of words in document i assigned to topic j
            nd[m][topic] += 1;
            // total number of words assigned to topic j
            nwsum[topic] += 1;
        }
        // total number of words in document i
        ndsum[m] = ptrndata->docs[m]->real_length;;
    }

    theta = new double*[M];
    for (m = 0; m < M; m++) {
        theta[m] = new double[K];
    }

    phi = new double*[K];
    for (k = 0; k < K; k++) {
        phi[k] = new double[V];
    }

    return 0;
}

void model::estimate() {
    if (twords > 0) {
        // print out top words per topic
        dataset::read_wordmap(dir + wordmapfile, &id2word);
    }

    //printf("Sampling %d iterations!\n", niters);

    int last_iter = liter;
    for (liter = last_iter + 1; liter <= niters + last_iter; liter++) {
        //printf("Iteration %d ...\n", liter);
        
        // for all z_i
        for (int m = 0; m < M; m++) {
            for (int n = 0; n < ptrndata->docs[m]->length; n++) {
                if (ptrndata->docs[m]->word_counts[n] == 0) {
                    continue;
                }
                //for (int k = 0; k < ptrndata->docs[m]->word_counts[n]; ++k) {
                    // (z_i = z[m][n])
                    // sample from p(z_i|z_-i, w)
                    int topic = sampling(m, n);
                    z[m][n] = topic;
                //}

            }
        }
        
        if (r_iter > 0) {
            if (liter % r_iter == 0) {
                sparse_theta();
                sparse_phi();
            }
        }
        if (savestep > 0) {
            if (liter % savestep == 0) {
                // saving the model
                printf("Saving the model at iteration %d ...\n", liter);
                compute_theta();
                compute_phi();
                save_model(utils::generate_model_name(model_name, liter));
            }
        }
        if (piter > 0) {
            if (liter % piter == 0) {
                compute_theta();
                compute_phi();
                if (perplexity_callback) {
                    perplexity_callback(utils::calc_perplexity(this), liter, parent, experiment);
                }
                if (use_hungarian) {
                    compute_distance();
                }
            }
        }
        if (progress_callback) {
            progress_callback((100 * (liter - last_iter)) / niters, parent, experiment);
        }
    }
    
    printf("Gibbs sampling completed!\n");
    printf("Saving the final model!\n");
    compute_theta();
    compute_phi();
    if (perplexity_callback) {
        perplexity_callback(utils::calc_perplexity(this), liter, parent, experiment);
    }
    liter--;
    save_model(utils::generate_model_name(model_name, -1));
}

int model::sampling(int m, int n) {
    // remove z_i from the count variables
    int topic = z[m][n];
    int w = ptrndata->docs[m]->words[n];
    int count = ptrndata->docs[m]->word_counts[n];
    nw[w][topic] -= count;
    nd[m][topic] -= count;
    nwsum[topic] -= count;
    ndsum[m] -= count;

    if (nw[w][topic] < 0) {
        nw[w][topic] = 0;
        nwsum[topic] = 0;
    }
    if (nd[m][topic] < 0) {
        nd[m][topic] = 0;
        ndsum[topic] = 0;
    }

    // do multinomial sampling via cumulative method
    for (int k = 0; k < K; k++) {
        p[k] = (nw[w][k] + beta) / (nwsum[k] + V * beta) *
                (nd[m][k] + alpha) / (ndsum[m] + K * alpha);
    }
    // cumulate multinomial parameters
    for (int k = 1; k < K; k++) {
        p[k] += p[k - 1];
    }

    // scaled sample because of unnormalized p[]
    double u = ((double)random() / RAND_MAX) * p[K - 1];
    
    for (topic = 0; topic < K; topic++) {
        if (p[topic] > u) {
            break;
        }
    }

    // add newly estimated z_i to count variables
    nw[w][topic] += count;
    nd[m][topic] += count;
    nwsum[topic] += count;
    ndsum[m] += count;
    
    return topic;
}

void model::compute_theta() {
    for (int k = 0; k < K; k++) {
        double theta_sum = 0;
        for (int m = 0; m < M; m++) {
            if (use_soft) {
                if (ndsum[m] == 0) {
                    theta[m][k] = 0;
                } else {
                    theta[m][k] = nd[m][k] / double(ndsum[m]);
                }
            } else {
                theta[m][k] = (nd[m][k] + alpha) / (ndsum[m] + K * alpha);
            }
            theta_sum += theta[m][k];
        }
        for (int m = 0; m < M; ++m) {
            if (theta_sum < eps) {
                inv_theta[k][m] = 0;
            } else {
                inv_theta[k][m] = theta[m][k] / theta_sum;
            }
        }
    }
}

void model::compute_phi() {
    for (int k = 0; k < K; k++) {
        for (int w = 0; w < V; w++) {
            if (use_soft) {
                if (nwsum[k] == 0) {
                    phi[k][w] = 0;
                } else {
                    phi[k][w] = nw[w][k] / double(nwsum[k]);
                }
            } else {
                phi[k][w] = (nw[w][k] + beta) / (nwsum[k] + V * beta);
            }
        }
    }
}

void model::sparse_theta() {
    for (int m = 0; m < M; ++m) {
        int non_zero = 0;
        for (int k = 0; k < K; ++k) {
            if (nd[m][k] > 0) {
                non_zero++;
            }
        }
        int theta_it = non_zero * theta_perc / 100.0;
        for (int i = 0; i < theta_it; ++i) {
            int min = ndsum[m];
            int min_idx = 0;
            for (int k = 0; k < K; ++k) {
                if (nd[m][k] < min) {
                    min = nd[m][k];
                    min_idx = k;
                }
            }
            ndsum[m] -= min;
            nd[m][min_idx] = 0;
        }
    }
}

void model::sparse_phi() {
    for (int k = 0; k < K; ++k) {
        int non_zero = 0;
        for (int w = 0; w < V; ++w) {
            if (nw[w][k] > 0) {
                non_zero++;
            }
        }
        int phi_it = non_zero * phi_perc / 100.0;
        for (int i = 0; i < phi_it; ++i) {
            int min = nwsum[k];
            int min_idx = 0;
            for (int w = 0; w < V; ++w) {
                if (nw[w][k] < min) {
                    min = nw[w][k];
                    min_idx = w;
                }
            }
            nwsum[k] -= min;
            nw[k][min_idx] = 0;
        }
    }
}

void model::compute_distance() {
    int cp = 1;
    if (K > K_real) {
        cp = (K + K_real - 1) / K_real;
    }
    for (int i = 0; i < K; ++i) {
        for (int j = 0; j < K_real * cp; ++j) {
            distance(i, j) = utils::hellinger_dist(phi[i], phi_real[j % K_real], V);
            //cout << distance(i, j) << "->";
            distance(i, j) += gamma * utils::hellinger_dist(inv_theta[i], inv_theta_real[j % K_real], M);
            //cout << distance(i, j) << ' ';
        }
        //cout << endl;
    }
    Munkres m;
    m.solve(distance);
    double dist = 0;
    int h_topic = 0;
    //cout << "Dist: " << endl;
    for (int i = 0; i < K; ++i) {
        for (int j = 0; j < K_real * cp; ++j) {
            if (distance(i, j) > -eps) {
                h_topic = j % K_real;
                break;
            }
        }
        dist += utils::hellinger_dist(phi[i], phi_real[h_topic], V);
        //cout << dist << "->";
        dist += gamma * utils::hellinger_dist(inv_theta[i], inv_theta_real[h_topic], M);
        //cout << dist << endl;
    }
    dist /= K;
    //cout << " = " << dist << endl;
    if (distance_callback) {
        distance_callback(dist, liter, parent, experiment);
    }
}

int model::init_inf() {
    // estimating the model from a previously estimated one
    int m, n, w, k;

    p = new double[K];

    // load moel, i.e., read z and ptrndata
    if (load_model(model_name)) {
        printf("Fail to load word-topic assignmetn file of the model!\n");
        return 1;
    }

    nw = new int*[V];
    for (w = 0; w < V; w++) {
        nw[w] = new int[K];
        for (k = 0; k < K; k++) {
            nw[w][k] = 0;
        }
    }

    nd = new int*[M];
    for (m = 0; m < M; m++) {
        nd[m] = new int[K];
        for (k = 0; k < K; k++) {
            nd[m][k] = 0;
        }
    }

    nwsum = new int[K];
    for (k = 0; k < K; k++) {
        nwsum[k] = 0;
    }
    
    ndsum = new int[M];
    for (m = 0; m < M; m++) {
        ndsum[m] = 0;
    }

    for (m = 0; m < ptrndata->M; m++) {
        int N = ptrndata->docs[m]->length;

        // assign values for nw, nd, nwsum, and ndsum
        for (n = 0; n < N; n++) {
            int w = ptrndata->docs[m]->words[n];
            int topic = z[m][n];

            // number of instances of word i assigned to topic j
            nw[w][topic] += 1;
            // number of words in document i assigned to topic j
            nd[m][topic] += 1;
            // total number of words assigned to topic j
            nwsum[topic] += 1;
        }
        // total number of words in document i
        ndsum[m] = N;
    }
    
    // read new data for inference
    pnewdata = new dataset;
    if (withrawstrs) {
        if (pnewdata->read_newdata_withrawstrs(dir + dfile, dir + wordmapfile)) {
            printf("Fail to read new data!\n");
            return 1;
        }
    } else {
        if (pnewdata->read_newdata(dir + dfile, dir + wordmapfile)) {
            printf("Fail to read new data!\n");
            return 1;
        }
    }
    
    newM = pnewdata->M;
    newV = pnewdata->V;
    
    newnw = new int*[newV];
    for (w = 0; w < newV; w++) {
        newnw[w] = new int[K];
        for (k = 0; k < K; k++) {
            newnw[w][k] = 0;
        }
    }

    newnd = new int*[newM];
    for (m = 0; m < newM; m++) {
        newnd[m] = new int[K];
        for (k = 0; k < K; k++) {
            newnd[m][k] = 0;
        }
    }

    newnwsum = new int[K];
    for (k = 0; k < K; k++) {
        newnwsum[k] = 0;
    }
    
    newndsum = new int[newM];
    for (m = 0; m < newM; m++) {
        newndsum[m] = 0;
    }

    srandom(time(0)); // initialize for random number generation
    newz = new int*[newM];
    for (m = 0; m < pnewdata->M; m++) {
        int N = pnewdata->docs[m]->length;
        newz[m] = new int[N];

        // assign values for nw, nd, nwsum, and ndsum
        for (n = 0; n < N; n++) {
            int w = pnewdata->docs[m]->words[n];
            int _w = pnewdata->_docs[m]->words[n];
            int topic = (int)(((double)random() / RAND_MAX) * K);
            newz[m][n] = topic;

            // number of instances of word i assigned to topic j
            newnw[_w][topic] += 1;
            // number of words in document i assigned to topic j
            newnd[m][topic] += 1;
            // total number of words assigned to topic j
            newnwsum[topic] += 1;
        }
        // total number of words in document i
        newndsum[m] = N;
    }
    
    newtheta = new double*[newM];
    for (m = 0; m < newM; m++) {
        newtheta[m] = new double[K];
    }

    newphi = new double*[K];
    for (k = 0; k < K; k++) {
        newphi[k] = new double[newV];
    }
    
    return 0;
}

void model::inference() {
    if (twords > 0) {
        // print out top words per topic
        dataset::read_wordmap(dir + wordmapfile, &id2word);
    }

    printf("Sampling %d iterations for inference!\n", niters);
    
    for (inf_liter = 1; inf_liter <= niters; inf_liter++) {
        printf("Iteration %d ...\n", inf_liter);
        
        // for all newz_i
        for (int m = 0; m < newM; m++) {
            for (int n = 0; n < pnewdata->docs[m]->length; n++) {
                // (newz_i = newz[m][n])
                // sample from p(z_i|z_-i, w)
                int topic = inf_sampling(m, n);
                newz[m][n] = topic;
            }
        }
        if (progress_callback) {
            progress_callback((100 * inf_liter) / niters, parent, experiment);
        }
    }
    
    printf("Gibbs sampling for inference completed!\n");
    printf("Saving the inference outputs!\n");
    compute_newtheta();
    compute_newphi();
    inf_liter--;
    save_inf_model(dfile);
}

int model::inf_sampling(int m, int n) {
    // remove z_i from the count variables
    int topic = newz[m][n];
    int w = pnewdata->docs[m]->words[n];
    int _w = pnewdata->_docs[m]->words[n];
    newnw[_w][topic] -= 1;
    newnd[m][topic] -= 1;
    newnwsum[topic] -= 1;
    newndsum[m] -= 1;
    
    double Vbeta = V * beta;
    double Kalpha = K * alpha;
    // do multinomial sampling via cumulative method
    for (int k = 0; k < K; k++) {
        p[k] = (nw[w][k] + newnw[_w][k] + beta) / (nwsum[k] + newnwsum[k] + Vbeta) *
                (newnd[m][k] + alpha) / (newndsum[m] + Kalpha);
    }
    // cumulate multinomial parameters
    for (int k = 1; k < K; k++) {
        p[k] += p[k - 1];
    }
    // scaled sample because of unnormalized p[]
    double u = ((double)random() / RAND_MAX) * p[K - 1];
    
    for (topic = 0; topic < K; topic++) {
        if (p[topic] > u) {
            break;
        }
    }
    
    // add newly estimated z_i to count variables
    newnw[_w][topic] += 1;
    newnd[m][topic] += 1;
    newnwsum[topic] += 1;
    newndsum[m] += 1;
    
    return topic;
}

void model::compute_newtheta() {
    for (int m = 0; m < newM; m++) {
        for (int k = 0; k < K; k++) {
            newtheta[m][k] = (newnd[m][k] + alpha) / (newndsum[m] + K * alpha);
        }
    }
}

void model::compute_newphi() {
    map<int, int>::iterator it;
    for (int k = 0; k < K; k++) {
        for (int w = 0; w < newV; w++) {
            it = pnewdata->_id2id.find(w);
            if (it != pnewdata->_id2id.end()) {
                newphi[k][w] = (nw[it->second][k] + newnw[w][k] + beta) / (nwsum[k] + newnwsum[k] + V * beta);
            }
        }
    }
}

void model::generate_collection(string filename, int doc_len)
{
    if (progress_callback) {
        progress_callback(0, parent, experiment);
    }
    FILE *fout = fopen(filename.c_str(), "w");
    if (!fout) {
        printf("Couldn't open file %s to save!\n", filename.c_str());
        return;
    }

    fprintf(fout, "%d\n", M);
    gsl_rng_env_setup();
    const gsl_rng_type *T = gsl_rng_default;
    gsl_rng *r = gsl_rng_alloc (T);
    double *beta_vec = (double *)calloc(V, sizeof(*beta_vec));
    map<int, int>::const_iterator it;
    phi = (double **)calloc(K, sizeof(*phi));
    for (int i = 0; i < V; ++i) {
        beta_vec[i] = beta;
    }
    for (int t = 0; t < K; ++t) {
        phi[t] = (double *)calloc(V, sizeof(**phi));
        // for every topic draw a distribution over words
        gsl_ran_dirichlet(r, V, beta_vec, phi[t]);
    }
    free(beta_vec);
    unsigned int *tmp = (unsigned int *)calloc(K, sizeof(*tmp));
    unsigned int *tmp2 = (unsigned int *)calloc(V, sizeof(*tmp2));
    double alpha_vec[K];
    for (int i = 0; i < K; ++i) {
        alpha_vec[i] = alpha;
    }
    theta = (double **)calloc(M, sizeof(*theta));
    for (int d = 0; d < M; ++d) {
        theta[d] = (double *)calloc(K, sizeof(**theta));
        // for every document...
        // draw a vector of topic proportions
        gsl_ran_dirichlet(r, K, alpha_vec, theta[d]);
        // initialize document
        map<int, int> words;
        for (int w = 0; w < doc_len; ++w) {
            // for every word...
            // draw a topic assignment
            gsl_ran_multinomial(r, K, 1, theta[d], tmp);
            int topic = 0;
            for ( ; topic < K; ++topic) {
                if (tmp[topic]) {
                    break;
                }
            }
            // draw a word
            gsl_ran_multinomial(r, V, 1, phi[topic], tmp2);
            int word = 0;
            for ( ; word < V; ++word) {
                if (tmp2[word]) {
                    break;
                }
            }
            if (words.find(word) == words.end()) {
                // new word
                words[word] = 1;
            } else {
                words[word]++;
            }
        }/*
        for (it = words.begin(); it != words.end(); it++) {
            fprintf(fout, "%d:%d ", it->first, it->second);
        }*/
        for (int word = 0; word < V; ++word) {
            if (words.find(word) == words.end()) {
                fprintf(fout, "%d:0 ", word);
            } else {
                fprintf(fout, "%d:%d ", word, words.find(word)->second);
            }
        }
        fprintf(fout, "\n");
        if (progress_callback) {
            progress_callback((100 * (d + 1)) / M, parent, experiment);
        }
    }

    fclose(fout);
    gsl_rng_free(r);
    free(tmp);
    free(tmp2);
    save_model_theta(filename + theta_suffix);
    save_model_phi(filename + phi_suffix);
    save_model_others(filename + others_suffix);
    if (progress_callback) {
        progress_callback(100, parent, experiment);
    }
}

void model::set_callbacks(PCALLBACK progress, DCALLBACK perplexity, DCALLBACK distance, void *parent_)
{
    progress_callback = progress;
    perplexity_callback = perplexity;
    distance_callback = distance;
    parent = parent_;
}

void model::set_topics_number(int n_topics)
{
    K = n_topics;
}

void model::set_real_topics_number(int n_topics_real)
{
    K_real = n_topics_real;
}

void model::set_document_number(int n_docs)
{
    M = n_docs;
}

void model::set_words_number(int n_words)
{
    V = n_words;
}

void model::set_data_file(const string &data_file)
{
    dfile = data_file;
}

void model::set_work_dir(const string &work_dir)
{
    dir = work_dir;
}

void model::set_alpha(double new_alpha)
{
    alpha = new_alpha;
}

void model::set_beta(double new_beta)
{
    beta = new_beta;
}

void model::set_hyperparameters(double new_alpha, double new_beta)
{
    alpha = new_alpha;
    beta = new_beta;
}

void model::set_model_name(const string &new_name)
{
    model_name = new_name;
}

void model::set_model_status(int i)
{
    model_status = i;
}

void model::set_experiment_number(int n_experiment)
{
    experiment = n_experiment;
}

void model::set_iterations(int n_iters)
{
    niters = n_iters;
}

void model::set_last_iteration(int new_liter)
{
    liter = new_liter;
}

void model::set_save_iteration(int n_save_iters)
{
    savestep = n_save_iters;
}

void model::set_perplexity_iter(int new_piter)
{
    piter = new_piter;
}

void model::set_twords(int new_twords)
{
    twords = new_twords;
}

void model::set_gamma(double new_gamma)
{
    gamma = new_gamma;
}

void model::use_half_smoothing(bool use_half_)
{
    use_soft = use_half_;
}

void model::use_hungarian_algorithm(bool use_hungarian_)
{
    use_hungarian = use_hungarian_;
}

void model::set_robast_iter(int r_iter_)
{
    r_iter = r_iter_;
}

void model::set_phi_percentage(int phi_perc_)
{
    phi_perc = phi_perc_;
}

void model::set_theta_percentage(int theta_perc_)
{
    theta_perc = theta_perc_;
}

void model::set_withrawstrs(int withrawstrs_)
{
    withrawstrs = withrawstrs_;
}

double model::get_alpha(void) const
{
    return alpha;
}

double model::get_beta(void) const
{
    return beta;
}

int model::get_document_number(void) const
{
    return M;
}

int model::get_topics_number(void) const
{
    return K;
}

int model::get_words_number(void) const
{
    return V;
}

string model::get_model_name(void) const
{
    return model_name;
}

string model::get_work_dir(void) const
{
    return dir;
}

string model::get_data_file(void) const
{
    return dfile;
}

string model::get_others_suffix(void) const
{
    return others_suffix;
}

int model::get_doc_length(int document) const
{
    if (document >= M) {
        return 0;
    }
    return ptrndata->docs[document]->length;
}

int model::get_doc_real_length(int document) const
{
    if (document >= M) {
        return 0;
    }
    return ptrndata->docs[document]->real_length;
}

int model::get_word_counts(int document, int word) const
{
    if (document >= M || word >= ptrndata->docs[document]->length) {
        return 0;
    }
    return ptrndata->docs[document]->word_counts[word];
}

int model::get_word(int document, int word_number) const
{
    if (document >= M || word_number >= ptrndata->docs[document]->length) {
        return 0;
    }
    return ptrndata->docs[document]->words[word_number];
}

double model::get_phi(int word, int topic) const
{
    if (word >= V || topic >= K) {
        return -1;
    }
    return phi[topic][word];
}

double model::get_theta(int topic, int document) const
{
    if (topic >= K || document >= M) {
        return -1;
    }
    return theta[document][topic];
}

int model::get_model_status(void) const
{
    return model_status;
}
