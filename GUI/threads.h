#ifndef THREADS_H
#define THREADS_H

#include <QThread>
#include <QString>
#include <string>
#include <cstdlib>
#include <cstdio>

#include "model.h"

class EstimateThread : public QThread
{
    Q_OBJECT

signals:
    void finished();

private:
    model *lda;
    QString phiPath;
    QString thetaPath;
    int KReal;
    bool useHungarian;

protected:
    void run();

public:
    ~EstimateThread();
    EstimateThread(model *pmodel,
                   QString phiPath_,
                   QString thetaPath_,
                   int KReal_,
                   bool useHungarian_);
};

class InferenceThread : public QThread
{
    Q_OBJECT

signals:
    void finished();

private:
    model *lda;

protected:
    void run();

public:
    ~InferenceThread();
    explicit InferenceThread(model *pmodel);
};

class GenerateThread : public QThread
{
    Q_OBJECT

signals:
    void finished();

private:
    model *lda;
    string filename;
    int doc_len;

protected:
    void run();

public:
    ~GenerateThread();
    explicit GenerateThread(model *pmodel, string filename, int doc_len);
};

#endif // THREADS_H
