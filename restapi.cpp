#include "restapi.h"

RestApi::RestApi(QObject *parent)
    : QObject{parent}
{
    settings = new QSettings(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                                 + "/haukenremote.ini",
                             QSettings::IniFormat);

    connect(networkAccessManager,
            &QNetworkAccessManager::finished,
            this,
            &RestApi::networkManagerReplyHandler);
    connect(tmNetworkTimeout, &QTimer::timeout, this, &RestApi::tmNetworkTimeoutHandler);
    tmNetworkTimeout->setSingleShot(true);
}

void RestApi::activateChanges(QStringList names,
                              QStringList ips,
                              QStringList cmds,
                              QList<QVariant> values)
{
    qDebug() << names << ips << cmds << values;

    if (cmds.isEmpty() || ips.isEmpty()) {
        qDebug() << "Trying to do nothing to someone or something to noone, giving up";
        emit toLog("No commands and/or stations selected, doing nothing");
    } else {
        convertCmdsToJson(cmds, values);

        stnNames = names;
        for (auto &&ip : ips)
            createNetworkRequest(ip);
        sendRequests();
    }
}

void RestApi::convertCmdsToJson(QStringList cmds, QList<QVariant> vals)
{
    QJsonObject obj;
    QStringList::iterator cmd = cmds.begin();
    QList<QVariant>::iterator val = vals.begin();

    while (cmd != cmds.end() && val != vals.end()) {
        qDebug() << *cmd << *val;
        if (cmd->contains("pscan start", Qt::CaseInsensitive))
            obj.insert("pscanStartFreq", val->toDouble());
        else if (cmd->contains("pscan stop", Qt::CaseInsensitive))
            obj.insert("pscanStopFreq", val->toDouble());
        else if (cmd->contains("pscan res", Qt::CaseInsensitive))
            obj.insert("pscanResolution", val->toString());
        else if (cmd->contains("measurement time", Qt::CaseInsensitive))
            obj.insert("measurementtime", val->toDouble());
        else if (cmd->contains("manual att", Qt::CaseInsensitive))
            obj.insert("manualatt", val->toInt());
        else if (cmd->contains("auto att", Qt::CaseInsensitive))
            obj.insert("autoatt", val->toBool());
        else if (cmd->contains("antenna port", Qt::CaseInsensitive))
            obj.insert("autoatt", (val->toString() == "On" ? true : false));
        else if (cmd->contains("fft mode", Qt::CaseInsensitive))
            obj.insert("fftmode", val->toString());
        else if (cmd->contains("gain control", Qt::CaseInsensitive))
            obj.insert("gainControl", val->toString());
        else if (cmd->contains("start recording at", Qt::CaseInsensitive))
            obj.insert("startRecording", val->toDateTime().toString());
        else if (cmd->contains("end recording at", Qt::CaseInsensitive))
            obj.insert("stopRecording", val->toDateTime().toString());
        else if (cmd->contains("restart", Qt::CaseInsensitive))
            obj.insert("restart", true);
        else if (cmd->contains("change setting", Qt::CaseInsensitive))
            obj.insert("settings", val->toString());

        cmd++;
        val++;
    }

    jsonCmds.setObject(obj);

    //qDebug() << jsonCmds.toJson();
}

void RestApi::createNetworkRequest(const QString ip)
{
    QNetworkRequest networkRequest;
    networkRequest.setUrl(QUrl("http://" + ip + "/api"));
    networkRequest.setRawHeader(settings->value("restKey").toByteArray(),
                                settings->value("restSecret").toByteArray());
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    requests.append(networkRequest);
}

void RestApi::sendRequests()
{
    if (cancelRequested) {
        cancelRequested = false;
        tmNetworkTimeout->stop();
        cancelRequests();
    }

    if (!requests.isEmpty()) {
        emit toStatus("Sending request to " + stnNames.first());
        if (!tmNetworkTimeout->isActive())
            tmNetworkTimeout->start(5000);
        networkAccessManager->post(requests.first(), jsonCmds.toJson(QJsonDocument::Compact));
    }
}

void RestApi::networkManagerReplyHandler(QNetworkReply *reply)
{
    tmNetworkTimeout->stop();

    if (!stnNames.isEmpty() && !requests.isEmpty()) {
        if (reply->error() == QNetworkReply::NoError
            && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                   == 202) { // Look for ret.code 202 - accepted
            emit toLog("Successfully sent command request to " + stnNames.first());
        } else {
            emit toLog("Error while sending command request to " + stnNames.first() + ": "
                       + reply->errorString());
        }
        cleanLists();
    }
    if (!requests.isEmpty() && !stnNames.isEmpty()) // Repeat as needed
        sendRequests();
}

void RestApi::tmNetworkTimeoutHandler()
{
    tmNetworkTimeout->stop();
    qDebug() << "RestApi network timeout";
    emit toLog("Network request for " + stnNames.first() + " timed out");
    cleanLists();
    if (!requests.isEmpty()) // Repeat as needed
        sendRequests();
    else
        emit toStatus("Done");
}

void RestApi::cleanLists()
{
    if (stnNames.size() != requests.size()) {
        qDebug() << "Out of sync, aborting requests";
        stnNames.clear();
        requests.clear();
    } else if (stnNames.size() > 0 && requests.size() > 0) {
        stnNames.removeFirst();
        requests.removeFirst();
    }
}

void RestApi::cancelRequests()
{
    if (!stnNames.isEmpty())
        stnNames.clear();
    if (!requests.isEmpty())
        requests.clear();
    emit toLog("Cancelled requests");
    emit toStatus("");
}
