#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QTextStream>
#include <QCoreApplication>
#include <QPolygon>
#include <QImage>
#include <QImageWriter>
#include <qmath.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_legend.h>
#include <qwt/qwt_plot_grid.h>
#include <QDebug>
#include <cstdlib>
#include <cstdio>
#include <string>

#include "threads.h"
#include "model.h"
#include "utils.h"

namespace Ui {
class MainWindow;
}

class  QProgressEvent : public QEvent
{
public:
    int percent;
    QProgressEvent() : QEvent((QEvent::Type)(QEvent::User + 1)) {}
};
/*
class  QMessageEvent : public QEvent
{
public:
    string message;
    QMessageEvent() : QEvent((QEvent::Type)(QEvent::User + 2)) {}
};
*/
class  QPerplexityEvent : public QEvent
{
public:
    int iter;
    double perplexity;
    QPerplexityEvent() : QEvent((QEvent::Type)(QEvent::User + 3)) {}
};

class  QDistanceEvent : public QEvent
{
public:
    int iter;
    double distance;
    QDistanceEvent() : QEvent((QEvent::Type)(QEvent::User + 4)) {}
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void Estimate();
    void Inference();
    void Generate();
    void BrowseDataEst();
    void BrowseDataInf();
    void BrowseModelInf();
    void BrowseCollGen();
    void BrowsePhi();
    void BrowseTheta();
    void LoadModel();
    void SaveModel();
    void SavePlot();
    void ClearPlot();
    void About();

    void UseHungarian(int);
    void DirichletParams(int);
    void UseRobast(int);

    void ThreadFinish();
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    bool event(QEvent *event);
    
private:
    Ui::MainWindow *ui;
    //model lda;
    bool running;
    EstimateThread *estThread;
    InferenceThread *infThread;
    GenerateThread *genThread;
    double *perplexity;
    double *distance;
    double *iters;
    int lastIter;
    QwtPlot *perplexityPlot;
    QwtPlot *distancePlot;
    QwtPlotCurve *perplexityCurve;
    QwtPlotCurve *distanceCurve;
    QwtPlotGrid *perplexityGrid;
    QwtPlotGrid *distanceGrid;
    QFile *logFile;
    QTextStream *log;

    void setControls(bool enable);

    static void progressCallback(int percent, MainWindow *toThis);
    //static void messageCallback(string message, MainWindow *toThis);
    static void perplexityCallback(double perplexity, int iter, MainWindow *toThis);
    static void distanceCallback(double distance, int iter, MainWindow *toThis);
};

#endif // MAINWINDOW_H
