#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include "language.h"

#define PRO_VERSION "V1.03"
#define BUILT_DATE "2017-08-27"
void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,NULL,QString(tr("\nVersion: %1\n"
                                            "\nBuilt on %2\n"
                                            "\n\t---evening.wen\n"))
                           .arg(PRO_VERSION).arg(BUILT_DATE));
}
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_pSerial = new SerialObj;
    m_pThread = new QThread;

    connect(this,&MainWindow::setSerialPrefixSuffix,m_pSerial,&SerialObj::setSerialPrefixSuffix);
    connect(this,&MainWindow::openSerial,m_pSerial,&SerialObj::openSerial);
    connect(this,&MainWindow::closeSerial,m_pSerial,&SerialObj::closeSerial);
    connect(this,&MainWindow::sendSerialData,m_pSerial,&SerialObj::sendSerialData);
    connect(this,&MainWindow::setCommFileName,m_pSerial,&SerialObj::setCommFileName);
    connect(this,&MainWindow::setTimingFileName,m_pSerial,&SerialObj::setTimingFileName);
    connect(ui->comboBox_split,&QComboBox::currentTextChanged,m_pSerial,&SerialObj::setRegExpPattern);

    connect(m_pSerial,&SerialObj::serialIsOpen,this,&MainWindow::serialIsOpen);
    connect(m_pSerial,&SerialObj::log,this,&MainWindow::log);

    connect(m_pThread,&QThread::started,m_pSerial,&SerialObj::init);

    init();

    m_pSerial->moveToThread(m_pThread);
    m_pThread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    m_strLogFileName = "";
    m_strPrefix = "";
    m_strSuffix = "";
    getPortName();
    m_bSerialIsOpen = false;
    ui->comboBox_split->setCurrentIndex(1);
}

void MainWindow::getPortName()
{
    QList<QSerialPortInfo> portInfoList = QSerialPortInfo::availablePorts();
    if(portInfoList.isEmpty())
    {
//        ui->comboBox_portName->addItem("None");
        return;
    }
    foreach (QSerialPortInfo info, portInfoList)
    {
        ui->comboBox_portName->addItem(info.portName());
    }
}

void MainWindow::on_pushButton_refresh_clicked()
{
    ui->comboBox_portName->clear();
    getPortName();
}

void MainWindow::log(const QString &msg, SHOW_MSG index)
{
    QString time = QDateTime::currentDateTime().toString("yyyyMMdd_hh:mm:ss_zzz");
    QString strMsgPre;
    switch (index) {
    case SHOW_SENDER:
        strMsgPre = "Send";
        break;
    case SHOW_RECEIVE:
        strMsgPre = "Receive";
        break;
    case SHOW_NULL:
        strMsgPre = "";
        break;
    default:
        strMsgPre = "";
        break;
    }
    QString strMsg = QString("%1 %2:%3").arg(time).arg(strMsgPre).arg(msg);
    if(!ui->checkBox_pauseShow->isChecked())
    {
//        ui->textBrowser_showMsg->moveCursor(QTextCursor::End);
//        ui->textBrowser_showMsg->insertPlainText(strMsg);
//        ui->textBrowser_showMsg->moveCursor(QTextCursor::End);
        ui->textBrowser_showMsg->append(strMsg);
    }
    if(ui->checkBox_saveLog->isChecked())
    {
        saveLog(strMsg);
    }
}

void MainWindow::saveLog(QString &msg)
{
    if(!m_strLogFileName.isEmpty())
    {
        QFile file(m_strLogFileName);
        if(file.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&file);
            out << msg;
            if(!file.flush())
            {
                qWarning("log文件刷新失败!");
            }
            file.close();
        }
    }
}

void MainWindow::on_checkBox_saveLog_clicked()
{
    if(ui->checkBox_saveLog->isChecked())
    {
        m_strLogFileName = QFileDialog::getSaveFileName(this,tr("选择存储路径"),
                                                        "..\\Message_log.txt",
                                                        tr("Text files (*.txt)"));
        if(m_strLogFileName.isEmpty())
        {
            m_strLogFileName = "";
            ui->checkBox_saveLog->setChecked(false);
            return;
        }
        log(QString(tr("通讯信息将保存到文件 %1当中!")).arg(m_strLogFileName),SHOW_NULL);
    }
    else
    {
        m_strLogFileName = "";
    }
}

void MainWindow::on_pushButton_set_clicked()
{
    QString strPrefix = ui->comboBox_prefix->currentText();
    QString strSuffix = ui->comboBox_suffix->currentText();
    log(QString(tr("\n设置数据规则\n前缀: %1\n后缀: %2")
                .arg(strPrefix).arg(strSuffix)),SHOW_NULL);
    strSuffix.replace("\\r","\r");
    strSuffix.replace("\\n","\n");
    m_strPrefix = strPrefix;
    m_strSuffix = strSuffix;
    emit setSerialPrefixSuffix(m_strPrefix,m_strSuffix);
}

void MainWindow::on_pushButton_open_close_clicked()
{
    if(!m_bSerialIsOpen)
    {
        QString strPortName = ui->comboBox_portName->currentText();
        QString strBaudRate = ui->comboBox_baudRate->currentText();
        QString strDataBits = ui->comboBox_dataBit->currentText();
        QString strParityBits = ui->comboBox_parityBit->currentText();
        QString strStopBits = ui->comboBox_stopBit->currentText();
        bool bOk;
        bool bOk1;
        int iBaudRate = strBaudRate.toInt(&bOk,10);
        int iDataBits = strDataBits.toInt(&bOk1,10);
        if(!bOk || !bOk1)
            return;
        SerialPar serialPar;
        serialPar.portName = strPortName;
        serialPar.baudRate = iBaudRate;
        serialPar.dataBit = iDataBits;
        serialPar.parityBit = strParityBits;
        serialPar.stopBit = strStopBits;
        serialPar.setDTR = true;
        serialPar.setRTS = true;
        emit openSerial(serialPar);
    }
    else
    {
        emit closeSerial();
    }
}

void MainWindow::on_pushButton_send_clicked()
{
    if(m_bSerialIsOpen)
    {
        QString strSendMsg = ui->lineEdit_input->text();
        if(!strSendMsg.isEmpty())
        {
            emit sendSerialData(strSendMsg);
        }
    }
    else
    {
        log(tr("串口未打开"),SHOW_NULL);
    }
}

void MainWindow::on_pushButton_clearShow_clicked()
{
    ui->textBrowser_showMsg->clear();
}

void MainWindow::serialIsOpen(const bool &isOpen)
{
    m_bSerialIsOpen = isOpen;
    if(m_bSerialIsOpen)
    {
        ui->pushButton_open_close->setText(tr("关闭串口"));
        log(tr("串口已打开"),SHOW_NULL);
    }
    else
    {
        ui->pushButton_open_close->setText(tr("打开串口"));
        log(tr("串口已关闭或被占用"),SHOW_NULL);
    }
    ui->comboBox_portName->setDisabled(isOpen);
    ui->comboBox_baudRate->setDisabled(isOpen);
    ui->comboBox_dataBit->setDisabled(isOpen);
    ui->comboBox_parityBit->setDisabled(isOpen);
    ui->comboBox_stopBit->setDisabled(isOpen);
    ui->pushButton_refresh->setDisabled(isOpen);
}

void MainWindow::on_pushButton_loadCommFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("load"),
                                                     "..\\Message_list.txt",
                                                     tr("Text files (*.txt)"));
    if(!fileName.isEmpty())
    {
        log(tr("加载通讯文件 %1成功!").arg(fileName),SHOW_NULL);
    }
    else
    {
        log(tr("已取消加载通讯文件!"),SHOW_NULL);
    }
    emit setCommFileName(fileName);
}

void MainWindow::on_pushButton_loadTimerFile_clicked()
{
    QString timerFileName = QFileDialog::getOpenFileName(this,tr("load"),
                                                       "..\\timer_list.txt",
                                                       tr("Text Files (*.txt)"));
    if(!timerFileName.isEmpty())
    {
        log(tr("加载定时文件 %1成功!").arg(timerFileName),SHOW_NULL);
    }
    else
    {
        log(tr("已取消加载定时文件!"),SHOW_NULL);
    }
    emit setTimingFileName(timerFileName);
}
