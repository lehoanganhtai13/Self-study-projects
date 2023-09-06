#ifndef PLUGINS_DIALOG_H
#define PLUGINS_DIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QWidgetItem>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QStringList>
#include <QString>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QProcess>

namespace Ui {
class plugins_dialog;
}

class plugins_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit plugins_dialog(QWidget *parent = nullptr);
    ~plugins_dialog();

    void updatePlugin();
    QStringList getPluginList(int *index);

private slots:
    void on_load_clicked();

    void on_plugin_table_cellDoubleClicked(int row, int column);

    void on_remove_clicked();

    void on_open_clicked();

signals:
    void loadPlugin(bool);

private:
    Ui::plugins_dialog *ui;
    QStringList plugin_list;
    QByteArray load_list;
    int final_row;

    void updatePluginList();
    void showInFolder(QString dir);
};

#endif // PLUGINS_DIALOG_H
