#include "model.h"
#include <cstdlib>
#include <iostream>

using namespace std;

void show_help(void);

int main(int argc, char **argv)
{
    model lda;

    if (lda.init(argc, argv)) {
        show_help();
        return 1;
    }

    if (lda.get_model_status() == MODEL_STATUS_EST || lda.get_model_status() == MODEL_STATUS_ESTC) {
        // parameter estimation
        lda.estimate();
    }

    if (lda.get_model_status() == MODEL_STATUS_INF) {
        // do inference
        lda.inference();
    }

    return 0;
}

void show_help(void)
{
    cout << "Command line usage:" << endl;
    cout << "\tlda -est [-alpha <double>] [-beta <double>] [-ntopics <int>] -niters <int> [-savestep <int>] [-twords <int>] -dfile <string> " <<
            "[-use_undarian -real_phi <file> -real_theta <file> -real_ntopics <int> [-gamma <double>]] [-use_half_smooth]" << endl;
    cout << "\tlda -estc -dir <string> -model <string> -niters <int> -savestep <int> -twords <int>" << endl;
    cout << "\tlda -inf -dir <string> -model <string> -niters <int> -twords <int> -dfile <string>" << endl;
}

