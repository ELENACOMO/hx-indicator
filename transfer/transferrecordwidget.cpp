#include "transferrecordwidget.h"
#include "ui_transferrecordwidget.h"

#include "wallet.h"

TransferRecordWidget::TransferRecordWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransferRecordWidget)
{
    ui->setupUi(this);

    ui->transferRecordTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->transferRecordTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->transferRecordTableWidget->setFocusPolicy(Qt::NoFocus);
//    ui->transferRecordTableWidget->setFrameShape(QFrame::NoFrame);
    ui->transferRecordTableWidget->setMouseTracking(true);
    ui->transferRecordTableWidget->setShowGrid(false);//隐藏表格线

    ui->transferRecordTableWidget->horizontalHeader()->setSectionsClickable(true);
    ui->transferRecordTableWidget->horizontalHeader()->setFixedHeight(30);
    ui->transferRecordTableWidget->horizontalHeader()->setVisible(true);
    ui->transferRecordTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);


//    ui->transferRecordTableWidget->setColumnWidth(0,90);
//    ui->transferRecordTableWidget->setColumnWidth(1,70);
//    ui->transferRecordTableWidget->setColumnWidth(2,100);
//    ui->transferRecordTableWidget->setColumnWidth(3,100);
//    ui->transferRecordTableWidget->setColumnWidth(4,110);
//    ui->transferRecordTableWidget->setColumnWidth(5,70);

    init();
}

TransferRecordWidget::~TransferRecordWidget()
{
    delete ui;
}

void TransferRecordWidget::init()
{
    ui->assetComboBox->addItem(tr("ALL"), "ALL");
    QStringList assetIds = UBChain::getInstance()->assetInfoMap.keys();
    foreach (QString assetId, assetIds)
    {
        ui->assetComboBox->addItem(UBChain::getInstance()->assetInfoMap.value(assetId).symbol, assetId);
    }
}

void TransferRecordWidget::showTransferRecord(QString _accountAddress, QString _assetId)
{
    accountAddress = _accountAddress;
    TransactionTypeIds typeIds = UBChain::getInstance()->transactionDB.getAccountTransactionTypeIdsByType(_accountAddress,TRANSACTION_TYPE_NORMAL);

    int size = typeIds.size();
    ui->transferRecordTableWidget->setRowCount(0);
    int rowCount = 0;

    for(int i = 0; i < size; i++)
    {
        QString transactionId = typeIds.at(size - i - 1).transactionId;
        TransactionStruct ts = UBChain::getInstance()->transactionDB.getTransactionStruct(transactionId);
        if(ts.type == -1)
        {
            qDebug() << "can not find transaction in transactionstruct db: " << transactionId;
            continue;
        }

        QJsonObject object = QJsonDocument::fromJson(ts.operationStr.toLatin1()).object();
        QJsonObject amountObject = object.take("amount").toObject();
        unsigned long long amount = jsonValueToULL(amountObject.take("amount"));
        QString amountAssetId = amountObject.take("asset_id").toString();
        AssetInfo amountAssetInfo = UBChain::getInstance()->assetInfoMap.value(amountAssetId);

        if(_assetId != "ALL"  &&  amountAssetId != _assetId)    continue;


        ui->transferRecordTableWidget->setRowCount(rowCount + 1);

        ui->transferRecordTableWidget->setItem(rowCount,0, new QTableWidgetItem(QString::number(ts.blockNum)));
        ui->transferRecordTableWidget->setItem(rowCount,4, new QTableWidgetItem(transactionId));

        QString fromAddress = object.take("from_addr").toString();
        QString toAddress   = object.take("to_addr").toString();
        if(fromAddress == _accountAddress)
        {
            ui->transferRecordTableWidget->setItem(rowCount,1, new QTableWidgetItem(toAddress));

            if(toAddress == _accountAddress)    // 如果是自己转自己
            {
                QTableWidgetItem* item = new QTableWidgetItem(getBigNumberString(amount, amountAssetInfo.precision) + " " + amountAssetInfo.symbol);
                ui->transferRecordTableWidget->setItem(rowCount,2, item);
                item->setTextColor(QColor(255,255,0));
            }
            else
            {
                // 如果是转出
                QTableWidgetItem* item = new QTableWidgetItem( "- " + getBigNumberString(amount, amountAssetInfo.precision) + " " + amountAssetInfo.symbol);
                ui->transferRecordTableWidget->setItem(rowCount,2, item);
                item->setTextColor(QColor(255,0,0));
            }
        }
        else
        {
            ui->transferRecordTableWidget->setItem(rowCount,1, new QTableWidgetItem(fromAddress));

            // 如果是转入
            QTableWidgetItem* item = new QTableWidgetItem( "+ " + getBigNumberString(amount, amountAssetInfo.precision) + " " + amountAssetInfo.symbol);
            ui->transferRecordTableWidget->setItem(rowCount,2, item);
            item->setTextColor(QColor(0,255,0));
        }


        QJsonObject feeObject = object.take("fee").toObject();
        unsigned long long feeAmount = jsonValueToULL(feeObject.take("amount"));
        QString feeAssetId = feeObject.take("asset_id").toString();
        AssetInfo feeAssetInfo = UBChain::getInstance()->assetInfoMap.value(feeAssetId);
        ui->transferRecordTableWidget->setItem(rowCount,3, new QTableWidgetItem(getBigNumberString(feeAmount, feeAssetInfo.precision)));

        ui->transferRecordTableWidget->setItem(rowCount,5, new QTableWidgetItem(tr("confirmed")));

        rowCount++;
    }
}

void TransferRecordWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(248,249,253),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(248,249,253),Qt::SolidPattern));
    painter.drawRect(0,0,770,530);
}

void TransferRecordWidget::on_assetComboBox_currentIndexChanged(const QString &arg1)
{
    showTransferRecord(accountAddress, ui->assetComboBox->currentData().toString());
}
