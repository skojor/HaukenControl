#include "stationlist.h"

StationList::StationList()
{
    settings = new QSettings(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                                 + "/haukenremote.ini",
                             QSettings::IniFormat);
    qDebug() << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    connect(tmNetworkTimeout, &QTimer::timeout, this, &StationList::tmNetworkTimeoutHandler);
    connect(networkAccessManager,
            &QNetworkAccessManager::finished,
            this,
            &StationList::networkAccessManagerFinished);

    networkAccessManager->setCookieJar(cookieJar);
    QTimer::singleShot(100, this, [this]() { loadFile(); });
}

void StationList::loadFile()
{
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
               + "/stationdata.csv");
    //QTextStream ts(&file);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Couldn't open station data file" << file.fileName();
    } else {
        while (!file.atEnd()) {
            QString str = file.readLine();
            QStringList split = str.split(";");
            if (split.size() == 3) {
                usableStnIps.append(split[0].trimmed());
                usableStnNames.append(split[1].trimmed());
                usableStnTypes.append(split[2].trimmed());
            }
        }
        qDebug() << "Loaded station data from file" << file.fileName();
        emit toLog("Loaded station data from file " + file.fileName());
    }
    file.close();
    emit listReady(usableStnNames, usableStnIps, usableStnTypes);
}

void StationList::saveFile()
{
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
               + "/stationdata.csv");
    QTextStream ts(&file);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Couldn't save station data to file" << file.fileName();
    } else {
        for (int i = 0; i < usableStnIps.size(); i++) {
            ts << usableStnIps[i] << ";" << usableStnNames[i] << ";" << usableStnTypes[i]
               << Qt::endl;
        }
        qDebug() << "Saved station data to" << file.fileName();
    }
    file.close();
}

void StationList::tmNetworkTimeoutHandler()
{
    qWarning() << "Network request timed out";
    emit toLog("Station list: Network request timed out");
    state = FAILED;
}

void StationList::networkAccessManagerFinished(QNetworkReply *reply)
{
    tmNetworkTimeout->stop();
    fetchDataHandler(reply->readAll());
}

void StationList::parseLoginReply(const QByteArray &reply)
{
    if (!networkAccessManager->cookieJar()
             ->cookiesForUrl(settings->value("stationListUrl").toUrl())
             .isEmpty()) {
        qDebug() << "Cookie received, proceeding";
        state = LOGGEDIN;
    } else {
        qWarning() << "Couldn't login, aborting";
        emit toLog("Station list: Login failed");
        state = FAILED;
    }
    fetchDataHandler();
}

void StationList::loginRequest()
{
    QNetworkRequest networkRequest;
    networkRequest.setUrl(settings->value("authUrl").toUrl());
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                             "application/x-www-form-urlencoded");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("brukernavn", settings->value("user").toByteArray());
    urlQuery.addQueryItem("passord", settings->value("pw").toByteArray());
    networkReply = networkAccessManager->post(networkRequest,
                                              urlQuery.toString(QUrl::FullyEncoded).toUtf8());
    tmNetworkTimeout->start(10000);
}

void StationList::stationListRequest()
{
    QNetworkRequest networkRequest;
    networkRequest.setUrl(settings->value("stationListUrl").toUrl());
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                             "application/x-www-form-urlencoded");

    networkReply = networkAccessManager->get(networkRequest);
    tmNetworkTimeout->start(10000);
    state = ASKEDFORSTATIONS;
}

void StationList::parseStationList(const QByteArray &reply)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply);
    QJsonObject jsonObject = jsonDoc.object();
    QJsonObject metaObject = jsonObject.value("metadata").toObject();

    int nrOfElements = metaObject.value("totaltAntallTreff").toInt(0);
    if (nrOfElements > 0) {
        //nrOfStations = nrOfElements;
        QJsonArray data = jsonObject.value("data").toArray();
        if (data.size() == nrOfElements) { // Super simple fault check, improve! TODO
            for (auto &&array : data) {
                StationInfo stn;
                stn.StationIndex = array.toObject().value("SInd").toString().toInt();
                stn.name = array.toObject().value("Navn").toString();
                stn.officialName = array.toObject().value("Off_navn").toString();
                stn.address = array.toObject().value("Adresse").toString();
                stn.latitude = array.toObject().value("Latitude").toString().toDouble();
                stn.longitude = array.toObject().value("Longitude").toString().toDouble();
                stn.status = array.toObject().value("Status").toString();
                stn.type = (StationType) array.toObject().value("Type").toString().toInt();
                stn.mmsi = array.toObject().value("MMSI").toString().toULong();
                int active = array.toObject().value("Aktiv").toString().toInt();
                if (active == 0)
                    stn.active = false;
                else
                    stn.active = true;

                stationInfo.append(stn);
            }
        }
        // We should have a list of stations here, time to ask for what they can do
        stationIndex = 0;
        state = RECEIVEDSTATIONS;
        fetchDataHandler();
    }
}

void StationList::instrumentListRequest()
{
    QNetworkRequest networkRequest;
    networkRequest.setUrl(QUrl(settings->value("instrumentListUrl").toString()
                               + QString::number(stationInfo[stationIndex].StationIndex)));
    networkReply = networkAccessManager->get(networkRequest);
    state = ASKEDFORINSTRUMENTS;
}

void StationList::parseInstrumentList(const QByteArray &reply)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply);
    QJsonObject jsonObject = jsonDoc.object();
    QJsonObject metaObject = jsonObject.value("metadata").toObject();

    int nrOfElements = metaObject.value("totaltAntallTreff").toInt(0);
    if (nrOfElements > 0) {
        QJsonArray data = jsonObject.value("data").toArray();
        if (data.size() == nrOfElements) { // Super simple fault check, improve! TODO
            for (auto &&array : data) {
                InstrumentInfo info;
                info.index = array.toObject().value("Ind").toString().toInt();
                info.equipmentIndex = array.toObject().value("Utstyr_ind").toString().toInt();
                QString category = array.toObject().value("Kategori").toString();
                if (category.contains("Mottaker"))
                    info.category = InstrumentCategory::RECEIVER;
                else if (category.contains("PC"))
                    info.category = InstrumentCategory::PC;
                else
                    info.category = InstrumentCategory::UNKNOWN;
                info.frequencyRange = array.toObject().value("Frekvensområde").toString();
                info.ipAddress = array.toObject().value("IP").toString();
                info.producer = array.toObject().value("Produsent").toString();
                info.type = array.toObject().value("Type").toString();
                info.serialNumber = array.toObject().value("Serienummer").toString();

                stationInfo[stationIndex].instrumentInfo.append(info);
            }
        }
    }
    stationIndex++;
    if (stationIndex == stationInfo.size()) {
        state = RECEIVEDINSTRUMENTS;
        fetchDataHandler();
    } else {
        instrumentListRequest(); // Keep asking until all stns queried
    }
}

void StationList::equipmentListRequest()
{
    QNetworkRequest networkRequest;
    networkRequest.setUrl(QUrl(settings->value("equipmentListUrl").toString()));
    networkReply = networkAccessManager->get(networkRequest);
    if (!tmNetworkTimeout->isActive())
        tmNetworkTimeout->start(30000);
    state = ASKEDFOREQUIPMENT;
}

void StationList::parseEquipmentList(const QByteArray &reply)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply);
    QJsonObject jsonObject = jsonDoc.object();
    QJsonObject metaObject = jsonObject.value("metadata").toObject();

    int nrOfElements = metaObject.value("totaltAntallTreff").toInt(0);
    if (nrOfElements > 0) {
        QJsonArray data = jsonObject.value("data").toArray();
        if (data.size() == nrOfElements) { // Super simple fault check, improve! TODO
            for (auto &&array : data) {
                EquipmentInfo info;
                info.index = array.toObject().value("Ind").toString().toInt();
                info.type = array.toObject().value("Type").toString();
                equipmentInfo.append(info);
            }
        }
    }
    state = RECEIVEDEQUIPMENT;
    fetchDataHandler();
}

void StationList::updStationsWithEquipmentList()
{
    for (auto &&station : stationInfo) {
        for (auto &&instrument : station.instrumentInfo) {
            for (auto &&type : equipmentInfo) {
                if (instrument.equipmentIndex == type.index) {
                    instrument.type = type.type;
                    break;
                }
            }
        }
    }
}

void StationList::fetchDataHandler(const QByteArray &reply)
{
    if (state == BEGIN) {
        loginRequest();
        state = ASKEDFORLOGIN;
        emit toStatus("Logging in");
    } else if (state == ASKEDFORLOGIN) {
        parseLoginReply(reply);
    } else if (state == LOGGEDIN) {
        emit toStatus("Logged in, asking for station data");
        stationListRequest();
        state = ASKEDFORSTATIONS;
    } else if (state == ASKEDFORSTATIONS) {
        parseStationList(reply);
    } else if (state == RECEIVEDSTATIONS) {
        emit toStatus("Received station data, now asking for instruments");
        instrumentListRequest();
    } else if (state == ASKEDFORINSTRUMENTS) {
        parseInstrumentList(reply);
    } else if (state == RECEIVEDINSTRUMENTS) {
        emit toStatus("Received instruments, now asking for equipment list");
        equipmentListRequest();
    } else if (state == ASKEDFOREQUIPMENT) {
        parseEquipmentList(reply);
    } else if (state == RECEIVEDEQUIPMENT) {
        updStationsWithEquipmentList();
        state = DONE;
        qDebug() << "Station list compiled";
        emit toLog("Station list downloaded successfully");
        emit toStatus("Station list updated");
        generateLists();
        saveFile();
        emit listReady(usableStnNames, usableStnIps, usableStnTypes);
    } else if (state == FAILED) {
        qWarning() << "Failed retrieving station list, giving up";
        emit toStatus("");
        if (!usableStnNames.isEmpty())
            emit listReady(usableStnNames, usableStnIps, usableStnTypes); // Cached data
    }
}

void StationList::generateLists()
{
    usableStnIps.clear();
    usableStnNames.clear();
    usableStnTypes.clear();

    for (auto &&station : stationInfo) {
        for (auto &&instrument : station.instrumentInfo) {
            if (instrument.type.contains("Latte Panda")) {
                usableStnIps.append(instrument.ipAddress);
                usableStnNames.append(station.officialName);
                usableStnTypes.append(instrument.type);
            }
        }
    }
    //qDebug() << usableStnIps << usableStnNames << usableStnTypes;
}
