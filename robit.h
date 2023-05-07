#ifndef ROBIT_H
#define ROBIT_H

#include <iostream>
#include <stdlib.h>
#include <QMainWindow>
#include <QPixmap>
#include <QFont>
#include <QMessageBox>
#include <QStringList>
#include <QVector>
#include <QTreeWidgetItem>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QFormLayout>

#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <sys/stat.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class robit;
}
QT_END_NAMESPACE

class robit : public QMainWindow
{
    Q_OBJECT

public:
    explicit robit(QWidget *parent = 0);
    ~robit();

    bool if_connected = false;

    void ui_init();
    void ssh_connection();
    bool request_login();
    void read_files();
    void read_files_recursively(QTreeWidgetItem* rootItem, const QString& directory_path, sftp_session my_sftp_session);
    bool deleteRemoteFolder(sftp_session sftp, const char *dir_path);

    //void cloneRemoteFolder(sftp_session sftp, const char* remote_path, const char* local_path);

    QString default_path;

    QVector<QString> filenames;
    int file_count = 0;

private slots:
    void on_login_btn_clicked();

    void on_refresh_clicked();

   // void on_clone_btn_clicked();

    void on_delete_btn_clicked();

    void on_file_tree_itemDoubleClicked(QTreeWidgetItem *item);

private:
    Ui::robit *ui;

    QString IP, USER, PASSWORD, PORT = "22";
};

#endif // ROBIT_H
