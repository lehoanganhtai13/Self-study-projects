#include "about_dialog.h"
#include "ui_about_dialog.h"

about_dialog::about_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::about_dialog)
{
    ui->setupUi(this);
    this->setWindowTitle("About");
    ui->link_website->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    ui->link_website->setOpenExternalLinks(true);
    ui->link_website->setTextFormat(Qt::RichText);
    ui->link_website->setText("<a href=\"https://hcmut.edu.vn/\"> hcmut</a>");
    ui->link_linkedln->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    ui->link_linkedln->setOpenExternalLinks(true);
    ui->link_linkedln->setTextFormat(Qt::RichText);
    ui->link_linkedln->setText("<a href=\"https://www.linkedin.com/in/lehoanganhtai/\"> lehoanganhtai</a>");
}

about_dialog::~about_dialog()
{
    delete ui;
}

//void about_dialog::on_link_website_linkActivated(const QString &link){
//    QDesktopServices::openUrl(QUrl("https://autoss.vn/"));
//}

