#ifndef RESTAPI_H
#define RESTAPI_H

#include <QApplication>
#include <QCheckBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QObject>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>
#include <QUrlQuery>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

class RestApi : public QObject
{
    Q_OBJECT
public:
    explicit RestApi(QObject *parent = nullptr);

public slots:
    void activateChanges(QStringList names,
                         QStringList ips,
                         QStringList cmds,
                         QList<QVariant> values);
    void cancelOperation() { cancelRequested = true; }

private:
    QSettings *settings;
    QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager;
    QNetworkReply *networkReply;
    QTimer *tmNetworkTimeout = new QTimer;
    QList<QNetworkRequest> requests;
    QJsonDocument jsonCmds;
    QStringList stnNames; // For log purposes
    bool cancelRequested = false;

private slots:
    void convertCmdsToJson(QStringList cmds, QList<QVariant> vals);
    void createNetworkRequest(const QString ip);
    void networkManagerReplyHandler(QNetworkReply *reply);
    void sendRequests();
    void tmNetworkTimeoutHandler();
    void cleanLists();
    void cancelRequests();

signals:
    void toLog(QString);
    void toStatus(QString);
};

#endif // RESTAPI_H
