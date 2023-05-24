#include "addfrienddialog.h"
#include "ui_addfrienddialog.h"

AddFriendDialog::AddFriendDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddFriendDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("WeChat 查找");
    ui->widget_friendInfo->hide();
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint); // 最小化按钮
    setWindowFlags(windowFlags() | Qt::WindowContextHelpButtonHint); // 帮助按钮
}

AddFriendDialog::~AddFriendDialog()
{
    delete ui;
}

void AddFriendDialog::on_pushButton_find_clicked()
{
    ChatLogInfo()<<"on_pushButton_find_clicked in..";
    emit sendSearchMessageRequest(ui->lineEdit_account->text().toStdString());
}

void AddFriendDialog::onSearchResponseReceived(const std::vector<std::string> &usernames)
{
    ChatLogInfo()<<"onSearchResponseReceived";
    ui->listWidget->clear();
    for (const auto &username : usernames) {
        ui->listWidget->addItem(QString::fromStdString(username));
    }
}

void AddFriendDialog::on_pushButton_addFriend_clicked()
{
    ChatLogInfo()<<"on_pushButton_addFriend_clicked";
    if (ui->listWidget->selectedItems().empty()) {
        return;
    }
    selected_friend_name_ = ui->listWidget->selectedItems().front()->text().toStdString();
    return accept();    //Closes the dialog and emits the accepted() signal.
}
