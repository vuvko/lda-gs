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
    distancePlot = new QwtPlot(tr("Distance"));
    //perplexityPlot->insertLegend(new QwtLegend());
    perplexityPlot->setAxisTitle(QwtPlot::xBottom, "Iterations");
    distancePlot->setAxisTitle(QwtPlot::xBottom, "Iterations");
    //perplexityPlot->setAxisTitle(QwtPlot::yLeft, "Value"); // what does it say to you?
    perplexityPlot->setCanvasBackground(QBrush(Qt::white));
    distancePlot->setCanvasBackground(QBrush(Qt::white));
    //perplexityPlot->setAutoReplot(true); // use replot() instead
    //ui->mainLayout->addWidget(perplexityPlot, 0, 1, 2, 1);
    ui->perpLayout->addWidget(perplexityPlot);
    ui->distLayout->addWidget(distancePlot);
    perplexityCurve = new QwtPlotCurve(tr("Perplexity"));
    distanceCurve = new QwtPlotCurve(tr("Distance"));
    perplexityCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    distanceCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    perplexityCurve->setPen(Qt::red, 3);
    distanceCurve->setPen(Qt::green, 3);
    perplexityCurve->attach(perplexityPlot);
    distanceCurve->attach(distancePlot);
    perplexityCurve->hide();
    distanceCurve->hide();
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
    perplexity = 0;
    iters = 0;
    infThread = 0;
    estThread = 0;
    genThread = 0;
    running = false;
    connect(ui->actionLoad, SIGNAL(triggered()), this, SLOT(LoadModel()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(SaveModel()));
    connect(ui->actionSavePlot, SIGNAL(triggered()), this, SLOT(SavePlot()));
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
    /*
    lda->set_default_values();
    lda->progress_callback = (PCALLBACK)progressCallback;
    lda->perplexity_callback = (DCALLBACK)perplexityCallback;
    lda->distance_callback = (DCALLBACK)distanceCallback;
    lda->parent = this;
    */
    logFile = new QFile("lda.log");
    if (!logFile->open(QFile::ReadOnly))
        qDebug() << "error opening log file!";
    log = new QTextStream(logFile);

}

MainWindow::~MainWindow()
{
    delete ui;
    if (estThread)
        delete estThread;
    if (infThread)
        delete infThread;
    if (genThread)
        delete genThread;
    delete perplexityCurve;
    delete perplexityGrid;
    delete perplexityPlot;
    delete distanceCurve;
    delete distanceGrid;
    delete distancePlot;
    if (distance)
        delete distance;
    if (perplexity)
        delete perplexity;
    if (iters)
        delete iters;
    logFile->close();
    delete log;
    delete logFile;
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
    lda.set_default_values();
    char filter[255];
    sprintf(filter, "Model (*%s)", lda.others_suffix.c_str());
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
    model *lda = new model;
    lda->set_default_values();
    lda->progress_callback = (PCALLBACK)progressCallback;
    lda->perplexity_callback = (DCALLBACK)perplexityCallback;
    lda->distance_callback = (DCALLBACK)distanceCallback;
    lda->parent = this;
    lda->r_iter = ui->iterRobBox->value();
    lda->phi_perc = ui->phiRobBox->value();
    lda->theta_perc = ui->thetaRobBox->value();
    lda->model_status = MODEL_STATUS_EST;
    lda->K = ui->estTopicBox->value();
    if (ui->alphaBox->value() > eps)
        lda->alpha = ui->alphaBox->value();
    if (ui->betaBox->value() > eps)
        lda->beta = ui->betaBox->value();
    lda->niters = ui->estIterBox->value();
    lda->savestep = ui->estSaveBox->value();
    if (ui->estTwordBox->value() > 0)
        lda->twords = ui->estTwordBox->value();
    lda->dfile = ui->estDataEdit->text().toStdString();
    string::size_type idx = lda->dfile.find_last_of("/");
    if (idx == string::npos) {
        lda->dir = "./";
    } else {
        lda->dir = lda->dfile.substr(0, idx + 1);
        lda->dfile = lda->dfile.substr(idx + 1, lda->dfile.size() - lda->dir.size());
    }
    lda->piter = ui->estPerpBox->value();
    //lda->init_est();
    perplexityPlot->setAxisScale(QwtPlot::xBottom, 1, lda->niters);
    distancePlot->setAxisScale(QwtPlot::xBottom, 1, lda->niters);
    int size = (lda->niters + lda->piter - 1) / lda->piter;
    if (!iters)
        iters = (double *)calloc(size + 1, sizeof(*iters));
    else
        iters = (double *)realloc(iters, (size + 1) * sizeof(*iters));
    if (!perplexity)
        perplexity = (double *)calloc(size + 1, sizeof(*perplexity));
    else
        perplexity = (double *)realloc(perplexity, (size + 1) * sizeof(*perplexity));
    if (!distance)
        distance = (double *) calloc(size + 1, sizeof(*distance));
    else
        distance = (double *)realloc(distance, (size + 1) * sizeof(*distance));
    lastIter = -1;
    perplexityCurve->setRawSamples(iters, perplexity, 0);
    distanceCurve->setRawSamples(iters, distance, 0);
    perplexityCurve->show();
    QString phiPath = ui->phiPathEdit->text();
    QString thetaPath = ui->thetaPathEdit->text();
    int KReal = ui->realTopicBox->value();
    bool useHungarian = ui->hungarianBox->isChecked();
    if (useHungarian)
        distanceCurve->show();
    if (estThread)
        delete estThread;
    estThread = new EstimateThread(lda, phiPath, thetaPath, KReal, useHungarian);
    connect(estThread, SIGNAL(finished()), this, SLOT(ThreadFinish()));
    estThread->start();
}

void MainWindow::Inference()
{
    if (running)
        return;
    setControls(false);
    model *lda = new model;
    lda->set_default_values();
    lda->progress_callback = (PCALLBACK)progressCallback;
    lda->perplexity_callback = (DCALLBACK)perplexityCallback;
    lda->distance_callback = (DCALLBACK)distanceCallback;
    lda->parent = this;
    lda->model_status = MODEL_STATUS_INF;
    lda->niters = ui->infIterBox->value();
    if (ui->infTwordsBox->value() > 0)
        lda->twords = ui->infTwordsBox->value();
    lda->dfile = ui->infDataEdit->text().toStdString();
    string::size_type idx = lda->dfile.find_last_of("/");
    if (idx != string::npos) {
        lda->dfile = lda->dfile.substr(idx + 1, lda->dfile.size() - lda->dir.size());
    }
    string ldaParam = ui->infModelEdit->text().toStdString();
    idx = ldaParam.find_last_of("/");
    if (idx == string::npos) {
        lda->dir = "./";
    } else {
        lda->dir = ldaParam.substr(0, idx + 1);
    }
    utils::read_and_parse(ldaParam, lda);
    //lda->init_inf();
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
    lda->set_default_values();
    lda->progress_callback = (PCALLBACK)progressCallback;
    lda->perplexity_callback = (DCALLBACK)perplexityCallback;
    lda->distance_callback = (DCALLBACK)distanceCallback;
    lda->parent = this;
    lda->M = ui->genNdocBox->value();
    lda->K = ui->genNTopicBox->value();
    lda->V = ui->genNWordsBox->value();
    lda->alpha = ui->genAlphaBox->value();
    lda->beta = ui->genBetaBox->value();
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
    /*
    if (!fileName.endsWith("." + selected))
        fileName.append("." + selected);
    */
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

void MainWindow::ClearPlot()
{
    perplexityPlot->clearFocus();
    perplexityCurve->hide();
    perplexityPlot->replot();
}

void MainWindow::setControls(bool enable)
{
    /*
    running = !enable;
    ui->actionLoad->setEnabled(enable);
    ui->actionSave->setEnabled(enable);
    ui->estButton->setEnabled(enable);
    ui->infButton->setEnabled(enable);
    ui->genButton->setEnabled(enable);
    */
}

void MainWindow::progressCallback(int percent, MainWindow *toThis)
{
    QProgressEvent *event = new QProgressEvent;
    event->percent = percent;
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
void MainWindow::perplexityCallback(double perplexity, int iter, MainWindow *toThis)
{
    QPerplexityEvent *event = new QPerplexityEvent;
    event->perplexity = perplexity;
    event->iter = iter;
    QCoreApplication::postEvent(toThis, event);
}

void MainWindow::distanceCallback(double distance, int iter, MainWindow *toThis)
{
    QDistanceEvent *event = new QDistanceEvent;
    event->distance = distance;
    event->iter = iter;
    QCoreApplication::postEvent(toThis, event);
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == (QEvent::Type)(QEvent::User + 1))
    {
        // Progress event
        QProgressEvent *pEvent = (QProgressEvent *)event;
        ui->progressBar->setValue(pEvent->percent);
        if (pEvent->percent >= 100)
            setControls(true);
        QString message;
        do {
            message = log->readLine();
            if (!message.isNull())
                ui->statusBar->showMessage(message);
        } while(!message.isNull());
    /*} else if (event->type() == (QEvent::Type)(QEvent::User + 2)) {
        // Message event
        QMessageEvent *mEvent = (QMessageEvent *)event;
        ui->statusBar->showMessage(QString(mEvent->message.c_str()));*/
    } else if (event->type() == (QEvent::Type)(QEvent::User + 3)) {
        // Perplexity event
        QPerplexityEvent *pEvent = (QPerplexityEvent *)event;
        perplexity[++lastIter] = pEvent->perplexity;
        iters[lastIter] = pEvent->iter;
        perplexityCurve->setRawSamples(iters, perplexity, lastIter + 1);
        perplexityPlot->replot();
        qDebug() << pEvent->iter << pEvent->perplexity;
    } else if (event->type() == (QEvent::Type)(QEvent::User + 4)) {
        // Distance event
        QDistanceEvent *pEvent = (QDistanceEvent *)event;
        //double dist = pEvent->distance;
        //qDebug() << pEvent->iter << dist;
        distance[lastIter] = pEvent->distance;
        distanceCurve->setRawSamples(iters, distance, lastIter + 1);
        distancePlot->replot();
    }
    return QMainWindow::event(event);;
}

void MainWindow::ThreadFinish()
{
    setControls(true);
}

void MainWindow::UseHungarian(int enable)
{
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
