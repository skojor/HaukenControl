#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    settings = new QSettings(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                                 + "/haukenremote.ini",
                             QSettings::IniFormat);

    setupWindow();
    setupSignals();
    updParams();
    createCmdTable();

    tmUpdRecordTime->start(1000);
}

MainWindow::~MainWindow()
{
}

void MainWindow::updateStationTable(QStringList stns, QStringList ips, QStringList types)
{
    tblStations->clear();
    stnSelectorBoxList.clear();
    tblStations->setRowCount(stns.size());
    tblStations->setColumnCount(3);
    tblStations->setHorizontalHeaderLabels(QStringList() << "" << "Name" << "IP");
    tblStations->verticalHeader()->hide();
    tblStations->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tblStations->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tblStations->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    tblStations->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    for (int i = 0; i < stns.size(); i++) {
        QTableWidgetItem *stn = new QTableWidgetItem(stns[i]);
        QTableWidgetItem *ip = new QTableWidgetItem(ips[i]);
        QCheckBox *cb = new QCheckBox;
        if (ips[i].isEmpty())
            cb->setCheckable(false);
        stnSelectorBoxList.append(cb);
        tblStations->setCellWidget(i, 0, cb);
        tblStations->setItem(i, 1, stn);
        tblStations->setItem(i, 2, ip);
    }
}

void MainWindow::createCmdTable()
{
    if (!paramsSelectorBoxList.isEmpty()) {
        for (auto &&cb : paramsSelectorBoxList)
            delete cb;
    }
    glParams->setColumnStretch(0, 1);
    glParams->setColumnStretch(1, 5);
    glParams->setColumnStretch(2, 8);
    QStringList params;
    params << "PScan start frequency (MHz)"
           << "PScan stop frequency (MHz)"
           << "PScan resolution (kHz)"
           << "Measurement time (ms)"
           << "Manual attenuator (dB)"
           << "Auto attenuator"
           << "Antenna port"
           << "Mode"
           << "FFT mode"
           << "Gain control";
    for (int i = 0; i < params.size(); i++) {
        QCheckBox *cb = new QCheckBox;
        glParams->addWidget(cb, i, 0);
        paramsSelectorBoxList.append(cb);
        glParams->addWidget(new QLabel(params[i]), i, 1);
    }
    glParams->addWidget(spbPscanStart, 0, 2);
    glParams->addWidget(spbPscanStop, 1, 2);
    glParams->addWidget(cbPscanRes, 2, 2);
    glParams->addWidget(spbMeasTime, 3, 2);
    glParams->addWidget(spbmanAtt, 4, 2);
    glParams->addWidget(cbAutoAtt, 5, 2);
    glParams->addWidget(cbAntPort, 6, 2);
    //glParams->addWidget(cbMode, 7, 2);
    paramsSelectorBoxList[7]->setCheckable(false);
    glParams->addWidget(cbFftMode, 8, 2);
    glParams->addWidget(cbGain, 9, 2);

    glParams->addWidget(cbstart, 10, 0);
    paramsSelectorBoxList.append(cbstart);
    glParams->addWidget(cbstop, 11, 0);
    paramsSelectorBoxList.append(cbstop);

    connect(cbstart, &QCheckBox::clicked, this, [this]() {
        if (cbstart->isChecked())
            cbstop->setChecked(true);
        else
            cbstop->setChecked(false);
    });
    connect(cbstop, &QCheckBox::clicked, this, [this]() {
        if (cbstop->isChecked())
            cbstart->setChecked(true);
        else
            cbstart->setChecked(false);
    });

    glParams->addWidget(new QLabel("Start recording at"), 10, 1);
    glParams->addWidget(new QLabel("End recording at"), 11, 1);
    glParams->addWidget(dteDateTimeBegin, 10, 2);
    glParams->addWidget(dteDateTimeEnd, 11, 2);
    dteDateTimeBegin->setMinimumDateTime(QDateTime::currentDateTime());
    dteDateTimeEnd->setMinimumDateTime(QDateTime::currentDateTime());
    dteDateTimeEnd->setDateTime(QDateTime::currentDateTime().addSecs(3600));

    QCheckBox *cb = new QCheckBox;
    glParams->addWidget(cb, 12, 0);
    glParams->addWidget(new QLabel("Restart Hauken"));
    glParams->addWidget(new QLabel("")); // Blank widget
    paramsSelectorBoxList.append(cb);

    QCheckBox *cb2 = new QCheckBox;
    glParams->addWidget(cb2, 13, 0);
    glParams->addWidget(new QLabel("Change setting"));
    glParams->addWidget(leChangeSetting);
    paramsSelectorBoxList.append(cb2);

    glParams->addWidget(lblRecordingTime, 14, 1, 1, 2);
    lblRecordingTime->setText(
        convertSecsToHourMinSecs(dteDateTimeBegin->dateTime().secsTo(dteDateTimeEnd->dateTime())));
}

void MainWindow::updParams()
{
    spbPscanStart->setRange(0, 9999);
    spbPscanStop->setRange(0, 9999);
    cbPscanRes->addItems(QStringList()
                         << "0.1" << "0.125" << "0.2" << "0.250" << "0.5" << "0.625" << "1"
                         << "1.25" << "2" << "2.5" << "3.125" << "5" << "6.25" << "10" << "12.5"
                         << "20" << "25" << "50" << "100" << "200" << "500" << "1000" << "2000");
    spbMeasTime->setRange(1, 1e4);
    cbAutoAtt->addItems(QStringList() << "On" << "Off");
    cbAntPort->addItems(QStringList() << "Ant. 1" << "Ant. 2");
    cbMode->addItems(QStringList() << "PScan" << "FFM");
    cbFftMode->addItems(QStringList() << "Off" << "Max" << "Min" << "Scalar" << "APeak");
    cbGain->addItems(QStringList() << "Low distortion" << "Normal" << "Low noise");

    spbPscanStart->setValue(settings->value("pscanStart", 1560).toDouble());
    spbPscanStop->setValue(settings->value("pscanStop", 1650).toDouble());
    cbPscanRes->setCurrentIndex(settings->value("pscanRes", 12).toInt());
    spbMeasTime->setValue(settings->value("measTime", 18).toInt());
    spbmanAtt->setValue(settings->value("manAtt", 0).toInt());
    cbAutoAtt->setCurrentIndex(settings->value("autoAtt", 0).toInt());
    cbAntPort->setCurrentIndex(settings->value("antPort", 0).toInt());
    cbMode->setCurrentIndex(settings->value("mode", 0).toInt());
    cbFftMode->setCurrentIndex(settings->value("fftMode", 1).toInt());
    cbGain->setCurrentIndex(settings->value("gain", 1).toInt());
    leChangeSetting->setText(settings->value("chSetting").toString());
}

void MainWindow::activateChanges()
{
    QStringList ips, names, cmds;
    QList<QVariant> values;

    for (int i = 0; i < stnSelectorBoxList.size(); i++) {
        if (stnSelectorBoxList[i]->isChecked()) {
            names.append(tblStations->item(i, 1)->text());
            ips.append(tblStations->item(i, 2)->text());
        }
    }
    for (int i = 0; i < paramsSelectorBoxList.size(); i++) {
        if (paramsSelectorBoxList[i]->isChecked()) {
            cmds.append(qobject_cast<QLabel *>(glParams->itemAtPosition(i, 1)->widget())->text());
            auto wid = glParams->itemAtPosition(i, 2)->widget();
            if (QSpinBox *spb = qobject_cast<QSpinBox *>(wid)) {
                values.append(spb->value());
            } else if (QComboBox *cb = qobject_cast<QComboBox *>(wid)) {
                values.append(cb->currentText());
            } else if (QLabel *lab = qobject_cast<QLabel *>(wid)) {
                if (lab->text().isEmpty())
                    values.append(1); // Restart, dummy value
                else
                    values.append(lab->text());
            } else if (QLineEdit *le = qobject_cast<QLineEdit *>(wid)) {
                values.append(le->text());
            } else if (QDateTimeEdit *dte = qobject_cast<QDateTimeEdit *>(wid)) {
                values.append(dte->dateTime());
            }
        }
    }
    restApi->activateChanges(names, ips, cmds, values);
}

void MainWindow::cancelChanges()
{
    restApi->cancelOperation();
}

QString MainWindow::convertSecsToHourMinSecs(int secs)
{
    if (secs < 0)
        secs = 0;
    QString strTime;
    QTextStream ts(&strTime);
    int mins = secs / 60;
    int hours = mins / 60;
    int days = hours / 24;

    ts << "Record time: " << days << " days " << hours % 24 << " hours " << mins % 60 << " mins ";
    return strTime;
}

void MainWindow::updStatusBar(QString s)
{
    statusBar->showMessage(s);
}

void MainWindow::setupWindow()
{
    setCentralWidget(centralWidget);

    gridLayout->addLayout(vlStnName, 0, 0);
    gridLayout->addLayout(glParams, 0, 1);
    gridLayout->addLayout(glBtns, 1, 0);
    gridLayout->addWidget(teInfoBox, 1, 1);
    vlStnName->addWidget(tblStations);

    glBtns->addWidget(btnSelAll, 0, 0);
    glBtns->addWidget(btnSelNone, 0, 1);
    glBtns->addWidget(btnUpdateList, 1, 0, 1, 2);
    glBtns->addWidget(btnActivateChanges, 2, 0, 1, 2);
    glBtns->addWidget(btnCancelOperation, 3, 0, 1, 2);

    btnSelAll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnSelNone->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnUpdateList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnActivateChanges->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnCancelOperation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    centralWidget->setLayout(gridLayout);
    centralWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    this->resize(800, 600);
    this->setStatusBar(statusBar);

    centralWidget->show();
}

void MainWindow::setupSignals()
{
    connect(stationList, &StationList::listReady, this, &MainWindow::updateStationTable);
    connect(btnSelAll, &QPushButton::clicked, this, [this]() {
        for (auto &&cb : stnSelectorBoxList)
            cb->setChecked(true);
    });
    connect(btnSelNone, &QPushButton::clicked, this, [this]() {
        for (auto &&cb : stnSelectorBoxList)
            cb->setChecked(false);
    });
    connect(btnUpdateList, &QPushButton::clicked, this, [this]() {
        stationList->fetchDataHandler();
    });
    connect(btnActivateChanges, &QPushButton::clicked, this, &MainWindow::activateChanges);
    connect(btnCancelOperation, &QPushButton::clicked, this, &MainWindow::cancelChanges);

    connect(spbPscanStart, &QSpinBox::valueChanged, this, [this](int i) {
        settings->setValue("pscanStart", i);
    });
    connect(spbPscanStop, &QSpinBox::valueChanged, this, [this](int i) {
        settings->setValue("pscanStop", i);
    });
    connect(cbPscanRes, &QComboBox::currentIndexChanged, this, [this](int i) {
        settings->setValue("pscanRes", i);
    });
    connect(spbMeasTime, &QSpinBox::valueChanged, this, [this](int i) {
        settings->setValue("measTime", i);
    });
    connect(spbmanAtt, &QSpinBox::valueChanged, this, [this](int i) {
        settings->setValue("manAtt", i);
    });
    connect(cbAutoAtt, &QComboBox::currentIndexChanged, this, [this](int i) {
        settings->setValue("autoAtt", i);
    });
    connect(cbAntPort, &QComboBox::currentIndexChanged, this, [this](int i) {
        settings->setValue("antPort", i);
    });
    connect(cbMode, &QComboBox::currentIndexChanged, this, [this](int i) {
        settings->setValue("mode", i);
    });
    connect(cbFftMode, &QComboBox::currentIndexChanged, this, [this](int i) {
        settings->setValue("fftMode", i);
    });
    connect(cbGain, &QComboBox::currentIndexChanged, this, [this](int i) {
        settings->setValue("gain", i);
    });
    connect(leChangeSetting, &QLineEdit::textEdited, this, [this](QString s) {
        settings->setValue("chSetting", s);
    });
    connect(dteDateTimeBegin, &QDateTimeEdit::dateTimeChanged, this, [this](QDateTime dt) {
        lblRecordingTime->setText(convertSecsToHourMinSecs(
            dteDateTimeBegin->dateTime().secsTo(dteDateTimeEnd->dateTime())));
    });
    connect(dteDateTimeEnd, &QDateTimeEdit::dateTimeChanged, this, [this](QDateTime dt) {
        lblRecordingTime->setText(convertSecsToHourMinSecs(
            dteDateTimeBegin->dateTime().secsTo(dteDateTimeEnd->dateTime())));
    });
    connect(tmUpdRecordTime, &QTimer::timeout, this, [this]() {
        lblRecordingTime->setText(convertSecsToHourMinSecs(
            dteDateTimeBegin->dateTime().secsTo(dteDateTimeEnd->dateTime())));
        if (dteDateTimeBegin->dateTime().secsTo(QDateTime::currentDateTime()) > 59) {
            dteDateTimeBegin->setDateTime(QDateTime::currentDateTime());
            dteDateTimeEnd->setDateTime(dteDateTimeEnd->dateTime().addSecs(
                60)); // TODO: Will this work? Should add 1 minute for every minute program running, if begin time is now
        }
    });
    connect(stationList, &StationList::toLog, this, [this](QString s) {
        teInfoBox->append(QDateTime::currentDateTime().toString("hh:mm:ss: ") + s);
    });
    connect(restApi, &RestApi::toLog, this, [this](QString s) {
        teInfoBox->append(QDateTime::currentDateTime().toString("hh:mm:ss: ") + s);
    });
    connect(stationList, &StationList::toStatus, this, &MainWindow::updStatusBar);
    connect(restApi, &RestApi::toStatus, this, &MainWindow::updStatusBar);
}
