#include "threads.h"

EstimateThread::EstimateThread(
        model *pmodel,
        QString phiPath_,
        QString thetaPath_,
        int KReal_,
        bool useHungarian_) :
    lda(pmodel), phiPath(phiPath_), thetaPath(thetaPath_), KReal(KReal_),
    useHungarian(useHungarian_) {}

void EstimateThread::run()
{
    stdout = fopen("lda.log", "w");
    lda->use_hungarian = useHungarian;
    lda->K_real = KReal;
    lda->init_est();
    lda->load_phi(phiPath.toStdString());
    lda->load_theta(thetaPath.toStdString());
    lda->estimate();
    fclose(stdout);
    emit finished();
}

EstimateThread::~EstimateThread()
{
    delete lda;
}

InferenceThread::InferenceThread(model *pmodel) : lda(pmodel) {}

void InferenceThread::run()
{
    stdout = fopen("lda.log", "w");
    lda->init_inf();
    lda->inference();
    delete lda;
    fclose(stdout);
    emit finished();
}

InferenceThread::~InferenceThread()
{
    delete lda;
}

GenerateThread::GenerateThread(model *pmodel, string filename_, int doc_len_) :
    lda(pmodel), filename(filename_), doc_len(doc_len_) {}

void GenerateThread::run()
{
    stdout = fopen("lda.log", "w");
    lda->generate_collection(filename, doc_len);
    delete lda;
    fclose(stdout);
    emit finished();
}

GenerateThread::~GenerateThread()
{
    delete lda;
}
