#include "robit.h"
#include "ui_robit.h"

using namespace std;

robit::robit(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::robit)
{
    ui->setupUi(this);
    ui_init();
}

robit::~robit()
{
    delete ui;
}

void robit::ui_init(){
    ui->input_password->setEchoMode(QLineEdit::Password);
    ui->input_PORT->setPlaceholderText("Default : 22");

    QPixmap logo(":/image/resources/logo.jpeg");
    ui->logo->setPixmap(logo.scaled(100, 100, Qt::KeepAspectRatio));

    QPixmap logo2(":/image/resources/rogit.png");
    ui->head->setPixmap(logo2.scaled(400, 60, Qt::KeepAspectRatioByExpanding));

    ui->connection->setStyleSheet("QLabel { background-color : red; }");

    QPixmap refresh_img(":/image/resources/refresh.png");
    ui->refresh->setIcon(refresh_img.scaled(1000, 1000, Qt::KeepAspectRatioByExpanding));

    ui->name->setText("RO:BIT 17th LMJ");
}

void robit::ssh_connection(){
    ssh_session my_ssh_session = ssh_new();
    if (my_ssh_session == NULL) {
        // handle error
    }

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, IP.toUtf8());
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, USER.toUtf8());

    int rc = ssh_connect(my_ssh_session);
    if (rc != SSH_OK) {
        QMessageBox msgbox;
        msgbox.setWindowTitle("WARNING");
        msgbox.setText("CONNECTION FAILED \n CHECK YOUR INPUT");
        msgbox.setStandardButtons(QMessageBox::NoButton);
        msgbox.addButton(QMessageBox::Ok);
        msgbox.exec();
        return;
    }

    rc = ssh_userauth_password(my_ssh_session, NULL, PASSWORD.toUtf8());
    if (rc == SSH_AUTH_SUCCESS) {
        ui->connection->setStyleSheet("QLabel { background-color : green; }");
        ui->login_btn->setEnabled(false);
        ui->login_btn->setStyleSheet("background-color: green");
    }
    else if(rc != SSH_AUTH_SUCCESS){
        QMessageBox msgbox;
        msgbox.setWindowTitle("WARNING");
        msgbox.setText("WRONG PASSWORD \n CHECK YOUR PASSWORD");
        msgbox.setStandardButtons(QMessageBox::NoButton);
        msgbox.addButton(QMessageBox::Ok);
        msgbox.exec();
        return;
    }
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    ssh_finalize();
}

void robit::read_files(){
    ui->loading->setText("LOADING ...");
    ui->file_tree->clear();
    QTreeWidgetItem* rootItem = new QTreeWidgetItem(ui->file_tree);
    rootItem->setText(0, "/");
    ui->file_tree->addTopLevelItem(rootItem);

    ssh_session my_ssh_session = ssh_new();

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, IP.toUtf8());
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, USER.toUtf8());
    ssh_connect(my_ssh_session);
    ssh_userauth_password(my_ssh_session, NULL, PASSWORD.toUtf8());

    sftp_session my_sftp_session = sftp_new(my_ssh_session);
    sftp_init(my_sftp_session);

    const char* directory_path = "/home/robit_git/git";

    sftp_dir my_sftp_dir = sftp_opendir(my_sftp_session, directory_path);
    if(!my_sftp_dir){
        QMessageBox msgbox;
        msgbox.setWindowTitle("WARNING");
        msgbox.setText("Failed to read file");
        msgbox.setStandardButtons(QMessageBox::NoButton);
        msgbox.addButton(QMessageBox::Ok);
        msgbox.exec();
        return;
    }

    sftp_attributes my_sftp_attributes;
    while ((my_sftp_attributes = sftp_readdir(my_sftp_session, my_sftp_dir)) != NULL){
        QString fileName = QString::fromUtf8(my_sftp_attributes->name);
        if (fileName == "." || fileName == "..") {
            continue; // skip the current and parent directories
        }
        QTreeWidgetItem* item;
        if (my_sftp_attributes->permissions & S_IFDIR) {
            item = new QTreeWidgetItem(rootItem);
            item->setText(0, fileName);
            rootItem->addChild(item);

            sftp_dir subSftpDir = sftp_opendir(my_sftp_session, fileName.toUtf8().constData());
            if (subSftpDir) {
                sftp_attributes subSftpAttributes;
                while ((subSftpAttributes = sftp_readdir(my_sftp_session, subSftpDir)) != NULL) {
                    QString subFileName = QString::fromUtf8(subSftpAttributes->name);
                    if (subFileName == "." || subFileName == "..") {
                        continue;
                    }

                    QTreeWidgetItem* subItem = new QTreeWidgetItem(item);
                    subItem->setText(0, subFileName);
                    item->addChild(subItem);
                }
                sftp_closedir(subSftpDir);
            }
        }
        else {
            item = new QTreeWidgetItem(rootItem);
            item->setText(0, fileName);
            rootItem->addChild(item);
        }
    }
    rootItem->setExpanded(true);
    ui->file_tree->expandAll();

    sftp_free(my_sftp_session);
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    ssh_finalize();
    ui->loading->setText("LOADED");

}

void robit::on_login_btn_clicked(){
    IP = ui->input_IP->text();
    USER = ui->input_USER->text();
    PASSWORD = ui->input_password->text();
    if(ui->input_PORT->hasAcceptableInput()){
        PORT = ui->input_PORT->text();
    }
    ssh_connection();
    read_files();
}

void robit::on_refresh_clicked()
{
    read_files();
}
