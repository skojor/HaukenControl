#ifndef STATIONLIST_H
#define STATIONLIST_H

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPair>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <QUrlQuery>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkCookie>
#include <QtNetwork/QNetworkCookieJar>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

enum class StationType { UNKNOWN, STATIONARY, OFFSHORE, SHIP, MOBILE, PORTABLE };

enum class InstrumentCategory { UNKNOWN, RECEIVER, PC };

class EquipmentInfo
{
public:
    EquipmentInfo() {}
    int index;
    QString type;
};

class InstrumentInfo
{
public:
    InstrumentInfo() {}
    unsigned char index = 0;
    InstrumentCategory category = InstrumentCategory::UNKNOWN;
    QString producer;
    QString type;
    QString frequencyRange;
    QString serialNumber;
    QString ipAddress;
    unsigned char equipmentIndex = 0;
};

class StationInfo
{
public:
    StationInfo() {}
    unsigned int StationIndex = 0;
    QString name;
    QString officialName;
    QString address;
    double latitude = 0;
    double longitude = 0;
    QString status;
    StationType type = StationType::UNKNOWN;
    unsigned long mmsi = 0;
    bool active = false;
    QList<InstrumentInfo> instrumentInfo;
};

enum StateDataHandler {
    BEGIN,
    ASKEDFORLOGIN,
    LOGGEDIN,
    ASKEDFORSTATIONS,
    RECEIVEDSTATIONS,
    ASKEDFORINSTRUMENTS,
    RECEIVEDINSTRUMENTS,
    ASKEDFOREQUIPMENT,
    RECEIVEDEQUIPMENT,
    FAILED,
    DONE
};

class StationList : public QObject
{
    Q_OBJECT
public:
    StationList();

public slots:
    void fetchDataHandler(const QByteArray &reply = QByteArray());

private slots:
    void loadFile();
    void saveFile();
    void parseStationList(const QByteArray &reply);
    void networkAccessManagerFinished(QNetworkReply *reply);
    void tmNetworkTimeoutHandler();
    void loginRequest();
    void parseLoginReply(const QByteArray &reply);
    void stationListRequest();
    void instrumentListRequest();
    void parseInstrumentList(const QByteArray &reply);
    void equipmentListRequest();
    void parseEquipmentList(const QByteArray &reply);
    void updStationsWithEquipmentList();
    void generateLists();

private:
    QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager;
    QNetworkReply *networkReply;
    QTimer *tmNetworkTimeout = new QTimer;
    QSettings *settings;
    QNetworkCookie networkCookie;
    QNetworkCookieJar *cookieJar = new QNetworkCookieJar;
    QList<StationInfo> stationInfo;
    QList<EquipmentInfo> equipmentInfo;
    //int nrOfStations = 0;
    int stationIndex = 0;
    StateDataHandler state = BEGIN;
    QStringList usableStnNames, usableStnIps, usableStnTypes;

signals:
    void listReady(QStringList, QStringList, QStringList);
    void toLog(QString);
    void toStatus(QString);
};

#endif // STATIONLIST_H
