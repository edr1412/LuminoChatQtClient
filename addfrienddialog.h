#ifndef ADDFRIENDDIALOG_H
#define ADDFRIENDDIALOG_H

#include "common.h"
#include "widget.h"
#include <QDialog>

namespace Ui {
class AddFriendDialog;
}

class AddFriendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFriendDialog(QWidget *parent = nullptr);
    ~AddFriendDialog();
public:
    std::string getFriendName(){
        return selected_friend_name_;
    }
public slots:
    void onSearchResponseReceived(const QStringList &usernames);
private slots:
    void on_pushButton_find_clicked();

    void on_pushButton_addFriend_clicked();
signals:
    void sendSearchMessageRequest(const QString &pattern);
private:
    Ui::AddFriendDialog *ui;
    std::string selected_friend_name_;
};

#endif // ADDFRIENDDIALOG_H
