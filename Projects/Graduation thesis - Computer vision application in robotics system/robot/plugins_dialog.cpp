#include "plugins_dialog.h"
#include "ui_plugins_dialog.h"

plugins_dialog::plugins_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::plugins_dialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Plugins");
    ui->plugin_table->setColumnWidth(0, 160);
    ui->plugin_table->setColumnWidth(1, 60);
    ui->load1->setGeometry(210,53,16,16);
    ui->load2->setGeometry(210,83,16,16);
    ui->load3->setGeometry(210,113,16,16);
    ui->load4->setGeometry(210,143,16,16);
    ui->load5->setGeometry(210,173,16,16);
    ui->load6->setGeometry(210,203,16,16);
    ui->load7->setGeometry(210,233,16,16);
    ui->load1->setVisible(false);
    ui->load2->setVisible(false);
    ui->load3->setVisible(false);
    ui->load4->setVisible(false);
    ui->load5->setVisible(false);
    ui->load6->setVisible(false);
    ui->load7->setVisible(false);


    updatePluginList();
    QFile plugin_file("C:\\Autoss\\Draft\\Plugins\\plugin_list.txt");
    if(plugin_file.open(QIODevice::ReadOnly)){
        QTextStream temp_str(&plugin_file);
        int row, column;
        column = 0;
        row = 1;
        while(!temp_str.atEnd()){
            for(int j = 1; j < 8; j++){
                ui->plugin_table->setCurrentCell(j, column);
                qDebug() << ui->plugin_table->currentItem()->text();
                if(ui->plugin_table->currentItem()->text() == ""){
                    row = j;
                    break;
                }
            }
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setText(temp_str.readLine());
            ui->plugin_table->setItem(row, column, item);
            switch (row) {
            case 1:
//                ui->load1->setStyleSheet("QLineEdit {background-color: rgb(255, 0, 0);border: 2px solid gray;border-radius: 8px;}");
                ui->load1->setVisible(true);
                break;
            case 2:
                ui->load2->setVisible(true);
                break;
            case 3:
                ui->load3->setVisible(true);
                break;
            case 4:
                ui->load4->setVisible(true);
                break;
            case 5:
                ui->load5->setVisible(true);
                break;
            case 6:
                ui->load6->setVisible(true);
                break;
            case 7:
                ui->load7->setVisible(true);
                break;
            }
        }
        QFile load_file("C:\\Autoss\\Draft\\Plugins\\load_list.txt");
        if(load_file.open(QIODevice::ReadOnly)){
            temp_str.setDevice(&load_file);
            QString array = temp_str.readLine();
            for(int i = 0; i < 7; i++){
                load_list.append(array.split(",").at(i).toInt());
            }
            load_file.flush();
            load_file.close();
            for(int i = 0; i < 7; i++){
                if(load_list.at(i) == 1){
                    switch (i + 1) {
                    case 1:
                        ui->load1->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
                        break;
                    case 2:
                        ui->load2->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
                        break;
                    case 3:
                        ui->load3->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
                        break;
                    case 4:
                        ui->load4->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
                        break;
                    case 5:
                        ui->load5->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
                        break;
                    case 6:
                        ui->load6->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
                        break;
                    case 7:
                        ui->load7->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
                        break;
                    }
                }
            }
        }
        ui->plugin_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
//    ui->plugin_table->currentItem()->checkState();
//    ui->plugin_table->itemSelectionChanged();
}

plugins_dialog::~plugins_dialog()
{
    delete ui;
}

void plugins_dialog::updatePlugin(){
    for(int i = 0; i < 7; i++){
        if(load_list.at(i) == 1){
            final_row = i;
            emit loadPlugin(true);
        }
    }
}

void plugins_dialog::on_load_clicked(){
    QString plugin_load;
    plugin_load = QFileDialog::getOpenFileName(this, tr("Plugin folder"), QCoreApplication::applicationDirPath() + "/plugins", "dll files (*.dll);; All files (*.*)");
    if(plugin_load.size() == 0){
        QMessageBox::warning(this, "Load plugin error", "Can't file .dll file");
        return;
    }

    if(plugin_load.split("/").last().contains("-")){
        QMessageBox::warning(this, "Load plugin error", "Plugin file name contains hyphens");
        return;
    }

    int row, column;
    column = 0;
    row = 1;

    if(plugin_list.size() == 7){
        QMessageBox::warning(this, "Load plugin error", "Out of range           ");
        return;
    }

    // Update plugin list file
    QFile plugin_file("C:\\Autoss\\Draft\\Plugins\\plugin_list.txt");
    if(plugin_file.open(QIODevice::ReadOnly)){
        QTextStream str(&plugin_file);
        QString tmp = str.readAll();
        if(tmp.contains(plugin_load.split("/").last())){
            QMessageBox::warning(this, "Load plugin error", "Plugin is already existed");
            return;
        }
        tmp = tmp + plugin_load.split("/").last() + "\r\n";
        plugin_file.flush();
        plugin_file.close();
        if(plugin_file.open(QIODevice::WriteOnly)){
            str.setDevice(&plugin_file);
            str << tmp;
            plugin_file.flush();
            plugin_file.close();
        }
    }

    for(int j = 1; j < 8; j++){
        ui->plugin_table->setCurrentCell(j, column);
        qDebug() << ui->plugin_table->currentItem()->text();
        if(ui->plugin_table->currentItem()->text() == ""){
            row = j;
            break;
        }
    }
    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText(plugin_load.split("/").last());
    ui->plugin_table->setItem(row, column, item);
    ui->plugin_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    ui->plugin_table->setCurrentCell(row, column + 1);
//    ui->plugin_table->currentItem()->setCheckState(Qt::Checked);

    switch (row) {
    case 1:
        ui->load1->setVisible(true);
        break;
    case 2:
        ui->load2->setVisible(true);
        break;
    case 3:
        ui->load3->setVisible(true);
        break;
    case 4:
        ui->load4->setVisible(true);
        break;
    case 5:
        ui->load5->setVisible(true);
        break;
    case 6:
        ui->load6->setVisible(true);
        break;
    case 7:
        ui->load7->setVisible(true);
        break;
    }

    updatePluginList();
}

void plugins_dialog::updatePluginList(){
    QFile plugin_file("C:\\Autoss\\Draft\\Plugins\\plugin_list.txt");
    if(plugin_file.open(QIODevice::ReadOnly)){
        QTextStream str(&plugin_file);
        QString content;
        if(str.readAll() > 4){
            plugin_file.seek(0);
            plugin_list.clear();
            while(!str.atEnd()){
                content = str.readLine();
//                content = str.readLine().split(".").at(0);

//                // Split string by Uppercase letters
//                QRegularExpression regexp("[A-Z][^A-Z]*");
//                QRegularExpressionMatchIterator match = regexp.globalMatch(content);
//                QStringList content_split;
//                while(match.hasNext()){
//                    content_split.append(match.next().capturedTexts());
//                }
//                QString temp;
//                for(int i = 0; i < content_split.size(); i++){
//                    if(i == content_split.size() - 1){
//                        temp.append(content_split.at(i));
//                    } else temp.append(content_split.at(i) + " ");
//                }
                plugin_list.append(content);
            }
            plugin_file.flush();
            plugin_file.close();
        }
    }
}

void plugins_dialog::on_plugin_table_cellDoubleClicked(int row, int column){
    if(column == 1 && row <= plugin_list.size()){
        QFile load_file("C:\\Autoss\\Draft\\Plugins\\load_list.txt");
        if(load_file.open(QIODevice::ReadOnly)){
            QTextStream str(&load_file);
            QString array = str.readLine();
            for(int i = 0; i < 7; i++){
                load_list.append(array.split(",").at(i).toInt());
            }
            load_file.flush();
            load_file.close();
        }

        row = row - 1;
        qDebug() << plugin_list.at(row);
        int row_tmp = row + 1;
        switch (row_tmp) {
        case 1:
            if(load_list.at(row) == 1) ui->load1->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
            else ui->load1->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
            break;
        case 2:
            if(load_list.at(row) == 1) ui->load2->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
            else ui->load2->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
            break;
        case 3:
            if(load_list.at(row) == 1) ui->load3->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
            else ui->load3->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
            break;
        case 4:
            if(load_list.at(row) == 1) ui->load4->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
            else ui->load4->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
            break;
        case 5:
            if(load_list.at(row) == 1) ui->load5->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
            else ui->load5->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
            break;
        case 6:
            if(load_list.at(row) == 1) ui->load6->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
            else ui->load6->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
            break;
        case 7:
            if(load_list.at(row) == 1) ui->load7->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
            else ui->load7->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
            break;
        }
        QByteArray data;
        if(load_list.at(row) == 1) data.append((char)0);
        else data.append(1);
        load_list.replace(row, 1, data);
        if(load_file.open(QIODevice::WriteOnly)){
            QTextStream str(&load_file);
            for(int i = 0; i < 7; i++){
                if(i == 6) str << QString::number(load_list.at(i));
                else str << QString::number(load_list.at(i)) + ",";
            }
            load_file.flush();
            load_file.close();
        }
        final_row = row;
        if (load_list.at(row) == 1) emit loadPlugin(true);
        else emit loadPlugin(false);
    }
}

QStringList plugins_dialog::getPluginList(int *index){
    *index = final_row;
    return plugin_list;
}

void plugins_dialog::on_remove_clicked(){
    int row = ui->plugin_table->currentRow();

    for(int i = 0; i < plugin_list.size(); i++){
        final_row = i;
        if(load_list.at(i) == 1) emit loadPlugin(false);
    }

    QStringList new_list;
    QString txt;
    for(int i = 0; i < plugin_list.size(); i++){
        if(i == row - 1) continue;
        new_list.append(plugin_list.at(i));
        txt.append(plugin_list.at(i) + "\r\n");
    }
    plugin_list.empty();
    plugin_list = new_list;

    for(int j = 1; j < 8; j++){
        QTableWidgetItem *item = new QTableWidgetItem;
        if(j <= new_list.size()) item->setText(new_list.at(j - 1));
        else item->setText("");
        ui->plugin_table->setItem(j, 0, item);
    }

    QByteArray new_load_list;
    for(int i = 0; i < row - 1; i++){
        new_load_list.append(load_list.at(i));
    }
    for(int i = row - 1; i < 6; i++){
        int next_row = i + 1;
        if(load_list.at(next_row) == 1){
            new_load_list.append(1);
        } else new_load_list.append((char) 0);
    }
    new_load_list.append((char) 0);
    load_list = new_load_list;

    for(int i = row; i < 8; i++){
        if(i <= new_list.size()){
            switch (i) {
            case 1:
                if(new_load_list.at(0) == 1) ui->load1->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
                else ui->load1->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
                break;
            case 2:
                if(new_load_list.at(1) == 1) ui->load2->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
                else ui->load2->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
                break;
            case 3:
                if(new_load_list.at(2) == 1) ui->load3->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
                else ui->load3->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
                break;
            case 4:
                if(new_load_list.at(3) == 1) ui->load4->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
                else ui->load4->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
                break;
            case 5:
                if(new_load_list.at(4) == 1) ui->load5->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
                else ui->load5->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
                break;
            case 6:
                if(new_load_list.at(5) == 1) ui->load6->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
                else ui->load6->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
                break;
            case 7:
                if(new_load_list.at(6) == 1) ui->load7->setStyleSheet("QLineEdit {background-color: rgb(170, 255, 127);border: 2px solid gray;border-radius: 8px;}");
                else ui->load7->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
                break;
            }
        } else{
            switch (i) {
            case 1:
                ui->load1->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
                ui->load1->setVisible(false);
                break;
            case 2:
                ui->load2->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
                ui->load2->setVisible(false);
                break;
            case 3:
                ui->load3->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
                ui->load3->setVisible(false);
                break;
            case 4:
                ui->load4->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
                ui->load4->setVisible(false);
                break;
            case 5:
                ui->load5->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
                ui->load5->setVisible(false);
                break;
            case 6:
                ui->load6->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
                ui->load6->setVisible(false);
                break;
            case 7:
                ui->load7->setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 8px;}");
                ui->load7->setVisible(false);
                break;
            }
        }
    }

    QFile plugin_file("C:\\Autoss\\Draft\\Plugins\\plugin_list.txt");
    if(plugin_file.open(QIODevice::WriteOnly)){
        QTextStream str(&plugin_file);
        str << txt;
        plugin_file.flush();
        plugin_file.close();

        QFile load_file("C:\\Autoss\\Draft\\Plugins\\load_list.txt");
        if(load_file.open(QIODevice::WriteOnly)){
            str.setDevice(&load_file);
            str << "";
            load_file.flush();
            load_file.close();
            if(load_file.open(QIODevice::ReadWrite)){
                for(int i = 0; i < 7; i++){
                    if(i == 6) str << QString::number(new_load_list.at(i));
                    else{
                        str << QString::number(new_load_list.at(i));
                        str << ",";
                    }
                }
                load_file.flush();
                load_file.close();

                for(int i = 0; i < new_list.size(); i++){
                    final_row = i;
                    if(load_list.at(i) == 1) emit loadPlugin(true);
                }
            }
        }
    }
}


void plugins_dialog::on_open_clicked(){
    showInFolder(QCoreApplication::applicationDirPath() + "/plugins");
}

void plugins_dialog::showInFolder(QString dir){
    #ifdef _WIN32    //Code for Windows
        QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(dir)});
    #elif defined(__APPLE__)    //Code for Mac
        QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to reveal POSIX file \"" + dir + "\""});
        QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to activate"});
    #endif
}
