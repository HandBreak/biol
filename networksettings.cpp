#include "networksettings.h"
#include "ui_networksettings.h"

#include <QDebug>

NetworkSettings::NetworkSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NetworkSettings)
{
    ui->setupUi(this);
    uci = new QProcess();

    /* Во время инициализации сетевых интерфесов могут возникать проблемы с буфером ttyS1 !!  */
    //    uci->start("/etc/init.d/network start");
    //    uci->waitForFinished();

    QObject::connect(ui->pbExit, SIGNAL(clicked(bool)), this, SLOT(onReturnClicked()));
    QObject::connect(ui->pbAccept, SIGNAL(clicked(bool)), this, SLOT(onAcceptClicked()));
    QObject::connect(ui->dynamicIP, SIGNAL(toggled(bool)), ui->lnAddr_1, SLOT(setDisabled(bool)));
    QObject::connect(ui->dynamicIP, SIGNAL(toggled(bool)), ui->lnAddr_2, SLOT(setDisabled(bool)));
    QObject::connect(ui->dynamicIP, SIGNAL(toggled(bool)), ui->lnAddr_3, SLOT(setDisabled(bool)));
    QObject::connect(ui->dynamicIP, SIGNAL(toggled(bool)), ui->lnAddr_4, SLOT(setDisabled(bool)));
    QObject::connect(ui->dynamicIP, SIGNAL(toggled(bool)), ui->lnMask, SLOT(setDisabled(bool)));
    QObject::connect(ui->dynamicIP, SIGNAL(toggled(bool)), ui->lnGw_1, SLOT(setDisabled(bool)));
    QObject::connect(ui->dynamicIP, SIGNAL(toggled(bool)), ui->lnGw_2, SLOT(setDisabled(bool)));
    QObject::connect(ui->dynamicIP, SIGNAL(toggled(bool)), ui->lnGw_3, SLOT(setDisabled(bool)));
    QObject::connect(ui->dynamicIP, SIGNAL(toggled(bool)), ui->lnGw_4, SLOT(setDisabled(bool)));
    QObject::connect(ui->dynamicIP, SIGNAL(toggled(bool)), ui->lnDns_1, SLOT(setDisabled(bool)));
    QObject::connect(ui->dynamicIP, SIGNAL(toggled(bool)), ui->lnDns_2, SLOT(setDisabled(bool)));
    QObject::connect(ui->dynamicIP, SIGNAL(toggled(bool)), ui->lnDns_3, SLOT(setDisabled(bool)));
    QObject::connect(ui->dynamicIP, SIGNAL(toggled(bool)), ui->lnDns_4, SLOT(setDisabled(bool)));
}

NetworkSettings::~NetworkSettings()
{
    qDebug() << "Object network settings is destroyed!";
    delete uci;
    delete ui;
}

void NetworkSettings::onReturnClicked()                                                 // Обрабатываем при нажатии на кнопку "Выход"
{
    this->close();                                                                      // Закрываем текущее окно
    emit toMainReturn();                                                                // Вызываем сигнал открытия основного окна
}

void NetworkSettings::onAcceptClicked()                                                 // При нажатии "Применить" устанавливаем параметры
{
    if (ui->dynamicIP->isChecked())
    {
        uci->start("uci set network.wan.proto=dhcp");
        uci->waitForFinished();
        uci->start("uci set network.wan.ipaddr=");
        uci->waitForFinished();
        uci->start("uci set network.wan.netmask=");
        uci->waitForFinished();
        uci->start("uci set network.wan.gateway=");
        uci->waitForFinished();
        uci->start("uci set network.wan.dns=");
        uci->waitForFinished();
    }
    else
    {
        uci->start("uci set network.wan.proto=static");
        uci->waitForFinished();
        uci->start("uci set network.wan.ipaddr=" + getInputIpAddr(ui->lnAddr_1,\
                                                                  ui->lnAddr_2,\
                                                                  ui->lnAddr_3,\
                                                                  ui->lnAddr_4).toAscii());
        uci->waitForFinished();

        uci->start("uci set network.wan.gateway=" + getInputIpAddr(ui->lnGw_1,\
                                                                  ui->lnGw_2,\
                                                                  ui->lnGw_3,\
                                                                  ui->lnGw_4).toAscii());
        uci->waitForFinished();
        uci->start("uci set network.wan.dns=" + getInputIpAddr(ui->lnDns_1,\
                                                                  ui->lnDns_2,\
                                                                  ui->lnDns_3,\
                                                                  ui->lnDns_4).toAscii());
        uci->waitForFinished();
        uci->start("uci set network.wan.netmask=" + getInputMask(ui->lnMask->text().toUShort()));
        uci->waitForFinished();

    }
    if (ui->wifiOn->isChecked())
    {
        uci->start("uci set wireless.radio0.disabled=0");
        uci->waitForFinished();
        if (ui->rbAp->isChecked())
            //uci->start("wifi_mode ap");
        {
            uci->start("uci set wireless.radio0.linkit_mode=ap");
            uci->waitForFinished();
            uci->start("uci set wireless.ap.ssid=" + ui->lnSsid->text());
            uci->waitForFinished();
            uci->start("uci set wireless.ap.key=" + ui->lnKey->text());
            uci->waitForFinished();
            uci->start("uci set wireless.ap.encryption=" + ui->lnEnc->text());
        }
        else
            //uci->start("wifi_mode sta");
        {
            uci->start("uci set wireless.radio0.linkit_mode=sta");
            uci->waitForFinished();
            uci->start("uci set wireless.sta.ssid=" + ui->lnSsid->text());
            uci->waitForFinished();
            uci->start("uci set wireless.sta.key=" + ui->lnKey->text());
            uci->waitForFinished();
            uci->start("uci set wireless.sta.encryption=" + ui->lnEnc->text());
        }
        uci->waitForFinished();
     }
    else
    {
        uci->start("uci set wireless.radio0.disabled=1");
        uci->waitForFinished();
    }
    uci->start("uci commit");
    uci->waitForFinished();

    uci->start("wifi");
    uci->waitForFinished();

    uci->start("/etc/init.d/network reload");                                           // Во время инициализации сетевых интерфесов могут возникать проблемы с буфером ttyS1 !!
    uci->waitForFinished();

    if (ui->lanOn->isChecked())
    {
        uci->start("ifconfig eth0 up");
        uci->waitForFinished();
    }
    else
    {
        uci->start("ifconfig eth0 down");
        uci->waitForFinished();
    }
    readSettings();
}
void NetworkSettings::readSettings()
{
    readWiFiSettings();
    readLanSettings();
}
QStringList NetworkSettings::readWiFiSettings()
{
    QStringList wifisettings;
    uci->start("uci get wireless.radio0.disabled");
    uci->waitForFinished();
    QString mode = uci->readAll();
    if (mode.toShort() == 1)
    {
        ui->wifiOn->setChecked(false);
        ui->wifiMode->setEnabled(false);
    }
    else
    {
        ui->wifiOn->setChecked(true);
        ui->wifiMode->setEnabled(true);
    }
    uci->start("uci get wireless.radio0.linkit_mode");
    uci->waitForFinished();
    mode = uci->readAll();
    if (mode == "ap\n")                                                                 // 'sta' mode - station
    {
        ui->rbAp->setChecked(true);
        uci->start("uci get wireless.ap.ssid");
        uci->waitForFinished();
        wifisettings.append(uci->readAll());
        uci->start("uci get wireless.ap.key");
        uci->waitForFinished();
        wifisettings.append(uci->readAll());
        uci->start("uci get wireless.ap.encryption");
        uci->waitForFinished();
        wifisettings.append(uci->readAll());
        ui->lnSsid->setDisabled(true);
        ui->lnKey->setDisabled(true);
        ui->lnEnc->setDisabled(true);
    }
    else
    {
        ui->rbSta->setChecked(true);
        uci->start("uci get wireless.sta.ssid");
        uci->waitForFinished();
        wifisettings.append(uci->readAll());
        uci->start("uci get wireless.sta.key");
        uci->waitForFinished();
        wifisettings.append(uci->readAll());
        uci->start("uci get wireless.sta.encryption");
        uci->waitForFinished();
        wifisettings.append(uci->readAll());
        ui->lnSsid->setDisabled(false);
        ui->lnKey->setDisabled(false);
        ui->lnEnc->setDisabled(false);
    }
    ui->lnSsid->setText(wifisettings.at(0).toAscii());
    ui->lnKey->setText(wifisettings.at(1).toAscii());
    ui->lnEnc->setText(wifisettings.at(2).toAscii());
    return wifisettings;
}
QStringList NetworkSettings::readLanSettings()
{

    QStringList ipsettings;
    uci->start("ifconfig");
    uci->waitForFinished();
    if (uci->readAll().contains("eth0"))
        ui->lanOn->setChecked(true);
    else
        ui->lanOn->setChecked(false);
    uci->start("uci get network.wan.proto");
    uci->waitForFinished();
    QString ip = uci->readAll();
    if (ip == "dhcp\n")
    {
        ui->dynamicIP->setChecked(true);
        uci->start("ifstatus wan");
        uci->waitForFinished();
        QByteArray rawData = uci->readAll();
        QJsonDocument doc(QJsonDocument::fromJson(rawData));
        QJsonObject json = doc.object();
        if (json["up"].toBool())
        {
            QJsonArray ipv4 = json["ipv4-address"].toArray();
            QJsonArray gate = json["route"].toArray();
            QJsonArray dns  = json["dns-server"].toArray();
            json =  ipv4[0].toObject();
            ipsettings.append(json["address"].toString());
            ipsettings.append(QString::number(json["mask"].toDouble()));
            ipsettings.append(gate[0].toObject()["target"].toString());
            ipsettings.append(dns[0].toString());
        }
        return ipsettings;
    }
    else
    {
        ui->dynamicIP->setChecked(false);

        uci->start("uci get network.wan.ipaddr");
        uci->waitForFinished();
        ipsettings.append(uci->readAll());
        uci->start("uci get network.wan.netmask");
        uci->waitForFinished();
        ipsettings.append(QString::number(setInputMask(uci->readAll())));               //  ПРЕОБРАЗОВАТЬ К ФОРМАТУ /XX
        uci->start("uci get network.wan.gateway");
        uci->waitForFinished();
        ipsettings.append(uci->readAll());
        uci->start("uci get network.wan.dns");
        uci->waitForFinished();
        ipsettings.append(uci->readAll());
    }
    return ipsettings;
}

void NetworkSettings::setCurrentParameters()
{
    QStringList wfset = readWiFiSettings();
    QStringList ipset = readLanSettings();
    if (ipset.size() == 4)
    {
        ui->lnMask->setText(ipset.at(1));

        fillIpAddrFields(ipset.at(0),
                         ui->lnAddr_1,
                         ui->lnAddr_2,
                         ui->lnAddr_3,
                         ui->lnAddr_4);

        fillIpAddrFields(ipset.at(2),
                         ui->lnGw_1,
                         ui->lnGw_2,
                         ui->lnGw_3,
                         ui->lnGw_4);

        fillIpAddrFields(ipset.at(3),
                         ui->lnDns_1,
                         ui->lnDns_2,
                         ui->lnDns_3,
                         ui->lnDns_4);
    }
}

void NetworkSettings::fillIpAddrFields(const QString ip, \
                                       QLineEdit *oct1, \
                                       QLineEdit *oct2, \
                                       QLineEdit *oct3, \
                                       QLineEdit *oct4 )
{
    QStringList octets = ip.split(".");
    if (octets.size() == 4)
    {
        oct1->setText(octets.at(0));
        oct2->setText(octets.at(1));
        oct3->setText(octets.at(2));
        oct4->setText(octets.at(3));
    }
    qDebug() << octets;
}

QString NetworkSettings::getInputIpAddr( \
                                     QLineEdit *oct1, \
                                     QLineEdit *oct2, \
                                     QLineEdit *oct3, \
                                     QLineEdit *oct4 )
{
    QString ip = oct1->text() + "." +\
                 oct2->text() + "." + \
                 oct3->text() + "." + \
                 oct4->text();
    return ip;
}

QString NetworkSettings::getInputMask(ushort m)
{
    QString mask = "";
    if (m > 32)
    {
        return mask;
    }
    char loop = 4;
    do
    {
        loop--;
        if (m >= 8)
            mask = mask + "255";
        else
        {
            ushort n = 128, byte = 0;
            for (ushort i = 1; i <= m; i++)
            {
                byte = byte + n;
                n >>= 1;
            }
            mask = mask + QString::number(byte).toAscii();
        }
        if (loop > 0)
            mask = mask + ".";
        m -= 8;
    } while (m > 0);
    while (loop)
    {
            loop--;
            mask = mask + "0";
            if (loop > 0)
                mask = mask + ".";
    }
    return mask;
}

char NetworkSettings::setInputMask(QString ip)
{
    char m = 0;
    QStringList octets = ip.split(".");
    if (octets.size() == 4)
    {

        for (char i = 0; i < 4; i++)
        {
            ushort byte = octets.at(i).toUShort();
            ushort n = 128;
            while (((byte & 255) & n) != 0) {
                m++;
                n >>= 1;
            }
        }
    }
    return m;
}

void NetworkSettings::initNetwork()
{
        uci->start("/etc/init.d/network start");                                        // Во время инициализации сетевых интерфесов могут возникать проблемы с буфером ttyS1 !!
        uci->waitForFinished();
}
