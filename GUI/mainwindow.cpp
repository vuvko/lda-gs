#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->centralWidget->setLayout(ui->mainLayout);
    ui->estTab->setLayout(ui->estLayout);
    ui->infTab->setLayout(ui->infLayout);
    ui->genTab->setLayout(ui->genLayout);
    ui->perpTab->setLayout(ui->perpLayout);
    ui->distTab->setLayout(ui->distLayout);
    perplexityPlot = new QwtPlot(tr("Perplexity"));
    distancePlot = new QwtPlot(tr("Mean of hellinger distance"));
    perplexityPlot->setAxisTitle(QwtPlot::xBottom, "Iterations");
    distancePlot->setAxisTitle(QwtPlot::xBottom, "Iterations");
    perplexityPlot->setCanvasBackground(QBrush(Qt::white));
    distancePlot->setCanvasBackground(QBrush(Qt::white));
    ui->perpLayout->addWidget(perplexityPlot);
    ui->distLayout->addWidget(distancePlot);
    perplexityCurves = 0;
    distanceCurves = 0;
    perplexityAvrCurve = new QwtPlotCurve(tr("Average perplexity"));
    perplexityAvrCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    perplexityAvrCurve->setPen(Qt::red, 3);
    perplexityAvrCurve->attach(perplexityPlot);
    distanceAvrCurve = new QwtPlotCurve(tr("Average distance"));
    distanceAvrCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    distanceAvrCurve->setPen(Qt::red, 3);
    distanceAvrCurve->attach(distancePlot);
    perplexityGrid = new QwtPlotGrid;
    distanceGrid = new QwtPlotGrid;
    perplexityGrid->enableX(true);
    distanceGrid->enableX(true);
    perplexityGrid->enableXMin(true);
    distanceGrid->enableXMin(true);
    perplexityGrid->enableY(true);
    distanceGrid->enableY(true);
    perplexityGrid->enableYMin(true);
    distanceGrid->enableYMin(true);
    perplexityGrid->setMinorPen(QPen(Qt::DotLine));
    distanceGrid->setMinorPen(QPen(Qt::DotLine));
    perplexityGrid->attach(perplexityPlot);
    distanceGrid->attach(distancePlot);
    distance = 0;
    distanceAvr = 0;
    perplexity = 0;
    perplexityAvr = 0;
    iters = 0;
    infThread = 0;
    estThreads = 0;
    genThread = 0;
    running = false;
    nExperiments = 10;
    lastIter = new int[nExperiments];
    connect(ui->actionLoad, SIGNAL(triggered()), this, SLOT(LoadModel()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(SaveModel()));
    connect(ui->actionSavePlot, SIGNAL(triggered()), this, SLOT(SavePlot()));
    connect(ui->actionSavePlotVector, SIGNAL(triggered()), this, SLOT(SavePlotVector()));
    connect(ui->actionClearPlot, SIGNAL(triggered()), this, SLOT(ClearPlot()));
    connect(ui->estButton, SIGNAL(clicked()), this, SLOT(Estimate()));
    connect(ui->infButton, SIGNAL(clicked()), this, SLOT(Inference()));
    connect(ui->genButton, SIGNAL(clicked()), this, SLOT(Generate()));
    connect(ui->estDataButton, SIGNAL(clicked()), this, SLOT(BrowseDataEst()));
    connect(ui->infModelButton, SIGNAL(clicked()), this, SLOT(BrowseModelInf()));
    connect(ui->infDataButton, SIGNAL(clicked()), this, SLOT(BrowseDataInf()));
    connect(ui->genDataButton, SIGNAL(clicked()), this, SLOT(BrowseCollGen()));
    connect(ui->phiPathButton, SIGNAL(clicked()), this, SLOT(BrowsePhi()));
    connect(ui->thetaPathButton, SIGNAL(clicked()), this, SLOT(BrowseTheta()));
    connect(ui->actionAbout, SIGNAL(changed()), this, SLOT(About()));
    connect(ui->hungarianBox, SIGNAL(stateChanged(int)), this, SLOT(UseHungarian(int)));
    UseHungarian(0);
    connect(ui->dirichletBox, SIGNAL(stateChanged(int)), this, SLOT(DirichletParams(int)));
    DirichletParams(0);
    connect(ui->robastBox, SIGNAL(stateChanged(int)), this, SLOT(UseRobast(int)));
    UseRobast(0);
    logFile = new QFile("lda.log");
    if (!logFile->open(QFile::ReadOnly))
        qDebug() << "error opening log file!";
    log = new QTextStream(logFile);

}

MainWindow::~MainWindow()
{
    delete ui;
    if (estThreads)
    {
        for (int i = 0; i < nExperiments; ++i)
            delete estThreads[i];
        delete [] estThreads;
    }
    if (infThread)
        delete infThread;
    if (genThread)
        delete genThread;
    if (perplexityCurves)
    {
        for (int i = 0; i < nExperiments; ++i)
            delete perplexityCurves[i];
        delete [] perplexityCurves;
    }
    delete perplexityAvrCurve;
    delete perplexityGrid;
    delete perplexityPlot;
    if (distanceCurves)
    {
        for (int i = 0; i < nExperiments; ++i)
            delete distanceCurves[i];
        delete [] distanceCurves;
    }
    delete distanceAvrCurve;
    delete distanceGrid;
    delete distancePlot;
    if (distance)
    {
        for (int i = 0; i < nExperiments; ++i)
            delete [] distance[i];
        delete [] distance;
    }
    if (distanceAvr)
        delete [] distanceAvr;
    if (perplexity)
    {
        for (int i = 0; i < nExperiments; ++i)
            delete [] perplexity[i];
        delete [] perplexity;
    }
    if (perplexityAvr)
        delete [] perplexityAvr;
    if (iters)
        free(iters);
    logFile->close();
    delete log;
    delete logFile;
    if (lastIter)
        delete [] lastIter;
}

void MainWindow::About() {}

void MainWindow::BrowseDataEst()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose data file"), QDir::currentPath());
    if (!fileName.isNull())
        ui->estDataEdit->setText(fileName);
}

void MainWindow::BrowseDataInf()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose data file"), QDir::currentPath());
    if (!fileName.isNull())
        ui->infDataEdit->setText(fileName);
}

void MainWindow::BrowsePhi()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose file with phi distribution"), QDir::currentPath());
    if (!fileName.isNull())
        ui->phiPathEdit->setText(fileName);
}

void MainWindow::BrowseTheta()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose file with theta distribution"), QDir::currentPath());
    if (!fileName.isNull())
        ui->thetaPathEdit->setText(fileName);
}

void MainWindow::BrowseModelInf()
{
    model lda;
    char filter[255];
    sprintf(filter, "Model (*%s)", lda.get_others_suffix().c_str());
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose model parameters file"),
                                                    QDir::currentPath(),
                                                    tr(filter));
    if (!fileName.isNull())
        ui->infModelEdit->setText(fileName);
}

void MainWindow::BrowseCollGen()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save data file"), QDir::currentPath());
    if (!fileName.isNull())
        ui->genDataEdit->setText(fileName);
}

void MainWindow::Estimate()
{
    if (running)
        return;
    setControls(false);
    int nIters = ui->estIterBox->value();
    int pIter = ui->estPerpBox->value();
    perplexityPlot->setAxisScale(QwtPlot::xBottom, 1, nIters);
    distancePlot->setAxisScale(QwtPlot::xBottom, 1, nIters);
    int size = (nIters + pIter - 1) / pIter;
    if (!iters)
        iters = (double *)calloc(size + 1, sizeof(*iters));
    else
        iters = (double *)realloc(iters, (size + 1) * sizeof(*iters));
    if (perplexity)
    {
        for (int i = 0; i < nExperiments; ++i)
            delete [] perplexity[i];
        delete [] perplexity;
    }
    perplexity = new double *[nExperiments];
    if (perplexityAvr)
        delete perplexityAvr;
    perplexityAvr = new double[size + 1];
    for (int i = 0; i <= size; ++i) {
        perplexityAvr[i] = 0;
    }
    perplexityAvrCurve->setRawSamples(iters, perplexityAvr, 0);
    if (distance)
    {
        for (int i = 0; i < nExperiments; ++i)
            delete [] distance[i];
        delete [] distance;
    }
    if (distanceAvr)
        delete distanceAvr;
    distanceAvrCurve->setRawSamples(iters, distanceAvr, 0);
    distanceAvr = new double [size + 1];
    distance = new double *[nExperiments];
    QString phiPath = ui->phiPathEdit->text();
    QString thetaPath = ui->thetaPathEdit->text();
    int KReal = ui->realTopicBox->value();
    bool useHungarian = ui->hungarianBox->isChecked();
    if (estThreads)
    {
        for (int i = 0; i < nExperiments; ++i)
            delete estThreads[i];
        delete [] estThreads;
    }
    estThreads = new EstimateThread *[nExperiments];
    if (perplexityCurves)
    {
        for (int i = 0; i < nExperiments; ++i)
            perplexityCurves[i]->detach();
        delete [] perplexityCurves;
    }
    perplexityCurves = new QwtPlotCurve *[nExperiments];
    if (distanceCurves)
    {
        for (int i = 0; i < nExperiments; ++i)
            distanceCurves[i]->detach();
        delete [] distanceCurves;
    }
    distanceCurves = new QwtPlotCurve *[nExperiments];
    char buff[BUFF_SIZE];
    for (int i = 0; i < nExperiments; ++i) {
        model *lda = new model;
        lda->set_callbacks((PCALLBACK)progressCallback, (DCALLBACK)perplexityCallback, (DCALLBACK)distanceCallback, this);
        lda->set_robast_iter(ui->iterRobBox->value());
        lda->set_phi_percentage(ui->phiRobBox->value());
        lda->set_theta_percentage(ui->thetaRobBox->value());
        lda->set_model_status(MODEL_STATUS_EST);
        lda->set_topics_number(ui->estTopicBox->value());
        if (ui->alphaBox->value() > -eps)
            lda->set_alpha(ui->alphaBox->value());
        if (ui->betaBox->value() > -eps)
            lda->set_beta(ui->betaBox->value());
        lda->set_iterations(nIters);
        lda->set_save_iteration(ui->estSaveBox->value());
        if (ui->estTwordBox->value() > 0)
            lda->set_twords(ui->estTwordBox->value());
        lda->set_data_file(ui->estDataEdit->text().toStdString());
        string::size_type idx = lda->get_data_file().find_last_of("/");
        if (idx == string::npos) {
            lda->set_work_dir("./");
        } else {
            lda->set_work_dir(lda->get_data_file().substr(0, idx + 1));
            lda->set_data_file(lda->get_data_file().substr(idx + 1, lda->get_data_file().size() - lda->get_work_dir().size()));
        }
        lda->set_perplexity_iter(pIter);
        lda->use_half_smoothing(ui->softBox->isChecked());
        lda->set_gamma(ui->gammaBox->value());
        lda->set_experiment_number(i);
        sprintf(buff, "experiment-%d", i);
        lda->set_model_name(buff);
        lastIter[i] = -1;
        perplexity[i] = new double[size + 1];
        distance[i] = new double[size + 1];
        perplexityCurves[i] = new QwtPlotCurve();
        perplexityCurves[i]->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        perplexityCurves[i]->setPen(Qt::blue, 2);
        perplexityCurves[i]->attach(perplexityPlot);
        perplexityCurves[i]->setRawSamples(iters, perplexity[i], 0);
        distanceCurves[i] = new QwtPlotCurve();
        distanceCurves[i]->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        distanceCurves[i]->setPen(Qt::green, 2);
        distanceCurves[i]->attach(distancePlot);
        distanceCurves[i]->setRawSamples(iters, distance[i], 0);
        estThreads[i] = new EstimateThread(lda, phiPath, thetaPath, KReal, useHungarian);
        connect(estThreads[i], SIGNAL(finished()), this, SLOT(ThreadFinish()));
        estThreads[i]->start();
    }
    perplexityAvrCurve->detach();
    perplexityAvrCurve->attach(perplexityPlot);
    distanceAvrCurve->detach();
    distanceAvrCurve->attach(distancePlot);
}

void MainWindow::Inference()
{
    if (running)
        return;
    setControls(false);
    model *lda = new model;
    lda->set_callbacks((PCALLBACK)progressCallback, (DCALLBACK)perplexityCallback, (DCALLBACK)distanceCallback, this);
    lda->set_model_status(MODEL_STATUS_INF);
    lda->set_iterations(ui->infIterBox->value());
    if (ui->infTwordsBox->value() > 0)
        lda->set_twords(ui->infTwordsBox->value());
    lda->set_data_file(ui->infDataEdit->text().toStdString());
    string::size_type idx = lda->get_data_file().find_last_of("/");
    if (idx != string::npos) {
        lda->set_data_file(lda->get_data_file().substr(idx + 1, lda->get_data_file().size() - lda->get_work_dir().size()));
    }
    string ldaParam = ui->infModelEdit->text().toStdString();
    idx = ldaParam.find_last_of("/");
    if (idx == string::npos) {
        lda->set_work_dir("./");
    } else {
        lda->set_work_dir(ldaParam.substr(0, idx + 1));
    }
    utils::read_and_parse(ldaParam, lda);
    if (infThread)
        delete infThread;
    infThread = new InferenceThread(lda);
    connect(infThread, SIGNAL(finished()), this, SLOT(ThreadFinish()));
    infThread->start();
}

void MainWindow::Generate()
{
    if (ui->genDataEdit->text().isNull())
        return;
    if (running)
        return;
    setControls(false);
    ui->statusBar->showMessage("Generating document collection...");
    model *lda = new model;
    lda->set_callbacks((PCALLBACK)progressCallback, 0, 0, this);
    lda->set_document_number(ui->genNdocBox->value());
    lda->set_topics_number(ui->genNTopicBox->value());
    lda->set_words_number(ui->genNWordsBox->value());
    lda->set_hyperparameters(ui->genAlphaBox->value(), ui->genBetaBox->value());
    lda->set_experiment_number(0);
    if (genThread)
        delete genThread;
    genThread = new GenerateThread(lda, ui->genDataEdit->text().toStdString(), ui->genNDWordsBox->value());
    connect(genThread, SIGNAL(finished()), this, SLOT(ThreadFinish()));
    genThread->start();
}

void MainWindow::LoadModel() {}
void MainWindow::SaveModel() {}

void MainWindow::SavePlot()
{
    QList<QByteArray> formats = QImageWriter::supportedImageFormats();
    QString format, selected;
    for (int i = 0; i < formats.size(); ++i)
        format.append("Format " + formats[i] + "(*." + formats[i] + ");;");
    format.remove(format.size() - 3, 2);
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save plot image."),
                                                    QDir::currentPath(),
                                                    format,
                                                    &selected);
    if (fileName.isNull())
        return;
    int astIdx = selected.lastIndexOf("*");
    selected.remove(0, astIdx + 2);
    selected.remove(selected.size() - 1, 1);
    QString perpName = fileName + ".perp." + selected;
    QString distName = fileName + ".dist." + selected;
    QImage image(perplexityPlot->size(), QImage::Format_RGB32);
    QImageWriter imageWriter;
    imageWriter.setFileName(perpName);
    imageWriter.setFormat(selected.toLocal8Bit());
    perplexityPlot->render(&image);
    imageWriter.write(image);
    image = QImage(distancePlot->size(), QImage::Format_RGB32);
    imageWriter.setFileName(distName);
    distancePlot->render(&image);
    imageWriter.write(image);
}

void MainWindow::SavePlotVector()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save plot image in vector format (*.svg)."),
                                                    QDir::currentPath());
    if (fileName.isNull())
        return;
    QwtPlotRenderer renderer;
    renderer.renderDocument(perplexityPlot, fileName + "_perp.svg", "svg", QSize(110, 115), 150);
    renderer.renderDocument(distancePlot, fileName + "_dist.svg", "svg", QSize(110, 115), 150);
}

void MainWindow::ClearPlot()
{
    perplexityPlot->clearFocus();
    distancePlot->clearFocus();
    for (int i = 0; i < nExperiments; ++i) {
        perplexityCurves[i]->hide();
        distanceCurves[i]->hide();
    }
    perplexityPlot->replot();
    distancePlot->replot();
}

void MainWindow::setControls(bool enable)
{
    running = !enable;
    /*
    ui->actionLoad->setEnabled(enable);
    ui->actionSave->setEnabled(enable);
    ui->estButton->setEnabled(enable);
    ui->infButton->setEnabled(enable);
    ui->genButton->setEnabled(enable);
    */
}

void MainWindow::progressCallback(int percent, MainWindow *toThis, int experiment)
{
    QProgressEvent *event = new QProgressEvent;
    event->percent = percent;
    event->experiment = experiment;
    QCoreApplication::postEvent(toThis, event);
}
/*
void MainWindow::messageCallback(string message, MainWindow *toThis)
{
    QMessageEvent *event = new QMessageEvent;
    event->message = message;
    QCoreApplication::postEvent(toThis, event);
}
*/
void MainWindow::perplexityCallback(double perplexity, int iter, MainWindow *toThis, int experiment)
{
    QPerplexityEvent *event = new QPerplexityEvent;
    event->perplexity = perplexity;
    event->iter = iter;
    event->experiment = experiment;
    QCoreApplication::postEvent(toThis, event);
}

void MainWindow::distanceCallback(double distance, int iter, MainWindow *toThis, int experiment)
{
    QDistanceEvent *event = new QDistanceEvent;
    event->distance = distance;
    event->iter = iter;
    event->experiment = experiment;
    QCoreApplication::postEvent(toThis, event);
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == (QEvent::Type)(QEvent::User + 1))
    {
        // Progress event
        QProgressEvent *pEvent = (QProgressEvent *)event;
        ui->progressBar->setValue(pEvent->percent);

        if (ui->progressBar->value() >= 100)
            setControls(true);
/*
        QString message;
        do {
            message = log->readLine();
            if (!message.isNull())
                ui->statusBar->showMessage(message);
        } while(!message.isNull());
*/
        /*} else if (event->type() == (QEvent::Type)(QEvent::User + 2)) {
        // Message event
        QMessageEvent *mEvent = (QMessageEvent *)event;
        ui->statusBar->showMessage(QString(mEvent->message.c_str()));*/
    } else if (event->type() == (QEvent::Type)(QEvent::User + 3)) {
        // Perplexity event
        QPerplexityEvent *pEvent = (QPerplexityEvent *)event;
        lastIter[pEvent->experiment] += 1;
        perplexity[pEvent->experiment][lastIter[pEvent->experiment]] = pEvent->perplexity;
        perplexityAvr[lastIter[pEvent->experiment]] += pEvent->perplexity / nExperiments;
        iters[lastIter[pEvent->experiment]] = pEvent->iter;
        perplexityCurves[pEvent->experiment]->setRawSamples(iters,
                                                            perplexity[pEvent->experiment], lastIter[pEvent->experiment] + 1);
        int minIter = lastIter[pEvent->experiment];
        for (int i = 0; i < nExperiments; ++i) {
            if (lastIter[i] < minIter) {
                minIter = lastIter[i];
            }
        }
        //perplexityAvrCurve->setRawSamples(iters, perplexityAvr, minIter + 1);
        perplexityPlot->replot();
    } else if (event->type() == (QEvent::Type)(QEvent::User + 4)) {
        // Distance event
        QDistanceEvent *pEvent = (QDistanceEvent *)event;
        distance[pEvent->experiment][lastIter[pEvent->experiment]] = pEvent->distance;
        distanceAvr[lastIter[pEvent->experiment]] += pEvent->distance / nExperiments;
        distanceCurves[pEvent->experiment]->setRawSamples(iters,
                                                          distance[pEvent->experiment], lastIter[pEvent->experiment] + 1);
        int minIter = lastIter[pEvent->experiment];
        for (int i = 0; i < nExperiments; ++i) {
            if (lastIter[i] < minIter) {
                minIter = lastIter[i];
            }
        }
        //distanceAvrCurve->setRawSamples(iters, distanceAvr, minIter);
        distancePlot->replot();
    }
    return QMainWindow::event(event);;
}

void MainWindow::ThreadFinish()
{
    int min = lastIter[0];
    for (int i = 1; i < nExperiments; ++i) {
        if (lastIter[i] < min) {
            min = lastIter[i];
        }
    }
    if (min == lastIter[0]) {
        setControls(true);
        int minIdx = 0;
        double minPerp = perplexity[0][lastIter[0]];
        for (int j = 1; j < nExperiments; ++j) {
            if (perplexity[j][lastIter[j]] < minPerp) {
                minPerp = perplexity[j][lastIter[j]];
                minIdx = j;
            }
        }
        perplexityCurves[minIdx]->setPen(Qt::red, 3);
        perplexityCurves[minIdx]->detach();
        perplexityCurves[minIdx]->attach(perplexityPlot);
        perplexityPlot->replot();
        distanceCurves[minIdx]->setPen(Qt::red, 3);
        distanceCurves[minIdx]->detach();
        distanceCurves[minIdx]->attach(distancePlot);
        distancePlot->replot();
    }
}

void MainWindow::UseHungarian(int enable)
{
    ui->gammaBox->setVisible(enable);
    ui->gammaLabel->setVisible(enable);
    ui->phiPathButton->setVisible(enable);
    ui->phiPathEdit->setVisible(enable);
    ui->phiPathLabel->setVisible(enable);
    ui->thetaPathButton->setVisible(enable);
    ui->thetaPathEdit->setVisible(enable);
    ui->thetaPathLabel->setVisible(enable);
    ui->realTopicBox->setVisible(enable);
    ui->realTopicsLabel->setVisible(enable);
}

void MainWindow::DirichletParams(int enable)
{
    ui->alphaBox->setVisible(enable);
    ui->betaBox->setVisible(enable);
    ui->alphaLabel->setVisible(enable);
    ui->betaLabel->setVisible(enable);
}

void MainWindow::UseRobast(int enable)
{
    ui->phiRobBox->setVisible(enable);
    ui->phiRobLabel->setVisible(enable);
    ui->thetaRobBox->setVisible(enable);
    ui->thetaRobLabel->setVisible(enable);
    ui->iterRobBox->setVisible(enable);
    ui->iterRobLabel->setVisible(enable);
}
