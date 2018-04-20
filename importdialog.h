#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>

namespace Ui {
class ImportDialog;
}

class ShadowWidget;

class ImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportDialog(QWidget *parent = 0);
    ~ImportDialog();

    void pop();

signals:
    void accountImported();

private slots:
    void on_cancelBtn_clicked();

    void on_pathBtn_clicked();

    void on_importBtn_clicked();

    void jsonDataUpdated(QString id);

    void on_privateKeyLineEdit_textChanged(const QString &arg1);

    void on_closeBtn_clicked();
private:
    void InitWidget();
    void InitStyle();
protected:
    void paintEvent(QPaintEvent *event);
private:
    Ui::ImportDialog *ui;
    ShadowWidget* shadowWidget;

};

#endif // IMPORTDIALOG_H
