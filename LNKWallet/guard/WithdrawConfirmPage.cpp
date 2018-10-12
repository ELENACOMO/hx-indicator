#include "WithdrawConfirmPage.h"
#include "ui_WithdrawConfirmPage.h"

#include "wallet.h"
#include "ToolButtonWidget.h"
#include "commondialog.h"
#include "dialog/TransactionResultDialog.h"
#include "dialog/ErrorResultDialog.h"
#include "showcontentdialog.h"
#include "WithdrawInfoDialog.h"
#include "ColdHotTransferPage.h"

static const int ROWNUMBER = 6;
WithdrawConfirmPage::WithdrawConfirmPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WithdrawConfirmPage)
{
    ui->setupUi(this);

    connect( HXChain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->crosschainTransactionTableWidget->installEventFilter(this);
    ui->crosschainTransactionTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->crosschainTransactionTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->crosschainTransactionTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->crosschainTransactionTableWidget->setMouseTracking(true);
    ui->crosschainTransactionTableWidget->setShowGrid(false);//隐藏表格线

    ui->crosschainTransactionTableWidget->horizontalHeader()->setSectionsClickable(true);
    ui->crosschainTransactionTableWidget->horizontalHeader()->setVisible(true);
    ui->crosschainTransactionTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->crosschainTransactionTableWidget->setColumnWidth(0,110);
    ui->crosschainTransactionTableWidget->setColumnWidth(1,90);
    ui->crosschainTransactionTableWidget->setColumnWidth(2,100);
    ui->crosschainTransactionTableWidget->setColumnWidth(3,100);
    ui->crosschainTransactionTableWidget->setColumnWidth(4,80);
    ui->crosschainTransactionTableWidget->setColumnWidth(5,80);
    ui->crosschainTransactionTableWidget->setColumnWidth(6,80);
    ui->crosschainTransactionTableWidget->setStyleSheet(TABLEWIDGET_STYLE_1);

    ui->ethFinalTrxTableWidget->installEventFilter(this);
    ui->ethFinalTrxTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->ethFinalTrxTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->ethFinalTrxTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->ethFinalTrxTableWidget->setMouseTracking(true);
    ui->ethFinalTrxTableWidget->setShowGrid(false);//隐藏表格线

    ui->ethFinalTrxTableWidget->horizontalHeader()->setSectionsClickable(true);
    ui->ethFinalTrxTableWidget->horizontalHeader()->setVisible(true);
    ui->ethFinalTrxTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->ethFinalTrxTableWidget->setColumnWidth(0,500);
    ui->ethFinalTrxTableWidget->setColumnWidth(1,80);
    ui->ethFinalTrxTableWidget->setColumnWidth(2,80);
    ui->ethFinalTrxTableWidget->setStyleSheet(TABLEWIDGET_STYLE_1);

    ui->typeCurrentBtn->setCheckable(true);
    ui->typeWaitingBtn->setCheckable(true);
    ui->typeETHBtn->setCheckable(true);

    setStyleSheet(PUSHBUTTON_CHECK_STYLE);

    ui->typeCurrentBtn->adjustSize();
    ui->typeCurrentBtn->resize(ui->typeCurrentBtn->width(), 18);
    ui->typeWaitingBtn->adjustSize();
    ui->typeWaitingBtn->resize(ui->typeWaitingBtn->width(), 18);
    ui->typeWaitingBtn->move(ui->typeCurrentBtn->x() + ui->typeCurrentBtn->width() + 30, ui->typeWaitingBtn->y());
    ui->typeETHBtn->adjustSize();
    ui->typeETHBtn->resize(ui->typeETHBtn->width(), 18);
    ui->typeETHBtn->move(ui->typeWaitingBtn->x() + ui->typeWaitingBtn->width() + 30, ui->typeETHBtn->y());

    pageWidget = new PageScrollWidget();
    ui->stackedWidget->addWidget(pageWidget);
    connect(pageWidget,&PageScrollWidget::currentPageChangeSignal,this,&WithdrawConfirmPage::pageChangeSlot);

    ui->stackedWidget2->setCurrentIndex(0);

    blankWidget = new BlankDefaultWidget(ui->crosschainTransactionTableWidget);
    blankWidget->setTextTip(tr("There are no withdraw trxs to authorize currently!"));
    init();
}

WithdrawConfirmPage::~WithdrawConfirmPage()
{
    delete ui;
}

void WithdrawConfirmPage::init()
{
    connect(&httpManager,SIGNAL(httpReplied(QByteArray,int)),this,SLOT(httpReplied(QByteArray,int)));

    ui->accountComboBox->clear();
    QStringList accounts = HXChain::getInstance()->getMyFormalGuards();
    if(accounts.size() > 0)
    {
        ui->accountComboBox->addItems(accounts);

        if(accounts.contains(HXChain::getInstance()->currentAccount))
        {
            ui->accountComboBox->setCurrentText(HXChain::getInstance()->currentAccount);
        }
    }
    else
    {
        ui->label->hide();
        ui->accountComboBox->hide();

        QLabel* label = new QLabel(this);
        label->setGeometry(QRect(ui->accountComboBox->pos(), QSize(300,30)));
        label->setText(tr("There are no senator accounts in the wallet."));
    }

    HXChain::getInstance()->mainFrame->installBlurEffect(ui->crosschainTransactionTableWidget);

    ui->typeCurrentBtn->setChecked(true);

    ui->autoConfirmBtn->setCheckable(true);
    ui->autoConfirmBtn->setChecked(HXChain::getInstance()->autoWithdrawConfirm);
    ui->autoConfirmBtn->hide();

    fetchCrosschainTransactions();
}

void WithdrawConfirmPage::refresh()
{
    fetchCrosschainTransactions();
    fetchEthBalance();
}

void WithdrawConfirmPage::fetchCrosschainTransactions()
{
    HXChain::getInstance()->postRPC( "id-get_crosschain_transaction-" + QString::number(0), toJsonFormat( "get_crosschain_transaction",
                                                                                    QJsonArray() << 0));

    HXChain::getInstance()->postRPC( "id-get_crosschain_transaction-" + QString::number(1), toJsonFormat( "get_crosschain_transaction",
                                                                                    QJsonArray() << 1));

    HXChain::getInstance()->postRPC( "id-get_crosschain_transaction-" + QString::number(2), toJsonFormat( "get_crosschain_transaction",
                                                                                    QJsonArray() << 2));

    HXChain::getInstance()->postRPC( "id-get_crosschain_transaction-" + QString::number(7), toJsonFormat( "get_crosschain_transaction",
                                                                                    QJsonArray() << 7));
}

void WithdrawConfirmPage::jsonDataUpdated(QString id)
{
    if(id.startsWith("id-get_crosschain_transaction-"))
    {
        QString result = HXChain::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        result.prepend("{");
        result.append("}");

        QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
        QJsonObject jsonObject = parse_doucment.object();
        QJsonArray array = jsonObject.take("result").toArray();

        int type = id.mid(QString("id-get_crosschain_transaction-").size()).toInt();
        if(type == 1)
        {
            applyTransactionMap.clear();
            generatedTransactionMap.clear();
        }
        else if(type == 2)
        {
            signTransactionMap.clear();
        }
        else if(type == 0)
        {
            pendingApplyTransactionMap.clear();
        }
        else if(type == 7)
        {
            ethFinalTrxMap.clear();
            foreach (QJsonValue v, array)
            {
                QJsonArray array2 = v.toArray();
                QJsonObject object = array2.at(1).toObject();
                QJsonArray operationArray = object.take("operations").toArray();
                QJsonArray array3 = operationArray.at(0).toArray();
                int operationType = array3.at(0).toInt();
                QJsonObject operationObject = array3.at(1).toObject();

                if( operationType == TRANSACTION_TYPE_WITHDRAW_FINAL)
                {
                    QJsonObject ethObject = operationObject.value("cross_chain_trx").toObject();
                    ETHFinalTrx eft;
                    eft.trxId = array2.at(0).toString();
                    eft.signer = ethObject.value("signer").toString();
                    eft.nonce = ethObject.value("nonce").toString();

                    ethFinalTrxMap.insert(eft.trxId, eft);
                }
            }

            showEthFinalTrxs();
            return;
        }


        foreach (QJsonValue v, array)
        {
            QJsonArray array2 = v.toArray();
            QJsonObject object = array2.at(1).toObject();
            QJsonArray operationArray = object.take("operations").toArray();
            QJsonArray array3 = operationArray.at(0).toArray();
            int operationType = array3.at(0).toInt();
            QJsonObject operationObject = array3.at(1).toObject();

            if(operationType == TRANSACTION_TYPE_WITHDRAW_CROSSCHAIN)
            {
                GeneratedTransaction gt;
                gt.trxId = array2.at(0).toString();
                QJsonArray ccwArray = operationObject.take("ccw_trx_ids").toArray();
                foreach (QJsonValue v, ccwArray)
                {
                    gt.ccwTrxIds += v.toString();
                }

                generatedTransactionMap.insert(gt.trxId, gt);
            }
            else if(operationType == TRANSACTION_TYPE_WITHDRAW)
            {
                ApplyTransaction at;
                at.trxId            = array2.at(0).toString();
                at.expirationTime   = object.take("expiration").toString();
                at.withdrawAddress  = operationObject.take("withdraw_account").toString();
                at.amount           = operationObject.take("amount").toString();
                at.assetSymbol      = operationObject.take("asset_symbol").toString();
                at.assetId          = operationObject.take("asset_id").toString();
                at.crosschainAddress  = operationObject.take("crosschain_account").toString();
                at.memo             = operationObject.take("memo").toString();

                if(type == 0)
                {
                    pendingApplyTransactionMap.insert(at.trxId, at);
                }
                else
                {
                    applyTransactionMap.insert(at.trxId, at);
                }
            }
            else if(operationType == TRANSACTION_TYPE_WITHDRAW_SIGN)
            {
                SignTransaction st;
                st.trxId            = array2.at(0).toString();
                st.generatedTrxId   = operationObject.take("ccw_trx_id").toString();
                st.guardAddress     = operationObject.take("guard_address").toString();

                signTransactionMap.insert(st.trxId, st);
            }
        }

        if(type != 2)
        {
            showCrosschainTransactions();
        }
        else
        {
            refreshCrosschainTransactionsState();
        }

        return;
    }

    if(id == "id-senator_sign_crosschain_transaction")
    {
        QString result = HXChain::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {            
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText(tr("Transaction of senator-sign-crosschain has been sent out!"));
            commonDialog.pop();
        }
        else if(result.startsWith("\"error\":"))
        {
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Failed!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }

        return;
    }

    if(id.startsWith("WithdrawConfirmPage-get_multi_address_obj-ETH-"))
    {
        QString accountId = id.mid(QString("WithdrawConfirmPage-get_multi_address_obj-ETH-").size());
        AccountInfo accountInfo = HXChain::getInstance()->accountInfoMap.value(ui->accountComboBox->currentText());
        if(accountId != accountInfo.id)     return;

        QString result = HXChain::getInstance()->jsonDataValue(id);
        result.prepend("{");
        result.append("}");

        QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
        QJsonObject jsonObject = parse_doucment.object();
        QJsonObject ethMultisigObject = jsonObject.value("result").toArray().at(0).toObject();

        QString hotAddress = ethMultisigObject.value("new_address_hot").toString();
        QString coldAddress = ethMultisigObject.value("new_address_cold").toString();

        QJsonObject object;
        object.insert("jsonrpc","2.0");
        object.insert("id",1011);
        object.insert("method","Zchain.Address.GetBalance");
        QJsonObject paramObject;
        paramObject.insert("chainId","ETH");
        paramObject.insert("addr",hotAddress);
        object.insert("params",paramObject);
        httpManager.post(HXChain::getInstance()->middlewarePath,QJsonDocument(object).toJson());

        paramObject.insert("addr",coldAddress);
        object.insert("params",paramObject);
        object.insert("id",1012);
        httpManager.post(HXChain::getInstance()->middlewarePath,QJsonDocument(object).toJson());

        return;
    }

    if(id == "id-senator_sign_eths_final_trx")
    {
        QString result = HXChain::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":null"))
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText(tr("Transaction of senator-sign-eth-withdraw-final has been sent out!"));
            commonDialog.pop();
        }
        else if(result.startsWith("\"error\":"))
        {
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Failed!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }

        return;
    }
}

void WithdrawConfirmPage::httpReplied(QByteArray _data, int _status)
{
    QJsonObject object  = QJsonDocument::fromJson(_data).object();
    int id = object.value("id").toInt();
    QJsonObject resultObject = object.value("result").toObject();
    QString assetSymbol = resultObject.value("chainId").toString().toUpper();
    QString balance = resultObject.value("balance").toString();
    QString address = resultObject.value("address").toString();

    if(id == 1011)
    {
        ui->hotAddressBalanceLabel->setText( QString("%1 (%2 ETH)").arg(address).arg(balance));
    }
    else if(id == 1012)
    {
        ui->coldAddressBalanceLabel->setText( QString("%1 (%2 ETH)").arg(address).arg(balance));
    }
}

void WithdrawConfirmPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(229,226,240),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(229,226,240),Qt::SolidPattern));
    painter.drawRect(rect());

    painter.setPen(QPen(QColor(229,226,240),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(243,241,250),Qt::SolidPattern));
    painter.drawRect(0,87,this->width(),24);
}

void WithdrawConfirmPage::on_crosschainTransactionTableWidget_cellClicked(int row, int column)
{
    if(column == 5)
    {
        if(ui->crosschainTransactionTableWidget->item(row,0))
        {
            QString trxId = ui->crosschainTransactionTableWidget->item(row,0)->data(Qt::UserRole).toString();

            qDebug() << trxId;

            WithdrawInfoDialog withdrawInfoDialog(this);
            withdrawInfoDialog.setTrxId(trxId);
            withdrawInfoDialog.pop();
        }

        return;
    }

    if(column == 6)
    {
        if(currentType != 1)    return;
        if(ui->crosschainTransactionTableWidget->item(row,0) && ui->crosschainTransactionTableWidget->item(row,4))
        {
            if(ui->crosschainTransactionTableWidget->item(row,4)->text() == tr("signed"))
            {
                CommonDialog commonDialog(CommonDialog::OkOnly);
                commonDialog.setText(tr("%1 has already signed!").arg(ui->accountComboBox->currentText()));
                commonDialog.pop();

                return;
            }

            QString trxId = ui->crosschainTransactionTableWidget->item(row,0)->data(Qt::UserRole).toString();

            QString generatedTrxId = lookupGeneratedTrxByApplyTrxId(trxId);

            if(!generatedTrxId.isEmpty() && !ui->accountComboBox->currentText().isEmpty())
            {
                HXChain::getInstance()->postRPC( "id-senator_sign_crosschain_transaction", toJsonFormat( "senator_sign_crosschain_transaction",
                                                 QJsonArray() << generatedTrxId << ui->accountComboBox->currentText()));

            }
        }

        return;
    }
}

void WithdrawConfirmPage::showCrosschainTransactions()
{
    if(currentType == 1)
    {
        QStringList keys = applyTransactionMap.keys();
        int size = keys.size();
        ui->crosschainTransactionTableWidget->setRowCount(0);
        ui->crosschainTransactionTableWidget->setRowCount(size);

        for(int i = 0; i < size; i++)
        {
            ui->crosschainTransactionTableWidget->setRowHeight(i,40);

            ApplyTransaction at = applyTransactionMap.value(keys.at(i));

            ui->crosschainTransactionTableWidget->setItem(i, 0, new QTableWidgetItem(toLocalTime(at.expirationTime)));
            ui->crosschainTransactionTableWidget->item(i,0)->setData(Qt::UserRole, at.trxId);

            ui->crosschainTransactionTableWidget->setItem(i, 1, new QTableWidgetItem(at.amount + " " + at.assetSymbol));

            ui->crosschainTransactionTableWidget->setItem(i, 2, new QTableWidgetItem(at.withdrawAddress));

            ui->crosschainTransactionTableWidget->setItem(i, 3, new QTableWidgetItem(at.crosschainAddress));

            ui->crosschainTransactionTableWidget->setItem(i, 4, new QTableWidgetItem(tr("checking")));

            ui->crosschainTransactionTableWidget->setItem(i, 5, new QTableWidgetItem(tr("check")));
            ToolButtonWidget *toolButton = new ToolButtonWidget();
            toolButton->setText(ui->crosschainTransactionTableWidget->item(i,5)->text());
            ui->crosschainTransactionTableWidget->setCellWidget(i,5,toolButton);
            connect(toolButton,&ToolButtonWidget::clicked,std::bind(&WithdrawConfirmPage::on_crosschainTransactionTableWidget_cellClicked,this,i,5));

            ui->crosschainTransactionTableWidget->setItem(i, 6, new QTableWidgetItem(tr("sign")));
            ToolButtonWidget *toolButton2 = new ToolButtonWidget();
            toolButton2->setText(ui->crosschainTransactionTableWidget->item(i,6)->text());
            ui->crosschainTransactionTableWidget->setCellWidget(i,6,toolButton2);
            connect(toolButton2,&ToolButtonWidget::clicked,std::bind(&WithdrawConfirmPage::on_crosschainTransactionTableWidget_cellClicked,this,i,6));

        }

        tableWidgetSetItemZebraColor(ui->crosschainTransactionTableWidget);
    }
    else if( currentType == 0)
    {
        QStringList keys = pendingApplyTransactionMap.keys();
        int size = keys.size();
        ui->crosschainTransactionTableWidget->setRowCount(0);
        ui->crosschainTransactionTableWidget->setRowCount(size);

        for(int i = 0; i < size; i++)
        {
            ui->crosschainTransactionTableWidget->setRowHeight(i,40);

            ApplyTransaction at = pendingApplyTransactionMap.value(keys.at(i));

            ui->crosschainTransactionTableWidget->setItem(i, 0, new QTableWidgetItem(toLocalTime(at.expirationTime)));
            ui->crosschainTransactionTableWidget->item(i,0)->setData(Qt::UserRole, at.trxId);

            ui->crosschainTransactionTableWidget->setItem(i, 1, new QTableWidgetItem(at.amount + " " + at.assetSymbol));

            ui->crosschainTransactionTableWidget->setItem(i, 2, new QTableWidgetItem(at.withdrawAddress));

            ui->crosschainTransactionTableWidget->setItem(i, 3, new QTableWidgetItem(at.crosschainAddress));

            ui->crosschainTransactionTableWidget->setItem(i, 4, new QTableWidgetItem(tr("waiting")));

            ui->crosschainTransactionTableWidget->setItem(i, 5, new QTableWidgetItem(tr("check")));
            ToolButtonWidget *toolButton = new ToolButtonWidget();
            toolButton->setText(ui->crosschainTransactionTableWidget->item(i,5)->text());
            ui->crosschainTransactionTableWidget->setCellWidget(i,5,toolButton);
            connect(toolButton,&ToolButtonWidget::clicked,std::bind(&WithdrawConfirmPage::on_crosschainTransactionTableWidget_cellClicked,this,i,5));

        }

        tableWidgetSetItemZebraColor(ui->crosschainTransactionTableWidget);
    }
    int page = (ui->crosschainTransactionTableWidget->rowCount()%ROWNUMBER==0 && ui->crosschainTransactionTableWidget->rowCount() != 0) ?
                ui->crosschainTransactionTableWidget->rowCount()/ROWNUMBER : ui->crosschainTransactionTableWidget->rowCount()/ROWNUMBER+1;
    pageWidget->SetTotalPage(page);
    pageWidget->setShowTip(ui->crosschainTransactionTableWidget->rowCount(),ROWNUMBER);
    pageChangeSlot(0);

    pageWidget->setVisible(0 != ui->crosschainTransactionTableWidget->rowCount());
    blankWidget->setVisible(ui->crosschainTransactionTableWidget->rowCount() == 0);


}

void WithdrawConfirmPage::refreshCrosschainTransactionsState()
{
    if(currentType != 1)    return;

    int rowCount = ui->crosschainTransactionTableWidget->rowCount();
    for(int i = 0; i < rowCount; i++)
    {
        QString trxId = ui->crosschainTransactionTableWidget->item(i,0)->data(Qt::UserRole).toString();
        QString generatedTrxId = lookupGeneratedTrxByApplyTrxId(trxId);
        QStringList guardAddresses = lookupSignedGuardsByGeneratedTrxId(generatedTrxId);

        QString currentAddress = HXChain::getInstance()->accountInfoMap.value(ui->accountComboBox->currentText()).address;
        if(guardAddresses.contains(currentAddress))
        {
            ui->crosschainTransactionTableWidget->item(i,4)->setText(tr("signed"));
        }
        else
        {
            ui->crosschainTransactionTableWidget->item(i,4)->setText(tr("unsigned"));
        }
    }
}

void WithdrawConfirmPage::fetchEthBalance()
{
    AccountInfo accountInfo = HXChain::getInstance()->accountInfoMap.value(ui->accountComboBox->currentText());

    HXChain::getInstance()->postRPC( "WithdrawConfirmPage-get_multi_address_obj-ETH-" + accountInfo.id, toJsonFormat( "get_multi_address_obj",
                                                                                                     QJsonArray() << "ETH" << accountInfo.id));

}

void WithdrawConfirmPage::showEthFinalTrxs()
{
    QStringList keys = ethFinalTrxMap.keys();
    int size = keys.size();
    ui->ethFinalTrxTableWidget->setRowCount(0);
    ui->ethFinalTrxTableWidget->setRowCount(size);

    for(int i = 0; i < size; i++)
    {
        ui->ethFinalTrxTableWidget->setRowHeight(i,40);

        ETHFinalTrx eft = ethFinalTrxMap.value(keys.at(i));

        ui->ethFinalTrxTableWidget->setItem(i, 0, new QTableWidgetItem(eft.signer));
        ui->ethFinalTrxTableWidget->item(i,0)->setData(Qt::UserRole, eft.trxId);

        ui->ethFinalTrxTableWidget->setItem(i, 1, new QTableWidgetItem(eft.nonce));

        ui->ethFinalTrxTableWidget->setItem(i, 2, new QTableWidgetItem(tr("sign")));
        ToolButtonWidget *toolButton = new ToolButtonWidget();
        toolButton->setText(ui->ethFinalTrxTableWidget->item(i,2)->text());
        ui->ethFinalTrxTableWidget->setCellWidget(i,2,toolButton);
        connect(toolButton,&ToolButtonWidget::clicked,std::bind(&WithdrawConfirmPage::on_ethFinalTrxTableWidget_cellClicked,this,i,2));

    }

    tableWidgetSetItemZebraColor(ui->ethFinalTrxTableWidget);
}

QString WithdrawConfirmPage::lookupGeneratedTrxByApplyTrxId(QString applyTrxId)
{
    QString generatedTrxId;
    QStringList keys = generatedTransactionMap.keys();
    foreach (QString key, keys)
    {
        GeneratedTransaction gt = generatedTransactionMap.value(key);
        if(gt.ccwTrxIds.contains(applyTrxId))
        {
            generatedTrxId = gt.trxId;
            break;
        }
    }

    return generatedTrxId;
}

QStringList WithdrawConfirmPage::lookupSignedGuardsByGeneratedTrxId(QString generatedTrxId)
{
    QStringList result;
    QStringList keys = signTransactionMap.keys();

    foreach (QString key, keys)
    {
        SignTransaction st = signTransactionMap.value(key);
        if(st.generatedTrxId == generatedTrxId)
        {
            result += st.guardAddress;
        }
    }

    return result;
}

void WithdrawConfirmPage::on_crosschainTransactionTableWidget_cellPressed(int row, int column)
{
    if( column == 2 || column == 3)
    {
        ShowContentDialog showContentDialog( ui->crosschainTransactionTableWidget->item(row, column)->text(),this);

        int x = ui->crosschainTransactionTableWidget->columnViewportPosition(column) + ui->crosschainTransactionTableWidget->columnWidth(column) / 2
                - showContentDialog.width() / 2;
        int y = ui->crosschainTransactionTableWidget->rowViewportPosition(row) - 10 + ui->crosschainTransactionTableWidget->horizontalHeader()->height();

        showContentDialog.move( ui->crosschainTransactionTableWidget->mapToGlobal( QPoint(x, y)));
        showContentDialog.exec();

        return;
    }
}

void WithdrawConfirmPage::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    ui->hotAddressBalanceLabel->clear();
    ui->coldAddressBalanceLabel->clear();
    refresh();
}

void WithdrawConfirmPage::on_typeCurrentBtn_clicked()
{
    currentType = 1;
    ui->stackedWidget2->setCurrentIndex(0);
    ui->typeCurrentBtn->setChecked(true);
    ui->typeWaitingBtn->setChecked(false);
    ui->typeETHBtn->setChecked(false);
    showCrosschainTransactions();
}

void WithdrawConfirmPage::on_typeWaitingBtn_clicked()
{
    currentType = 0;
    ui->stackedWidget2->setCurrentIndex(0);
    ui->typeCurrentBtn->setChecked(false);
    ui->typeWaitingBtn->setChecked(true);
    ui->typeETHBtn->setChecked(false);
    showCrosschainTransactions();
}

void WithdrawConfirmPage::on_typeETHBtn_clicked()
{
    currentType = 2;
    ui->stackedWidget2->setCurrentIndex(1);
    ui->typeCurrentBtn->setChecked(false);
    ui->typeWaitingBtn->setChecked(false);
    ui->typeETHBtn->setChecked(true);
}

void WithdrawConfirmPage::pageChangeSlot(unsigned int page)
{
    for(int i = 0;i < ui->crosschainTransactionTableWidget->rowCount();++i)
    {
        if(i < page*ROWNUMBER)
        {
            ui->crosschainTransactionTableWidget->setRowHidden(i,true);
        }
        else if(page * ROWNUMBER <= i && i < page*ROWNUMBER + ROWNUMBER)
        {
            ui->crosschainTransactionTableWidget->setRowHidden(i,false);
        }
        else
        {
            ui->crosschainTransactionTableWidget->setRowHidden(i,true);
        }
    }
}

void WithdrawConfirmPage::on_autoConfirmBtn_clicked()
{
    HXChain::getInstance()->autoWithdrawConfirm = !HXChain::getInstance()->autoWithdrawConfirm;
    HXChain::getInstance()->configFile->setValue("/settings/autoWithdrawConfirm", HXChain::getInstance()->autoWithdrawConfirm);
    ui->autoConfirmBtn->setChecked(HXChain::getInstance()->autoWithdrawConfirm);
}



void WithdrawConfirmPage::on_ethFinalTrxTableWidget_cellClicked(int row, int column)
{
    if(column == 2)
    {
        if(ui->ethFinalTrxTableWidget->item(row,0))
        {
            QString trxId = ui->ethFinalTrxTableWidget->item(row,0)->data(Qt::UserRole).toString();

            qDebug() << trxId;

            HXChain::getInstance()->postRPC( "id-senator_sign_eths_final_trx",
                                             toJsonFormat( "senator_sign_eths_final_trx",
                                                           QJsonArray() << trxId << ui->accountComboBox->currentText()));

        }

        return;
    }
}
