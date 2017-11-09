#include "dialog.h"
#include "ui_dialog.h"
#include <QString>
#include <QMessageBox>

#include <Windows.h>
#include <WinUser.h>

#include <string>
using namespace std;

bool getIntFromStringForFbl(const std::string fbl, int& width, int& height)
{
    if (fbl.empty() || string::npos == fbl.find_first_of("X"))
    {
        return false;
    }

    int pos = fbl.find_first_of("X");
    if (string::npos == pos)
    {
        return false;
    }

    string widthStr = fbl.substr(0, pos);
    string heightStr = fbl.substr(pos + 1, fbl.size() - pos - 1);

    if (widthStr.empty() || heightStr.empty())
    {
        return false;
    }

    width = atoi(widthStr.c_str());
    height = atoi(heightStr.c_str());

    return true;
}

std::string getCurrentResolution(DWORD devId)
{
    DISPLAY_DEVICE myDevMode;
    myDevMode.cb = sizeof(DISPLAY_DEVICE);
    DEVMODE tmpDevMode;
    tmpDevMode.dmSize = sizeof(DEVMODE);
    BOOL status = EnumDisplayDevices(NULL, devId, &myDevMode, EDD_GET_DEVICE_INTERFACE_NAME);
    if (FALSE == status)
    {
        QString errorStr;
        errorStr = QString("EnumDisplayDevices failed! errno = %1!").arg(GetLastError());
        QMessageBox box(QMessageBox::Critical, QString("Error"), errorStr, QMessageBox::Ok);
        box.exec();
        return "";
    }
    else
    {
        BOOL status = EnumDisplaySettings(myDevMode.DeviceName, ENUM_CURRENT_SETTINGS, &tmpDevMode);
        if(0 == status)
        {
            QString errorStr;
            errorStr = QString("EnumDisplaySettings failed! errno = %1!").arg(GetLastError());
            QMessageBox box(QMessageBox::Critical, QString("Error"), errorStr, QMessageBox::Ok);
            box.exec();
            return "";
        }
    }

    QString qstrResolution;
    qstrResolution = QString("%1 X %2").arg(tmpDevMode.dmPelsWidth).arg(tmpDevMode.dmPelsHeight);
    return qstrResolution.toStdString();
}

bool setCurrentResolution(std::string strResolution, DWORD devID)
{
    int iWidth = 0;
    int iHeight = 0;
    if(getIntFromStringForFbl(strResolution, iWidth, iHeight))
    {
        DISPLAY_DEVICE myDevMode;
        myDevMode.cb = sizeof(DISPLAY_DEVICE);
        BOOL status = EnumDisplayDevices(NULL, devID, &myDevMode, EDD_GET_DEVICE_INTERFACE_NAME);
        if (FALSE == status)
        {
            QString errorStr;
            errorStr = QString("EnumDisplayDevices failed! errno = %1!").arg(GetLastError());
            QMessageBox box(QMessageBox::Critical, QString("Error"), errorStr, QMessageBox::Ok);
            box.exec();
            return false;
        }
        else
        {
            DEVMODE tmpDevMode;
            tmpDevMode.dmSpecVersion = DM_SPECVERSION;
            tmpDevMode.dmSize = sizeof(DEVMODE);
            tmpDevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
            tmpDevMode.dmPelsWidth = iWidth;
            tmpDevMode.dmPelsHeight = iHeight;

            LONG ret = ChangeDisplaySettingsEx(myDevMode.DeviceName, &tmpDevMode, NULL, 0, NULL);
            QString strStatus;
            switch(ret)
            {
            case DISP_CHANGE_SUCCESSFUL:
                break;
            case DISP_CHANGE_BADDUALVIEW:
                strStatus = "The settings change was unsuccessful because the system is DualView capable.";
                break;
            case DISP_CHANGE_BADFLAGS:
                strStatus = "An invalid set of flags was passed in.";
                break;
            case DISP_CHANGE_BADMODE:
                strStatus = "The graphics mode is not supported.";
                break;
            case DISP_CHANGE_BADPARAM:
                strStatus = "An invalid parameter was passed in. This can include an invalid flag or combination of flags.";
                break;
            case DISP_CHANGE_FAILED:
                strStatus = "The display driver failed the specified graphics mode.";
                break;
            case DISP_CHANGE_NOTUPDATED:
                strStatus = "Unable to write settings to the registry.";
                break;
            case DISP_CHANGE_RESTART:
                strStatus = "The computer must be restarted for the graphics mode to work.";
                break;
            }

            if(!strStatus.isEmpty())
            {
                QMessageBox statusBox(QMessageBox::Critical, QString("Error"), strStatus, QMessageBox::Ok);
                statusBox.exec();
                return false;
            }

            return true;
        }
    }

    return false;
}


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    QSize tmpSize = this->size();
    setFixedSize(tmpSize);

    if(ui->ok_btn_)
        QObject::connect(ui->ok_btn_, SIGNAL(clicked(bool)), this, SLOT(onOKBtnClicked(bool)));
    if(ui->cancel_btn_)
        QObject::connect(ui->cancel_btn_, SIGNAL(clicked(bool)), this, SLOT(onCancelBtnClicked(bool)));

    QObject::connect(ui->num_com_box_, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));

    std::string strResolution = getCurrentResolution(ui->num_com_box_->currentIndex());
    if(!strResolution.empty())
    {
        QString qstrResolution = QString::fromStdString(strResolution);
        if(ui->resolution_com_box_)
        {
            int ret = ui->resolution_com_box_->findText(qstrResolution);
            if(-1 == ret)
            {
                ui->resolution_com_box_->addItem(qstrResolution);
                ret = ui->resolution_com_box_->findText(qstrResolution);
                if(-1 != ret)
                {
                    ui->resolution_com_box_->setCurrentIndex(ret);
                }
                else
                {
                    QMessageBox box(QMessageBox::Critical, QString("Error"), QString("show current resolution failed!"), QMessageBox::Ok);
                    box.exec();
                }
            }
            else
            {
                ui->resolution_com_box_->setCurrentIndex(ret);
            }
        }
    }
    else
    {
        ui->num_com_box_->setCurrentIndex(0);
        ui->resolution_com_box_->setCurrentIndex(0);;
    }
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::onOKBtnClicked(bool status)
{
    if(ui->resolution_com_box_)
    {
        QString qstrResolution = ui->resolution_com_box_->currentText();
        string strResolution = qstrResolution.toStdString();
        setCurrentResolution(strResolution, ui->num_com_box_->currentIndex());
    }
}

void Dialog::onCancelBtnClicked(bool status)
{
    this->reject();
}

void Dialog::onCurrentIndexChanged(int index)
{
    std::string strResolution = getCurrentResolution(index);
    if(!strResolution.empty())
    {
        QString qstrResolution = QString::fromStdString(strResolution);
        if(ui->resolution_com_box_)
        {
            int ret = ui->resolution_com_box_->findText(qstrResolution);
            if(-1 == ret)
            {
                ui->resolution_com_box_->addItem(qstrResolution);
                ret = ui->resolution_com_box_->findText(qstrResolution);
                if(-1 != ret)
                {
                    ui->resolution_com_box_->setCurrentIndex(ret);
                }
                else
                {
                    QMessageBox box(QMessageBox::Critical, QString("Error"), QString("show current resolution failed!"), QMessageBox::Ok);
                    box.exec();
                }
            }
            else
            {
                ui->resolution_com_box_->setCurrentIndex(ret);
            }
        }
    }
}

