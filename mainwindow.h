#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDebug>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFormLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QStandardPaths>
#include <QStatusBar>
#include <QString>
#include <QStyleHints>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include "restapi.h"
#include "stationlist.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:

private slots:
    void setupWindow();
    void setupSignals();
    void updateStationTable(QStringList stns, QStringList ips, QStringList types);
    void createCmdTable();
    void updParams();
    void activateChanges();
    void cancelChanges();
    QString convertSecsToHourMinSecs(int secs);
    void updStatusBar(QString);

private:
    //Ui::MainWindow *ui;
    QWidget *centralWidget = new QWidget;
    QGridLayout *gridLayout = new QGridLayout;
    QGridLayout *glBtns = new QGridLayout;
    QVBoxLayout *vlStnName = new QVBoxLayout;
    QGridLayout *glParams = new QGridLayout;
    QTextEdit *teInfoBox = new QTextEdit;
    QList<QCheckBox *> stnSelectorBoxList;
    QList<QCheckBox *> paramsSelectorBoxList;
    QList<QString> stnNameList;
    QTableWidget *tblStations = new QTableWidget;
    StationList *stationList = new StationList;
    RestApi *restApi = new RestApi;

    QSpinBox *spbPscanStart = new QSpinBox;
    QSpinBox *spbPscanStop = new QSpinBox;
    QComboBox *cbPscanRes = new QComboBox;
    QSpinBox *spbMeasTime = new QSpinBox;
    QSpinBox *spbmanAtt = new QSpinBox;
    QComboBox *cbAutoAtt = new QComboBox;
    QComboBox *cbAntPort = new QComboBox;
    QComboBox *cbMode = new QComboBox;
    QComboBox *cbFftMode = new QComboBox;
    QComboBox *cbGain = new QComboBox;
    QLineEdit *leChangeSetting = new QLineEdit;

    QPushButton *btnUpdateList = new QPushButton("Update station list from server");
    QPushButton *btnSelAll = new QPushButton("Select all");
    QPushButton *btnSelNone = new QPushButton("Select none");
    QPushButton *btnActivateChanges = new QPushButton("Send chosen commands to selected stations");
    QPushButton *btnCancelOperation = new QPushButton("Cancel current operation");

    QDateTimeEdit *dteDateTimeBegin = new QDateTimeEdit;
    QDateTimeEdit *dteDateTimeEnd = new QDateTimeEdit;
    QLabel *lblRecordingTime = new QLabel;

    QSettings *settings;
    QTimer *tmUpdRecordTime = new QTimer;
    QCheckBox *cbstart = new QCheckBox;
    QCheckBox *cbstop = new QCheckBox;
    QStatusBar *statusBar = new QStatusBar;
};
#endif // MAINWINDOW_H
