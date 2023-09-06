#include "license_server.h"

license_server::license_server(QObject *parent)
    : QObject{parent}{
    manager = new QNetworkAccessManager(this);
    consider_key = false;
}

void license_server::getIp(){
    foreach(const QHostAddress &address, QNetworkInterface::allAddresses()){
        if(address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost)){
            ip = address.toString();
        }
    }
//    qDebug() << ip;
}

void license_server::login(QString email, QString password){
    getIp();

//    QJsonObject user_obj;
//    user_obj.insert("email", QJsonValue::fromVariant(email));
//    user_obj.insert("ipAddress", QJsonValue::fromVariant(ip));
//    QJsonDocument doc(user_obj);
//    qDebug() << doc.toJson();

    QNetworkRequest request;

    QString link = "https://licensemanagementwebapi.azurewebsites.net/auth/signin/" + email + "/" + ip;
    qDebug() << link;
    QUrl url(/*"http://192.168.1.188:80/api/tickets"*/link);


    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1_2);
    request.setSslConfiguration(config);
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ServerHeader, "application/json");

//    reply = manager->post(request, doc.toJson());
//    reply = manager->post(request, doc.toJson());
//    change = false;


//    connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(uploadProgress(qint64,qint64)));
//    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishReply(QNetworkReply*)));
//    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(showError(QNetworkReply::NetworkError)));


//    while(!change){qDebug() << "lick my dick";}
    qDebug() << "changed your mind like a girl";
    reply = manager->get(request);
    connect(reply, SIGNAL(readyRead()), this, SLOT(readCallback()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishReplydown(QNetworkReply*)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(showErrordown(QNetworkReply::NetworkError)));
}

void license_server::uploadProgress(qint64 byte_sent, qint64 total_byte){
    qDebug() << "Uploaded:" << byte_sent << "of" << total_byte;
}


void license_server::finishReply(QNetworkReply *){
    if(reply->error() == QNetworkReply::NoError){
        qDebug() << "Transfered succesfully";
//        change = true;
    } else{
        qDebug() << "Failed to transfer";
    }

}

void license_server::showError(QNetworkReply::NetworkError error){
    qDebug() << "Error:" << error;
}

void license_server::readCallback(){
    qDebug() << "called";
    qDebug() << QJsonDocument::fromJson(reply->readAll()).object();
}

void license_server::finishReplydown(QNetworkReply *){
    change = false;
    qDebug() << "done";
    reply->deleteLater();
}

void license_server::showErrordown(QNetworkReply::NetworkError error){
    qDebug() << "Error:" << error;
}

void license_server::downloadProgress(qint64 byte_received, qint64 total_byte){
    qDebug() << "Downloaded:" << byte_received << "of" << total_byte;
}

int license_server::licenseProcess(QString key){
    QFile unlock_file1("C:\\Autoss\\Draft\\License\\Unlock_key1.txt");
    QFile unlock_file2("C:\\Autoss\\Draft\\License\\Unlock_key2.txt");
    QString key1, key2, key_used;
    bool confirmation = false;
    if(unlock_file1.open(QIODevice::ReadOnly)){
        key1 = unlock_file1.readLine();
        unlock_file1.flush();
        unlock_file1.close();
    }
    if(unlock_file2.open(QIODevice::ReadOnly)){
        key2 = unlock_file2.readLine();
        unlock_file2.flush();
        unlock_file2.close();
    }

    QString part1 = key.split("-").at(0);
    QString part2 = key.split("-").at(1);
    QString part3 = key.split("-").at(2);
    QString part4 = key.split("-").at(3);
    QString part5, part6;

    bool management_key = false;
    if(key.size() > 19){
        part5 = key.split("-").at(4);
        part6 = key.split("-").at(5);
    }

    key_used = "";
    if(consider_key){
        consider_key = false;

        if(key1.contains("1310")){
            if(part3 == key1){
                confirmation = true;
                key_used = "key1";
            }
        } else{
            if(key2.contains("2107")){
                if(part3 == key2){
                    confirmation = true;
                    key_used = "key2";
                }
            }
        }

        if(confirmation){
            confirmation = false;
            QFile lock_file("C:\\Autoss\\Draft\\License\\Lock.txt");
            QFile count_file("C:\\Autoss\\Draft\\License\\Crack_warning.txt");
            QFile key_file("C:\\Autoss\\Draft\\License\\Key.txt");

            if(lock_file.exists()){
                if(lock_file.open(QIODevice::WriteOnly)){
                    QTextStream str(&lock_file);
//                    qDebug() << "Lock is opennnnnnnnnnnnnnnnn";
                    str << "";
                    lock_file.flush();
                    lock_file.close();
                }
            }

            if(count_file.open(QIODevice::WriteOnly)){
                QTextStream str1(&count_file);
    //            qDebug() << "22222222222222222";
                str1 << "2";
                count_file.flush();
                count_file.close();
            }

            if(key_file.open(QIODevice::WriteOnly)){
                QTextStream str2(&count_file);
                str2 << "";
                key_file.flush();
                key_file.close();
            }

            if(key_used == "key1") unlock_file1.remove();
            if(key_used == "key2") unlock_file2.remove();

            return -1;
        }
    }


    int x1 = part1.split("").at(1).toInt();
    int x2 = part1.split("").at(2).toInt();
    int x3 = part1.split("").at(3).toInt();
    int x4 = part1.split("").at(4).toInt();

//    qDebug() << part1;
//    qDebug() << part2;
//    qDebug() << part3;
//    qDebug() << part4;

    int a, b, c, d;
    if(part2.split("").at(1) == "A") a = 65;
    else if(part2.split("").at(1) == "C") a = 67;
    else if(part2.split("").at(1) == "E") a = 69;

    if(part2.split("").at(2) == "F") b = 70;
    else if(part2.split("").at(2) == "G") b = 71;
    else if(part2.split("").at(2) == "H") b = 72;
    else if(part2.split("").at(2) == "I") b = 73;
    else if(part2.split("").at(2) == "J") b = 74;
    else if(part2.split("").at(2) == "K") b = 75;
    else if(part2.split("").at(2) == "L") b = 76;
    else if(part2.split("").at(2) == "M") b = 77;
    else if(part2.split("").at(2) == "N") b = 78;
    else if(part2.split("").at(2) == "O") b = 79;

    if(part2.split("").at(3) == "P") c = 80;
    else if(part2.split("").at(3) == "R") c = 82;
    else if(part2.split("").at(3) == "T") c = 84;
    else if(part2.split("").at(3) == "V") c = 86;
    else if(part2.split("").at(3) == "X") c = 88;

    if(part2.split("").at(4) == "Q") d = 81;
    else if(part2.split("").at(4) == "S") d = 83;
    else if(part2.split("").at(4) == "U") d = 85;
    else if(part2.split("").at(4) == "W") d = 87;
    else if(part2.split("").at(4) == "Y") d = 89;

    int s1 = part3.split("").at(1).toInt();
    int s2 = part3.split("").at(2).toInt();
    int s3 = part3.split("").at(3).toInt();
    int s4 = part3.split("").at(4).toInt();

    int x5 = part4.split("").at(1).toInt();
    int x6 = part4.split("").at(2).toInt();
    int x7 = part4.split("").at(3).toInt();
    int x8 = part4.split("").at(4).toInt();
//    qDebug() << x1;
//    qDebug() << x2;
//    qDebug() << x3;
//    qDebug() << x4;
//    qDebug() << a;
//    qDebug() << b;
//    qDebug() << c;
//    qDebug() << d;
//    qDebug() << s1;
//    qDebug() << s2;
//    qDebug() << s3;
//    qDebug() << s4;
//    qDebug() << x5;
//    qDebug() << x6;
//    qDebug() << x7;
//    qDebug() << x8;
    int sum1, sum2;
    sum1 = x1 + x2 + x3 + x4;
    sum2 = s1 + s2 + s3 + s4;
    if(!(x5*10 + x6 == sum1 + sum2 || x6*10 + x5 == sum1 + sum2)) return -1;

    int d1 = 0;
    if(s1 == 1) d1 = a - 60 - x1;
    else if(s1 == 0) d1 = a - 60 + x1;

    int d2 = 0;
    if(s2 == 9) d2 = b - 70 - x4;
    else if(s2 == 8) d2 = b - 70 + x4;

    int m1 = 0;
    if(s3 == 3) m1 = c - 80 - x2;
    else if(s3 == 2) m1 = c - 80 + x2;

    int m2 = 0;
    if(s4 == 7) m2 = d - 80 - x3;
    else if(s4 == 6) m2 = d - 80 + x3;

    int day = d1*10 + d2;
    int month = m1*10 + m2;

    int y3, y4;
    if(x1 >= x3) y4 = x7 - (x1 - x3);
    else y4 = x7 - (x3 - x1);
    if(x2 >= x4) y3 = x8 - (x2 - x4);
    else y3 = x8 - (x4 - x2);

    int year = y3*10 + y4;

//    qDebug() << day;
//    qDebug() << month;
//    qDebug() << year;

    if(day < 0 || day > 31 || month < 0 || month > 12 || year < 0 || year > 99 || (day > 28 && month == 2 && year % 4 != 0) || (day > 30 && (month == 2 || month == 4 || month == 6 || month == 9 || month == 11))) return -1;

//    qDebug() << "passed";
    QDate expiry_date(2000 + year, month, day);
    int days_left = QDate::currentDate().daysTo(expiry_date) + 1;
//    qDebug() << days_left;

    // Checking Autoss level key
    if(part5 == "2902"){
        int p1 = part6.split("").at(1).toInt();
        int p2 = part6.split("").at(2).toInt();
        int p3 = part6.split("").at(3).toInt();
        int p4 = part6.split("").at(4).toInt();
        int temp_day = (p1*10 + p2) - (x1*10 + x2);
        int temp_month = (p3*10 + p4) - (x3*10 + x4);
//        qDebug() << "Autossss:" << temp_day << temp_month;
        if(temp_day == 30 && temp_month == 9){
            management_key = true;
        }
    }

//    qDebug() << "Autossss:" << management_key;
    emit confirmAutossManagement(management_key);
    return days_left;
}

bool license_server::checkTimerCheating(){
    int year = QDate::currentDate().year();
    int month = QDate::currentDate().month();
    int day = QDate::currentDate().day();
    int hour = QTime::currentTime().hour();
    int minute = QTime::currentTime().minute();
    int second = QTime::currentTime().second();
    int msecond = QTime::currentTime().msec();

    QDir dir("C:\\Autoss\\Draft\\Date");
    if(!dir.exists()){
        dir.mkpath("C:\\Autoss\\Draft\\Date");
        SetFileAttributesW(L"C:\\Autoss", FILE_ATTRIBUTE_HIDDEN);
        SetFileAttributesW(L"C:\\Autoss\\Draft\\Date", FILE_ATTRIBUTE_HIDDEN);
        QFile file("C:\\Autoss\\Draft\\Date\\date.txt");
        if(file.open(QIODevice::WriteOnly)){
            QTextStream str(&file);
            str << QString::number(day) + " " + QString::number(month) + " " + QString::number(year);
            file.flush();
            file.close();
        }
        file.setFileName("C:\\Autoss\\Draft\\Date\\time.txt");
        if(file.open(QIODevice::WriteOnly)){
            QTextStream str(&file);
            str << QString::number(hour) + " " + QString::number(minute) + " " + QString::number(second) + " " + QString::number(msecond);
            file.flush();
            file.close();
        }
        dir.mkpath("C:\\Autoss\\Draft\\License");
        SetFileAttributesW(L"C:\\Autoss\\Draft\\License", FILE_ATTRIBUTE_HIDDEN);
        file.setFileName("C:\\Autoss\\Draft\\License\\Crack_warning.txt");
        if(file.open(QIODevice::WriteOnly)){
            qDebug() << "00000000000000000";
            QTextStream str(&file);
            str << "0";
            file.flush();
            file.close();
        }
        QFile unlock_file("C:\\Autoss\\Draft\\License\\Unlock_key1.txt");
        if(unlock_file.open(QIODevice::WriteOnly)){
            QTextStream str1(&unlock_file);
            str1 << "1310";
            unlock_file.flush();
            unlock_file.close();
            unlock_file.setFileName("C:\\Autoss\\Draft\\License\\Unlock_key2.txt");
            if(unlock_file.open(QIODevice::WriteOnly)){
                QTextStream str2(&unlock_file);
                str2 << "2107";
                unlock_file.flush();
                unlock_file.close();
            }
        }
//        qDebug() << true;
        return true;
    } else{
//        qDebug() << false;
        QFile file("C:\\Autoss\\Draft\\Date\\date.txt");
        QFile time_file("C:\\Autoss\\Draft\\Date\\time.txt");
        if(file.open(QIODevice::ReadOnly)){
            QString date = file.readLine();
            int l_day = date.split(" ").at(0).toInt();
            int l_month = date.split(" ").at(1).toInt();
            int l_year = date.split(" ").at(2).toInt();
            file.flush();
            file.close();
//            qDebug() << "day: " << l_day;
//            qDebug() << "month: " << l_month;
//            qDebug() << "year: " << l_year;
            QDate last_day(l_year, l_month, l_day);
//            qDebug() << "day gap: " << last_day.daysTo(QDate::currentDate());
            file.flush();
            file.close();
            if(time_file.open(QIODevice::ReadOnly)){
                QString time = time_file.readLine();
                int l_hour= time.split(" ").at(0).toInt();
                int l_min= time.split(" ").at(1).toInt();
                int l_sec= time.split(" ").at(2).toInt();
                int l_msec= time.split(" ").at(3).toInt();
                time_file.flush();
                time_file.close();

                QDateTime l_date(QDate(l_year, l_month, l_day), QTime(l_hour, l_min, l_sec, l_msec));
                QDateTime cur_date(QDate(year, month, day), QTime(hour, minute, second, msecond));
                int date_gap = l_date.msecsTo(cur_date);
//                qDebug() << "Date gap:" << date_gap;
                if(date_gap > 0){
                    if(time_file.open(QIODevice::WriteOnly)){
                        QTextStream str(&time_file);
                        str << QString::number(hour) + " " + QString::number(minute) + " " + QString::number(second) + " " + QString::number(msecond);
                        time_file.flush();
                        time_file.close();
                    }

                    if(file.open(QIODevice::WriteOnly)){
                        QTextStream str1(&file);
                        str1 << QString::number(day) + " " + QString::number(month) + " " + QString::number(year);
                        file.flush();
                        file.close();
                    }
                    return true;
                } else return false;
            }
        }
    }
}

void license_server::unlockConsider(){
    consider_key = true;
}
