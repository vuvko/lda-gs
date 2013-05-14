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
        delete p;
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
                delete z[m];
            }
        }
    }
    
    if (nw) {
        for (int w = 0; w < V; w++) {
            if (nw[w]) {
                delete nw[w];
            }
        }
    }

    if (nd) {
        for (int m = 0; m < M; m++) {
            if (nd[m]) {
                delete nd[m];
            }
        }
    }
    
    if (nwsum) {
        delete nwsum;
    }
    
    if (ndsum) {
        delete ndsum;
    }
    
    if (theta) {
        for (int m = 0; m < M; m++) {
            if (theta[m]) {
                delete theta[m];
            }
        }
    }
    
    if (theta_real) {
        for (int m = 0; m < M; m++) {
            if (theta_real[m]) {
                delete theta_real[m];
            }
        }
    }

    if (phi) {
        for (int k = 0; k < K; k++) {
            if (phi[k]) {
                delete phi[k];
            }
        }
    }

    if (phi_real) {
        for (int k = 0; k < K; k++) {
            if (phi_real[k]) {
                delete phi_real[k];
            }
        }
    }

    // only for inference
    if (newz) {
        for (int m = 0; m < newM; m++) {
            if (newz[m]) {
                delete newz[m];
            }
        }
    }
    
    if (newnw) {
        for (int w = 0; w < newV; w++) {
            if (newnw[w]) {
                delete newnw[w];
            }
        }
    }

    if (newnd) {
        for (int m = 0; m < newM; m++) {
            if (newnd[m]) {
                delete newnd[m];
            }
        }
    }
    
    if (newnwsum) {
        delete newnwsum;
    }
    
    if (newndsum) {
        delete newndsum;
    }
    
    if (newtheta) {
        for (int m = 0; m < newM; m++) {
            if (newtheta[m]) {
                delete newtheta[m];
            }
        }
    }
    
    if (newphi) {
        for (int k = 0; k < K; k++) {
            if (newphi[k]) {
                delete newphi[k];
            }
        }
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

void model::set_default_values() {
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
    if (p) {
        delete p;
    }
    p = NULL;
    if (ptrndata) {
        delete ptrndata;
    }
    ptrndata = NULL;
    if (pnewdata) {
        delete pnewdata;
    }
    pnewdata = NULL;
    use_hungarian = false;
    if (z) {
        for (int m = 0; m < M; m++) {
            if (z[m]) {
                delete z[m];
            }
        }
    }
    z = NULL;
    if (nw) {
        for (int w = 0; w < V; w++) {
            if (nw[w]) {
                delete nw[w];
            }
        }
    }
    nw = NULL;
    if (nd) {
        for (int m = 0; m < M; m++) {
            if (nd[m]) {
                delete nd[m];
            }
        }
    }
    nd = NULL;
    if (nwsum) {
        delete nwsum;
    }
    nwsum = NULL;
    if (ndsum) {
        delete ndsum;
    }
    ndsum = NULL;
    if (theta) {
        for (int m = 0; m < M; m++) {
            if (theta[m]) {
                delete theta[m];
            }
        }
    }
    theta = NULL;
    if (theta_real) {
        for (int m = 0; m < M; m++) {
            if (theta_real[m]) {
                delete theta_real[m];
            }
        }
    }
    theta_real = NULL;
    if (phi) {
        for (int k = 0; k < K; k++) {
            if (phi[k]) {
                delete phi[k];
            }
        }
    }
    phi = NULL;
    if (phi_real) {
        for (int k = 0; k < K; k++) {
            if (phi_real[k]) {
                delete phi_real[k];
            }
        }
    }
    phi_real = NULL;
    // only for inference
    if (newz) {
        for (int m = 0; m < newM; m++) {
            if (newz[m]) {
                delete newz[m];
            }
        }
    }
    newz = NULL;
    if (newnw) {
        for (int w = 0; w < newV; w++) {
            if (newnw[w]) {
                delete newnw[w];
            }
        }
    }
    newnw = NULL;
    if (newnd) {
        for (int m = 0; m < newM; m++) {
            if (newnd[m]) {
                delete newnd[m];
            }
        }
    }
    newnd = NULL;
    if (newnwsum) {
        delete newnwsum;
    }
    newnwsum = NULL;
    if (newndsum) {
        delete newndsum;
    }
    newndsum = NULL;
    if (newtheta) {
        for (int m = 0; m < newM; m++) {
            if (newtheta[m]) {
                delete newtheta[m];
            }
        }
    }
    newtheta = NULL;
    if (newphi) {
        for (int k = 0; k < K; k++) {
            if (newphi[k]) {
                delete newphi[k];
            }
        }
    }
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
    int i, j;
    
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        printf("Cannot open file %s to save!\n", filename.c_str());
        return 1;
    }

    // wirte docs with topic assignments for words
    for (i = 0; i < ptrndata->M; i++) {
        for (j = 0; j < ptrndata->docs[i]->length; j++) {
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
        progress_callback(0, parent);
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
                continue;
            }
            //for (int k = 0; k < ptrndata->docs[m]->word_counts[n]; ++k) {
            int topic = (int)(((double)random() / RAND_MAX) * K);
            z[m][n] = topic;

            // number of instances of word i assigned to topic j
            nw[ptrndata->docs[m]->words[n]][topic] += 1;
            // number of words in document i assigned to topic j
            nd[m][topic] += 1;
            // total number of words assigned to topic j
            nwsum[topic] += 1;
            ndsum[m] += 1;
            //}
        }
        // total number of words in document i
        //ndsum[m] = ptrndata->docs[m]->real_length;
    }
    
    theta = new double*[M];
    for (m = 0; m < M; m++) {
        theta[m] = new double[K];
    }

    phi = new double*[K];
    for (k = 0; k < K; k++) {
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
        progress_callback(0, parent);
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

#include <iostream>

void model::estimate() {
    if (twords > 0) {
        // print out top words per topic
        dataset::read_wordmap(dir + wordmapfile, &id2word);
    }

    printf("Sampling %d iterations!\n", niters);

    int last_iter = liter;
    for (liter = last_iter + 1; liter <= niters + last_iter; liter++) {
        printf("Iteration %d ...\n", liter);
        
        // for all z_i
        for (int m = 0; m < M; m++) {
            for (int n = 0; n < ptrndata->docs[m]->length; n++) {
                if (ptrndata->docs[m]->word_counts[n] == 0) {
                    continue;
                }
                for (int k = 0; k < ptrndata->docs[m]->word_counts[n]; ++k) {
                    // (z_i = z[m][n])
                    // sample from p(z_i|z_-i, w)
                    int topic = sampling(m, n);
                    z[m][n] = topic;
                }

            }
        }
        
        if (r_iter > 0) {
            if (liter % r_iter == 0) {
                long long phi_it = K * V * (phi_perc / 100.0);
                long long theta_it = M * K * (theta_perc / 100.0);
                for (int k = 0; k < K && theta_it; ++k) {
                    for (int m = 0; m < M && theta_it; ++m) {
                        double r = (rand() + 0.0) / RAND_MAX * 0.1;
                        if (r > (nd[m][k] + alpha) / (ndsum[m] + K * alpha)) {
                        //if ((nd[m][k] + alpha) / (ndsum[m] + K * alpha) < 0.1) {
                            ndsum[m] -= nd[m][k];
                            nd[m][k] = 0;
                            theta_it--;
                        }
                    }
                }
                for (int k = 0; k < K && phi_it; ++k) {
                    for (int w = 0; w < V && phi_it; ++w) {
                        double r = (rand() + 0.0) / RAND_MAX * 0.1;
                        if (r > (nw[w][k] + beta) / (nwsum[k] + V * beta)) {
                        //if ((nw[w][k] + beta) / (nwsum[k] + V * beta) < 0.1) {
                            nwsum[k] -= nw[w][k];
                            nw[w][k] = 0;
                            phi_it--;
                        }
                    }
                }
            }
        }
        if (savestep > 0) {
            if (liter % savestep == 0) {
                // saving the model
                printf("Saving the model at iteration %d ...\n", liter);
                compute_theta();
                compute_phi();
                save_model(utils::generate_model_name(liter));
            }
        }
        if (piter > 0) {
            if (liter % piter == 0) {
                compute_theta();
                compute_phi();
                if (perplexity_callback) {
                    perplexity_callback(utils::calc_perplexity(this), liter, parent);
                }
                if (use_hungarian) {
                    int cp = 1;
                    if (K > K_real) {
                        cp = (K + K_real - 1) / K_real;
                    }
                    for (int i = 0; i < K; ++i) {
                        for (int j = 0; j < K_real * cp; ++j) {
                            double tmp = utils::compute_dist(phi[i], phi_real[j % K_real], V);
                            cout << tmp << ' ';
                            distance(i, j) = tmp;
                        }
                        cout << endl;
                    }
                    Munkres m;
                    m.solve(distance);
                    double dist = 0;
                    int h_topic = 0;
                    for (int i = 0; i < K; ++i) {
                        for (int j = 0; j < K_real * cp; ++j) {
                            if (distance(i, j) > -eps) {
                                h_topic = j % K_real;
                                break;
                            }
                        }
                        dist += utils::compute_dist(phi[i], phi_real[h_topic], V);
                    }
                    dist /= K_real;
                    if (distance_callback) {
                        distance_callback(dist, liter, parent);
                    }
                }
            }
        }
        if (progress_callback) {
            progress_callback((100 * (liter - last_iter)) / niters, parent);
        }
    }
    
    printf("Gibbs sampling completed!\n");
    printf("Saving the final model!\n");
    compute_theta();
    compute_phi();
    if (perplexity_callback) {
        perplexity_callback(utils::calc_perplexity(this), liter, parent);
    }
    liter--;
    save_model(utils::generate_model_name(-1));
}

int model::sampling(int m, int n) {
    // remove z_i from the count variables
    int topic = z[m][n];
    int w = ptrndata->docs[m]->words[n];
    nw[w][topic] -= 1;
    nd[m][topic] -= 1;
    nwsum[topic] -= 1;
    ndsum[m] -= 1;

    if (nw[w][topic] < 0) {
        cout << "nw " << w << ':' << topic << " --- 0!!!" << endl;
        nw[w][topic] += 1;
        nwsum[topic] += 1;
    }
    if (nd[m][topic] < 0) {
        cout << "nd " << m << ':' << topic << " --- 0!!!" << endl;
        nd[m][topic] += 1;
        ndsum[topic] += 1;
    }
    if (nwsum[topic] < 0) {
        cout << "nwsum " << topic << " --- 0!!!" << endl;
    }
    if (ndsum[m] < 0) {
        cout << "ndsum " << m << " --- 0!!!" << endl;
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
    nw[w][topic] += 1;
    nd[m][topic] += 1;
    nwsum[topic] += 1;
    ndsum[m] += 1;
    
    return topic;
}

void model::compute_theta() {
    for (int m = 0; m < M; m++) {
        for (int k = 0; k < K; k++) {
            if (nd[m][k] < 0) {
                cout << "t nd " << m << ':' << k << " 0" << endl;
            }
            if (ndsum[m] < 0) {
                cout << "t ndsum" << m << " 0" << endl;
            }
            theta[m][k] = (nd[m][k] + alpha) / (ndsum[m] + K * alpha);
            if (theta[m][k] < 0) {
                cout << "theta < 0!!! " << m << ':' << k << endl;
            }
        }
    }
}

void model::compute_phi() {
    for (int k = 0; k < K; k++) {
        for (int w = 0; w < V; w++) {
            if (nw[w][k] < 0) {
                cout << "p nw " << w << ':' << k << " 0" << endl;
            }
            if (nwsum[k] < 0) {
                cout << "p nwsum " << k << " 0" << endl;
            }
            phi[k][w] = (nw[w][k] + beta) / (nwsum[k] + V * beta);
            if (phi[k][w] < 0) {
                cout << "phi 0!!!" << k << ':' << w << endl;
            }
        }
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
            progress_callback((100 * inf_liter) / niters, parent);
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
        progress_callback(0, parent);
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
            progress_callback((100 * (d + 1)) / M, parent);
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
        progress_callback(100, parent);
    }
}
