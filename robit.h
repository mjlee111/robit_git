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

#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <sys/stat.h>

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

    void ui_init();
    void ssh_connection();
    void read_files();

    QString default_path;

    QVector<QString> filenames;
    int file_count = 0;

private slots:
    void on_login_btn_clicked();

    void on_refresh_clicked();

private:
    Ui::robit *ui;

    QString IP, USER, PASSWORD, PORT = "22";
};

#endif // ROBIT_H
