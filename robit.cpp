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
    ui->input_IP->setEchoMode(QLineEdit::Password);
    ui->input_IP->setStyleSheet("QLineEdit {color : white };");
    ui->input_password->setEchoMode(QLineEdit::Password);
    ui->input_password->setStyleSheet("QLineEdit {color : white };");
    ui->input_PORT->setPlaceholderText("Default : 22");
    ui->input_PORT->setStyleSheet("QLineEdit {color : white };");
    ui->input_USER->setStyleSheet("QLineEdit {color : white };");

    ui->login_btn->setStyleSheet("QPushButton {color : white };");

    QPixmap logo(":/image/resources/logo.jpeg");
    ui->logo->setPixmap(logo.scaled(100, 100, Qt::KeepAspectRatio));

    QPixmap logo2(":/image/resources/rogit.png");
    ui->head->setPixmap(logo2.scaled(400, 60, Qt::KeepAspectRatioByExpanding));

    ui->connection->setStyleSheet("QLabel { background-color : red; }");

    QPixmap refresh_img(":/image/resources/refresh.png");
    ui->refresh->setIcon(refresh_img.scaled(1000, 1000, Qt::KeepAspectRatioByExpanding));

    ui->name->setText("RO:BIT 17th LMJ");
    ui->file_tree->setSelectionMode(QAbstractItemView::SingleSelection);

    QPixmap delet(":/image/resources/delet.png");
    ui->delete_btn->setIcon(delet.scaled(1000, 1000, Qt::KeepAspectRatioByExpanding));

    ui->label_ip->setStyleSheet("QLabel { background-color : gray; color : white; }");
    ui->label_PORT->setStyleSheet("QLabel { background-color : gray; color : white; }");
    ui->label_PASSWORD->setStyleSheet("QLabel { background-color : gray; color : white; }");
    ui->label_USER->setStyleSheet("QLabel { background-color : gray; color : white; }");
    ui->label_6->setStyleSheet("QLabel { background-color : gray; color : white; }");
    ui->label->setStyleSheet("QLabel {color : red; }");
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
    if_connected = true;
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    ssh_finalize();
}

bool robit::request_login(){
    if(!if_connected){
        QMessageBox msgbox;
        msgbox.setWindowTitle("WARNING");
        msgbox.setText("LOGIN REQUIRED");
        msgbox.setStandardButtons(QMessageBox::NoButton);
        msgbox.addButton(QMessageBox::Ok);
        msgbox.exec();
        return false;
    }
    else return true;
}

void robit::read_files_recursively(QTreeWidgetItem* rootItem, const QString& directory_path, sftp_session my_sftp_session)
{
    sftp_dir my_sftp_dir = sftp_opendir(my_sftp_session, directory_path.toUtf8().constData());
    if (!my_sftp_dir) {
        QMessageBox msgbox;
        msgbox.setWindowTitle("WARNING");
        msgbox.setText("Failed to read file");
        msgbox.setStandardButtons(QMessageBox::NoButton);
        msgbox.addButton(QMessageBox::Ok);
        msgbox.exec();
        return;
    }

    sftp_attributes my_sftp_attributes;
    while ((my_sftp_attributes = sftp_readdir(my_sftp_session, my_sftp_dir)) != NULL) {
        QString fileName = QString::fromUtf8(my_sftp_attributes->name);
        if (fileName == "." || fileName == "..") {
            continue; // skip the current and parent directories
        }
        QTreeWidgetItem* item;
        if (my_sftp_attributes->permissions & S_IFDIR) {
            item = new QTreeWidgetItem(rootItem);
            item->setText(0, fileName);
            rootItem->addChild(item);

            QString subdirectory_path = directory_path + "/" + fileName;
            read_files_recursively(item, subdirectory_path, my_sftp_session);
        }
        else {
            item = new QTreeWidgetItem(rootItem);
            item->setText(0, fileName);
            rootItem->addChild(item);
            item->setTextColor(0, Qt::white);
        }
    }

    sftp_closedir(my_sftp_dir);
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

    read_files_recursively(rootItem, directory_path, my_sftp_session);

    rootItem->setExpanded(true);
    ui->file_tree->expandAll();

    sftp_free(my_sftp_session);
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    ssh_finalize();

    ui->loading->setText("LOADED");
}

bool robit::deleteRemoteFolder(sftp_session sftp, const char *remote_dir){
    sftp_attributes file_attrs;
    sftp_dir remote_dir_handle = sftp_opendir(sftp, remote_dir);

    if (!remote_dir_handle) {
        cerr << "Failed to open remote directory" << endl;
        return false;
    }

    while ((file_attrs = sftp_readdir(sftp, remote_dir_handle)) != nullptr) {
        if (strcmp(file_attrs->name, ".") == 0 || strcmp(file_attrs->name, "..") == 0) {
            continue;
        }

        string remote_path = string(remote_dir) + "/" + file_attrs->name;

        if (S_ISDIR(file_attrs->permissions)) {
            deleteRemoteFolder(sftp, remote_path.c_str());
        } else {
            if (sftp_unlink(sftp, remote_path.c_str()) != SSH_OK) {
                cerr << "Failed to delete remote file" << endl;
                return false;
            }
        }
    }

    sftp_closedir(remote_dir_handle);

    if (sftp_rmdir(sftp, remote_dir) != SSH_OK) {
        cerr << "Failed to delete remote directory" << endl;
        return false;
    }

    return true;
}

void robit::on_refresh_clicked()
{
    if(!request_login()) return;
    read_files();
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

void robit::on_delete_btn_clicked(){
    if(!request_login()) return;

    QTreeWidgetItem* selected_item = ui->file_tree->currentItem();
    QString sel_folder;
    if(selected_item){
        sel_folder = selected_item->text(0);
        sel_folder = "/" + sel_folder;
    }
    else {
        QMessageBox msgbox;
        msgbox.setWindowTitle("WARNING");
        msgbox.setText("SELECT A REPOSITORY TO CLONE");
        msgbox.setStandardButtons(QMessageBox::NoButton);
        msgbox.addButton(QMessageBox::Ok);
        msgbox.exec();
        return;
    }

    QString message = "Are you sure you want to delete this file?";
    int ret = QMessageBox::question(this, tr("Delete file"), message, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret != QMessageBox::Ok) {
        return;
    }

    QString directory_path = "/home/robit_git/git/";
    QString remote_path = directory_path + sel_folder;

    ssh_session my_ssh_session = ssh_new();

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, IP.toUtf8());
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, USER.toUtf8());
    ssh_connect(my_ssh_session);
    ssh_userauth_password(my_ssh_session, NULL, PASSWORD.toUtf8());

    sftp_session my_sftp_session = sftp_new(my_ssh_session);
    sftp_init(my_sftp_session);
    deleteRemoteFolder(my_sftp_session, remote_path.toUtf8());
    read_files();

    QMessageBox msgbox;
    msgbox.setWindowTitle("WORK DONE");
    msgbox.setText("DELETED A REPO");
    msgbox.setStandardButtons(QMessageBox::NoButton);
    msgbox.addButton(QMessageBox::Ok);
    msgbox.exec();
    return;
}

//void robit::highlight(QByteArray data) {
//    QTextCharFormat keyFormat;
//    keyFormat.setForeground(QColor(230, 57, 70)); // Red

//    QTextCharFormat valueFormat;
//    valueFormat.setForeground(QColor(69, 161, 250)); // Blue

//    QTextCharFormat stringFormat;
//    stringFormat.setForeground(QColor(255, 204, 102)); // Yellow
//    stringFormat.setFontWeight(QFont::Bold);

//    QTextCharFormat numberFormat;
//    numberFormat.setForeground(QColor(102, 204, 255)); // Cyan

//    QTextCharFormat boolFormat;
//    boolFormat.setForeground(QColor(255, 153, 204)); // Pink

//    QTextCharFormat nullFormat;
//    nullFormat.setForeground(QColor(179, 179, 179)); // Gray

//    QRegularExpression keyRegex("\"(\\w+)\":");
//    QRegularExpression valueRegex(":\\s*(\".*?\"|null|true|false|\\d+\\.?\\d*)");

//    QTextDocument document;
//    document.setPlainText(QString::fromUtf8(data));

//    QTextCursor cursor(&document);
//    cursor.beginEditBlock();

//    while (!cursor.atEnd()) {
//        cursor.movePosition(QTextCursor::StartOfBlock);

//        // Highlight keys
//        while (keyRegex.match(cursor.block().text()).hasMatch()) {
//            QRegularExpressionMatch match = keyRegex.match(cursor.block().text());
//            cursor.movePosition(QTextCursor::StartOfBlock);
//            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, match.capturedStart(1));
//            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, match.capturedLength(1));
//            cursor.setCharFormat(keyFormat);
//        }

//        // Highlight values
//        while (valueRegex.match(cursor.block().text()).hasMatch()) {
//            QRegularExpressionMatch match = valueRegex.match(cursor.block().text());
//            cursor.movePosition(QTextCursor::StartOfBlock);
//            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, match.capturedStart(1));
//            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, match.capturedLength(1));

//            QString value = match.captured(1);
//            if (value.startsWith("\"") && value.endsWith("\"")) {
//                // Highlight strings
//                cursor.setCharFormat(stringFormat);
//            }
//            else if (value == "null") {
//                // Highlight null
//                cursor.setCharFormat(nullFormat);
//            }
//            else if (value == "true" || value == "false") {
//                // Highlight booleans
//                cursor.setCharFormat(boolFormat);
//            }
//            else {
//                // Highlight numbers
//                cursor.setCharFormat(numberFormat);
//            }
//        }
//    }

//    cursor.endEditBlock();
//    QString highlightedData = document.toPlainText();
//    // do something with the highlighted data
//    ui->textBrowser->setText(highlightedData);
//}


void robit::on_file_tree_itemDoubleClicked(QTreeWidgetItem *item){
    QString directory_path = "/home/robit_git/git/";

    ssh_session my_ssh_session = ssh_new();

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, IP.toUtf8());
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, USER.toUtf8());
    ssh_connect(my_ssh_session);
    ssh_userauth_password(my_ssh_session, NULL, PASSWORD.toUtf8());

    sftp_session my_sftp_session = sftp_new(my_ssh_session);
    sftp_init(my_sftp_session);
    if (item->childCount() == 0) {
        QString fileName = item->text(0);
        QTreeWidgetItem* parent = item->parent();
        while (parent != nullptr) {
            fileName = parent->text(0) + "/" + fileName;
            parent = parent->parent();
        }
        QString filePath = directory_path + fileName;
        QByteArray fileData;
        sftp_file file = sftp_open(my_sftp_session, filePath.toUtf8().constData(), O_RDONLY, 0);
        if (file) {
            char buffer[4096];
            int bytesRead = 0;
            while ((bytesRead = sftp_read(file, buffer, sizeof(buffer))) > 0) {
                fileData.append(buffer, bytesRead);
            }
            sftp_close(file);
        }
        ui->label_path->setText(filePath);
        ui->label_path->setStyleSheet("QLabel {color : white; }");
        //        highlight(fileData);
        ui->textBrowser->setText(fileData);
        QTextCursor cursor = ui->textBrowser->textCursor();
        cursor.select(QTextCursor::Document); // select the entire document
        QTextCharFormat format;
        format.setForeground(QColor(Qt::white)); // set the foreground color to red
        cursor.setCharFormat(format);
    }
    sftp_free(my_sftp_session);
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    ssh_finalize();
}


//void robit::cloneRemoteFolder(sftp_session sftp, const char *remote_path, const char *local_path){
//    sftp_dir remote_dir = sftp_opendir(sftp, remote_path);
//    if (remote_dir == NULL) {
//        cerr << "Failed to open remote directory: " << ssh_get_error(sftp) << endl;
//        return;
//    }
//    int status = mkdir(local_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
//    if (status == -1 && errno != EEXIST) {
//        cerr << "Failed to create local directory: " << strerror(errno) << endl;
//        return;
//    }
//    sftp_attributes attributes;
//    while ((attributes = sftp_readdir(sftp, remote_dir)) != NULL) {
//        if (attributes->type == SSH_FILEXFER_TYPE_DIRECTORY) {
//            if (strcmp(attributes->name, ".") != 0 && strcmp(attributes->name, "..") != 0) {
//                string remote_subdir = string(remote_path) + "/" + attributes->name;
//                string local_subdir = string(local_path) + "/" + attributes->name;
//                cloneRemoteFolder(sftp, remote_subdir.c_str(), local_subdir.c_str());
//            }
//        } else {
//            string remote_file = string(remote_path) + "/" + attributes->name;
//            string local_file = string(local_path) + "/" + attributes->name;
//            sftp_file remote_handle = sftp_open(sftp, remote_file.c_str(), O_RDONLY, 0);
//            if (remote_handle == NULL) {
//                cerr << "Failed to open remote file: " << ssh_get_error(sftp) << endl;
//                continue;
//            }
//            int local_fd = open(local_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, attributes->permissions);
//            if (local_fd == -1) {
//                cerr << "Failed to open local file: " << strerror(errno) << endl;
//                sftp_close(remote_handle);
//                continue;
//            }
//            char buffer[8192];
//            size_t nbytes;
//            while ((nbytes = sftp_read(remote_handle, buffer, sizeof(buffer))) > 0) {
//                write(local_fd, buffer, nbytes);
//            }
//            ::close(local_fd);
//            sftp_close(remote_handle);
//        }
//        sftp_attributes_free(attributes);
//    }

//    // Close the remote directory
//    sftp_closedir(remote_dir);

//}

//void robit::on_clone_btn_clicked()
//{
//    if(!request_login()) return;

//    QTreeWidgetItem* selected_item = ui->file_tree->currentItem();
//    QString sel_folder;
//    if(selected_item){
//        sel_folder = selected_item->text(0);
//        sel_folder = "/" + sel_folder;
//    }
//    else {
//        QMessageBox msgbox;
//        msgbox.setWindowTitle("WARNING");
//        msgbox.setText("SELECT A REPOSITORY TO CLONE");
//        msgbox.setStandardButtons(QMessageBox::NoButton);
//        msgbox.addButton(QMessageBox::Ok);
//        msgbox.exec();
//        return;
//    }

//    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Folder"), QDir::homePath());


//    QString directory_path = "/home/robit_git/git/";
//    QString remote_path = directory_path + sel_folder;

//    ssh_session my_ssh_session = ssh_new();

//    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, IP.toUtf8());
//    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, USER.toUtf8());
//    ssh_connect(my_ssh_session);
//    ssh_userauth_password(my_ssh_session, NULL, PASSWORD.toUtf8());

//    sftp_session my_sftp_session = sftp_new(my_ssh_session);
//    sftp_init(my_sftp_session);

//    cout << remote_path.toStdString() << endl << sel_folder.toStdString() << endl;
//    cloneRemoteFolder(my_sftp_session, remote_path.toUtf8(), sel_folder.toUtf8());
//}

