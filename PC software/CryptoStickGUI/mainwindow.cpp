/*
* Author: Copyright (C) Andrzej Surowiec 2012
*
*
* This file is part of GPF Crypto Stick.
*
* GPF Crypto Stick is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* GPF Crypto Stick is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with GPF Crypto Stick. If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "device.h"
#include "response.h"
#include "string.h"
#include "sleep.h"
#include "base32.h"
#include "passworddialog.h"


#include <QTimer>
#include <QMenu>
#include <QtGui>
#include <QDateTime>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusBar->showMessage("Device disconnected.");

    cryptostick =  new Device(0x20a0, 0x4107);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkConnection()));
    timer->start(1000);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/images/CS_icon.png"));

    trayIcon->show();

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    restoreAction = new QAction(tr("&Configure"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(startConfiguration()));

    totp1Action = new QAction(tr("&TOTP Slot 1"), this);
    connect(totp1Action, SIGNAL(triggered()), this, SLOT(getCode()));

    generateMenu();

}

void MainWindow::checkConnection(){

    currentTime= QDateTime::currentDateTime().toTime_t();

    int result = cryptostick->checkConnection();
    if (result==0)
        ui->statusBar->showMessage("Device connected.");
    else if (result==-1){
        ui->statusBar->showMessage("Device disconnected.");
        generateMenu();
        cryptostick->connect();
    }
    else if (result==1){ //recreate the settings and menus
        ui->statusBar->showMessage("Device connected.");
        generateMenu();
    }
}


void MainWindow::startTimer(){



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    this->hide();
    event->ignore();
}

void MainWindow::on_pushButton_clicked()
{
    if (cryptostick->isConnected){
        int64_t crc = cryptostick->getSlotName(0x11);

        Sleep::msleep(100);
        Response *testResponse=new Response();
        testResponse->getResponse(cryptostick);

        if (crc==testResponse->lastCommandCRC){

            QMessageBox message;
            QString str;
            //str.append(QString::number(testResponse->lastCommandCRC,16));
            QByteArray *data =new QByteArray(testResponse->data);
            str.append(QString(data->toHex()));

            message.setText(str);
            message.exec();

        }
    }
}

void MainWindow::on_pushButton_2_clicked()
{




}

void MainWindow::getSlotNames(){




}

void MainWindow::generateMenu()
{

    trayMenu = new QMenu();

    if (cryptostick->isConnected==false){
        trayMenu->addAction("Crypto Stick not connected");

        //
    }
    else{
        if (cryptostick->HOTPSlots[0]->isProgrammed==true){
            QString action1;
            action1.append("HOTP slot 1 ");
            action1.append((char *)cryptostick->HOTPSlots[0]->slotName);
            trayMenu->addAction(action1);
        }
        if (cryptostick->HOTPSlots[1]->isProgrammed==true){
            QString action2;
            action2.append("HOTP slot 2 ");
            action2.append((char *)cryptostick->HOTPSlots[1]->slotName);
            trayMenu->addAction(action2);

        }
        if (cryptostick->TOTPSlots[0]->isProgrammed==true){
            trayMenu->addAction(totp1Action);

        }
        if (cryptostick->TOTPSlots[1]->isProgrammed==true){
            QString action;
            action.append("TOTP slot 2 ");
            action.append((char *)cryptostick->TOTPSlots[1]->slotName);
            trayMenu->addAction(action);

        }
        if (cryptostick->TOTPSlots[2]->isProgrammed==true){
            QString action;
            action.append("TOTP slot 3 ");
            action.append((char *)cryptostick->TOTPSlots[2]->slotName);
            trayMenu->addAction(action);

        }
        if (cryptostick->TOTPSlots[3]->isProgrammed==true){
            QString action;
            action.append("TOTP slot 4 ");
            action.append((char *)cryptostick->TOTPSlots[3]->slotName);
            trayMenu->addAction(action);

        }
        trayMenu->addAction(restoreAction);
    }
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayMenu);

    ui->slotComboBox->setItemText(0,QString("HOTP slot 1 [").append((char *)cryptostick->HOTPSlots[0]->slotName).append("]"));
    ui->slotComboBox->setItemText(1,QString("HOTP slot 2 [").append((char *)cryptostick->HOTPSlots[1]->slotName).append("]"));
    ui->slotComboBox->setItemText(2,QString("TOTP slot 1 [").append((char *)cryptostick->TOTPSlots[0]->slotName).append("]"));
    ui->slotComboBox->setItemText(3,QString("TOTP slot 2 [").append((char *)cryptostick->TOTPSlots[1]->slotName).append("]"));
    ui->slotComboBox->setItemText(4,QString("TOTP slot 3 [").append((char *)cryptostick->TOTPSlots[2]->slotName).append("]"));
    ui->slotComboBox->setItemText(5,QString("TOTP slot 4 [").append((char *)cryptostick->TOTPSlots[3]->slotName).append("]"));
}


void MainWindow::generateHOTPConfig(HOTPSlot *slot)
{
    int selectedSlot=ui->slotComboBox->currentIndex();

    //    if (selectedSlot==0)
    //        slot->slotNumber=0x10;
    //    else if (selectedSlot==1)
    //        slot->slotNumber=0x11;

    if (selectedSlot>=0&&selectedSlot<=1){
        slot->slotNumber=selectedSlot+0x10;


    QByteArray secretFromGUI = QByteArray::fromHex(ui->secretEdit->text().toAscii());
    memset(slot->secret,0,20);
    memcpy(slot->secret,secretFromGUI.data(),secretFromGUI.size());

    QByteArray slotNameFromGUI = QByteArray(ui->nameEdit->text().toAscii());
    memset(slot->slotName,0,15);
    memcpy(slot->slotName,slotNameFromGUI.data(),slotNameFromGUI.size());

    memset(slot->tokenID,0,13);
    QByteArray ompFromGUI = (ui->ompEdit->text().toAscii());
    memcpy(slot->tokenID,ompFromGUI,2);

    QByteArray ttFromGUI = (ui->ttEdit->text().toAscii());
    memcpy(slot->tokenID+2,ttFromGUI,2);

    QByteArray muiFromGUI = (ui->muiEdit->text().toAscii());
    memcpy(slot->tokenID+4,muiFromGUI,8);

    slot->tokenID[12]=ui->keyboardComboBox->currentIndex()&0xFF;

    QByteArray counterFromGUI = QByteArray::fromHex(ui->counterEdit->text().toAscii());
    memset(slot->counter,0,8);
    memcpy(slot->counter,counterFromGUI.data(),counterFromGUI.length());

    slot->config=0;

    if (ui->digits8radioButton->isChecked())
        slot->config+=(1<<0);
    if (ui->enterCheckBox->isChecked())
        slot->config+=(1<<1);
    if (ui->tokenIDCheckBox->isChecked())
        slot->config+=(1<<2);


    }
   // qDebug() << slot->counter;

}

void MainWindow::generateTOTPConfig(TOTPSlot *slot)
{
    int selectedSlot=ui->slotComboBox->currentIndex();

    //    if (selectedSlot==0)
    //        slot->slotNumber=0x10;
    //    else if (selectedSlot==1)
    //        slot->slotNumber=0x11;

    if (selectedSlot>=2&&selectedSlot<=5){

        slot->slotNumber=selectedSlot+0x1E;

    QByteArray secretFromGUI = QByteArray::fromHex(ui->secretEdit->text().toAscii());
    memset(slot->secret,0,20);
    memcpy(slot->secret,secretFromGUI.data(),secretFromGUI.size());

    QByteArray slotNameFromGUI = QByteArray(ui->nameEdit->text().toAscii());
    memset(slot->slotName,0,15);
    memcpy(slot->slotName,slotNameFromGUI.data(),slotNameFromGUI.size());

    memset(slot->tokenID,0,13);
    QByteArray ompFromGUI = (ui->ompEdit->text().toAscii());
    memcpy(slot->tokenID,ompFromGUI,2);

    QByteArray ttFromGUI = (ui->ttEdit->text().toAscii());
    memcpy(slot->tokenID+2,ttFromGUI,2);

    QByteArray muiFromGUI = (ui->muiEdit->text().toAscii());
    memcpy(slot->tokenID+4,muiFromGUI,8);

    slot->config=0;

    if (ui->digits8radioButton->isChecked())
        slot->config+=(1<<0);
    if (ui->enterCheckBox->isChecked())
        slot->config+=(1<<1);
    if (ui->tokenIDCheckBox->isChecked())
        slot->config+=(1<<2);


}
}

void MainWindow::displayCurrentSlotConfig()
{
    uint8_t slotNo=ui->slotComboBox->currentIndex();

    if (slotNo>=0&&slotNo<=1){
        ui->hotpGroupBox->show();
        //slotNo=slotNo+0x10;
        ui->nameEdit->setText(QString((char *)cryptostick->HOTPSlots[slotNo]->slotName));

        QByteArray secret((char *) cryptostick->HOTPSlots[slotNo]->secret,20);
        ui->hexRadioButton->setChecked(true);
        ui->secretEdit->setText(secret.toHex());

        QByteArray counter((char *) cryptostick->HOTPSlots[slotNo]->counter,8);
        ui->counterEdit->setText(counter.toHex());

        if (cryptostick->HOTPSlots[slotNo]->counter==0)
            ui->counterEdit->setText("0");

        QByteArray omp((char *)cryptostick->HOTPSlots[slotNo]->tokenID,2);
        ui->ompEdit->setText(QString(omp));

        QByteArray tt((char *)cryptostick->HOTPSlots[slotNo]->tokenID+2,2);
        ui->ttEdit->setText(QString(tt));

        QByteArray mui((char *)cryptostick->HOTPSlots[slotNo]->tokenID+4,8);
        ui->muiEdit->setText(QString(mui));

        if (cryptostick->HOTPSlots[slotNo]->config&(1<<0))
            ui->digits8radioButton->setChecked(true);
        else ui->digits6radioButton->setChecked(true);

        if (cryptostick->HOTPSlots[slotNo]->config&(1<<1))
            ui->enterCheckBox->setChecked(true);
        else ui->enterCheckBox->setChecked(false);

        if (cryptostick->HOTPSlots[slotNo]->config&(1<<2))
            ui->tokenIDCheckBox->setChecked(true);
        else ui->tokenIDCheckBox->setChecked(false);


        //qDebug() << "Counter value:" << cryptostick->HOTPSlots[slotNo]->counter;

    }
    else if (slotNo>=2&&slotNo<=5){
        slotNo-=2;
        //ui->hotpGroupBox->hide();

        ui->nameEdit->setText(QString((char *)cryptostick->TOTPSlots[slotNo]->slotName));


        QByteArray secret((char *) cryptostick->TOTPSlots[slotNo]->secret,20);
        ui->hexRadioButton->setChecked(true);
        ui->secretEdit->setText(secret.toHex());

        ui->counterEdit->setText("0");

    QByteArray omp((char *)cryptostick->TOTPSlots[slotNo]->tokenID,2);
    ui->ompEdit->setText(QString(omp));

    QByteArray tt((char *)cryptostick->TOTPSlots[slotNo]->tokenID+2,2);
    ui->ttEdit->setText(QString(tt));

    QByteArray mui((char *)cryptostick->TOTPSlots[slotNo]->tokenID+4,8);
    ui->muiEdit->setText(QString(mui));

    if (cryptostick->TOTPSlots[slotNo]->config&(1<<0))
        ui->digits8radioButton->setChecked(true);
    else ui->digits6radioButton->setChecked(true);

    if (cryptostick->TOTPSlots[slotNo]->config&(1<<1))
        ui->enterCheckBox->setChecked(true);
    else ui->enterCheckBox->setChecked(false);

    if (cryptostick->TOTPSlots[slotNo]->config&(1<<2))
        ui->tokenIDCheckBox->setChecked(true);
    else ui->tokenIDCheckBox->setChecked(false);
    }

}

void MainWindow::displayCurrentGeneralConfig()
{
    QByteArray firmware = QByteArray((char *) cryptostick->firmwareVersion).toHex();
    ui->firmwareEdit->setText(QString(firmware));
    qDebug() << QString(firmware);
    QByteArray cardSerial = QByteArray((char *) cryptostick->cardSerial).toHex();

    ui->serialEdit->setText(QString(cardSerial));

    ui->numLockComboBox->setCurrentIndex(0);
    ui->capsLockComboBox->setCurrentIndex(0);
    ui->scrollLockComboBox->setCurrentIndex(0);

    if (cryptostick->generalConfig[0]==0||cryptostick->generalConfig[0]==1)
        ui->numLockComboBox->setCurrentIndex(cryptostick->generalConfig[0]+1);
    if (cryptostick->generalConfig[1]==0||cryptostick->generalConfig[1]==1)
        ui->capsLockComboBox->setCurrentIndex(cryptostick->generalConfig[1]+1);
    if (cryptostick->generalConfig[2]==0||cryptostick->generalConfig[2]==1)
        ui->scrollLockComboBox->setCurrentIndex(cryptostick->generalConfig[2]+1);



}

void MainWindow::startConfiguration()
{

    //PasswordDialog pd;
    //pd.exec();

    cryptostick->getSlotConfigs();
    displayCurrentSlotConfig();

    cryptostick->getStatus();
    displayCurrentGeneralConfig();

    showNormal();
}

void MainWindow::getCode(uint8_t slotNo)
{
    uint8_t result[18];
    memset(result,0,18);
    uint32_t code;


     QClipboard *clipboard = QApplication::clipboard();

     cryptostick->getCode(slotNo,currentTime/30,result);

     code=result[0]+(result[1]<<8)+(result[2]<<16)+(result[2]<<16);
     code=code%1000000;

     qDebug() << "Current time:" << currentTime;
     qDebug() << "Counter:" << currentTime/30;
     qDebug() << "TOTP:" << code;

}


void MainWindow::on_writeButton_clicked()
{
    QMessageBox msgBox;
    int res;

    if (cryptostick->isConnected){

        ui->hexRadioButton->toggle();

        if (ui->slotComboBox->currentIndex()<2){//HOTP slot
            HOTPSlot *hotp=new HOTPSlot();

            generateHOTPConfig(hotp);
            //HOTPSlot *hotp=new HOTPSlot(0x10,(uint8_t *)"Herp",(uint8_t *)"123456",(uint8_t *)"0",0);
            res = cryptostick->writeToHOTPSlot(hotp);

        }
        else{//TOTP slot
            TOTPSlot *totp=new TOTPSlot();
            generateTOTPConfig(totp);
            res = cryptostick->writeToTOTPSlot(totp);
        }



        if (res==0)
            msgBox.setText("Config written successfully!");
        else
            msgBox.setText("Error writing config!");

        msgBox.exec();

        Sleep::msleep(500);
        cryptostick->initializeConfig();
        cryptostick->getSlotConfigs();
        displayCurrentSlotConfig();
        generateMenu();

    }
    else{
        msgBox.setText("Crypto stick not connected!");
        msgBox.exec();
    }
}

void MainWindow::on_slotComboBox_currentIndexChanged(int index)
{
    displayCurrentSlotConfig();
}

void MainWindow::on_resetButton_clicked()
{
    displayCurrentSlotConfig();
}


void MainWindow::on_hexRadioButton_toggled(bool checked)
{
    if (checked){

        QByteArray secret;
        secret = ui->secretEdit->text().toAscii();

        qDebug() << "encoded secret:" << QString(secret);

        uint8_t encoded[128];
        uint8_t decoded[20];
        memset(encoded,'A',32);
        memcpy(encoded,secret.data(),secret.length());

        base32_decode(encoded,decoded,20);

        ui->secretEdit->setInputMask("HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH;");
        //ui->secretEdit->setInputMask("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH;0");
        //ui->secretEdit->setMaxLength(59);

        secret = QByteArray((char *)decoded,20).toHex();

        ui->secretEdit->setText(QString(secret));
        //qDebug() << QString(secret);

    }
}

void MainWindow::on_base32RadioButton_toggled(bool checked)
{
    if (checked){

        QByteArray secret;
        secret = QByteArray::fromHex(ui->secretEdit->text().toAscii());


        uint8_t encoded[128];
        uint8_t decoded[20];
        memset(decoded,0,20);
        memcpy(decoded,secret.data(),secret.length());

        base32_encode(decoded,secret.length(),encoded,128);

        ui->secretEdit->setInputMask("NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN;");
        ui->secretEdit->setMaxLength(32);
        ui->secretEdit->setText(QString((char *)encoded));
        //qDebug() << QString((char *)encoded);

    }

}

void MainWindow::on_setToZeroButton_clicked()
{
    ui->counterEdit->setText("0");
}

void MainWindow::on_setToRandomButton_clicked()
{
    uint64_t counter=qrand();
    counter<<=16;
    counter+=qrand();
    counter<<=16;
    counter+=qrand();
    counter<<=16;
    counter+=qrand();
    //qDebug() << counter;
    ui->counterEdit->setText(QString(QByteArray::number(counter,16)));
}

void MainWindow::on_checkBox_2_toggled(bool checked)
{


}

void MainWindow::on_tokenIDCheckBox_toggled(bool checked)
{

    if (checked){
        ui->ompEdit->setEnabled(true);
        ui->ttEdit->setEnabled(true);
        ui->muiEdit->setEnabled(true);


    }
    else{
        ui->ompEdit->setEnabled(false);
        ui->ttEdit->setEnabled(false);
        ui->muiEdit->setEnabled(false);


    }
}

void MainWindow::on_writeGeneralConfigButton_clicked()
{
    QMessageBox msgBox;
    int res;
    uint8_t data[3];

    if (cryptostick->isConnected){

        data[0]=ui->numLockComboBox->currentIndex()-1;
        data[1]=ui->capsLockComboBox->currentIndex()-1;
        data[2]=ui->scrollLockComboBox->currentIndex()-1;

        res =cryptostick->writeGeneralConfig(data);

        if (res==0)
            msgBox.setText("Config written successfully!");
        else
            msgBox.setText("Error writing config!");

        msgBox.exec();

        Sleep::msleep(500);
        cryptostick->getStatus();
        displayCurrentGeneralConfig();

    }
    else{
        msgBox.setText("Crypto stick not connected!");
        msgBox.exec();
    }

}
