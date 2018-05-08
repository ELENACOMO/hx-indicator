#include "WithdrawOrderDialog.h"
#include "ui_WithdrawOrderDialog.h"

#include "wallet.h"

WithdrawOrderDialog::WithdrawOrderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WithdrawOrderDialog)
{
    ui->setupUi(this);

    setParent(UBChain::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet(CONTAINERWIDGET_STYLE);

    ui->closeBtn->setIconSize(QSize(12,12));
    ui->closeBtn->setIcon(QIcon(":/ui/wallet_ui/close.png"));
    ui->closeBtn->setStyleSheet("QToolButton{background-color:transparent;border:none;}"
                                "QToolButton:hover{background-color:rgb(208,228,255);}");

    ui->okBtn->setStyleSheet(OKBTN_STYLE);
    ui->cancelBtn->setStyleSheet(CANCELBTN_STYLE);

    connect( UBChain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

}

WithdrawOrderDialog::~WithdrawOrderDialog()
{
    delete ui;
}

void WithdrawOrderDialog::pop()
{
    move(0,0);
    exec();
}

void WithdrawOrderDialog::on_okBtn_clicked()
{

}

void WithdrawOrderDialog::on_cancelBtn_clicked()
{
    close();
}

void WithdrawOrderDialog::on_closeBtn_clicked()
{
    close();
}
