#include "widget.h"
#include "ui_widget.h"
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget),
    loopThread_(),
    serverAddr_(SERVER_ADDR, SERVER_PORT),
    client_(loopThread_.startLoop(), serverAddr_),
    hasLogin_(false),
    username_("guest")
{
    client_.connect();
    ui->setupUi(this);
    InitUI();
    Init();
}

Widget::~Widget()
{
    delete ui;
}


void Widget::InitUI()
{
    this->setWindowTitle("WeChat");
//    int width = this->width()-10;
//    int height = this->height()-10;
//    ui->centerWidget->setGeometry(5,5,width,height);
    ui->centerWidget->setStyleSheet("QWidget#centerWidget{ border-radius:4px; background:rgba(255,255,255,1); }");

    this->setWindowFlags(Qt::FramelessWindowHint);          //去掉标题栏无边框
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    //实例阴影shadow
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    //设置阴影距离
    shadow->setOffset(0, 0);
    //设置阴影颜色
    shadow->setColor(QColor(39,40,43,100));
    //设置阴影圆角
    shadow->setBlurRadius(10);
    //给嵌套QWidget设置阴影
    ui->centerWidget->setGraphicsEffect(shadow);


    m_isfull = false;
}

void Widget::Init()
{
    connect(&client_, &ChatClient::loginResponseReceived, this, &Widget::onLoginResponseReceived);
    connect(&client_, &ChatClient::registerResponseReceived, this, &Widget::onRegistResponseReceived);
    connect(&client_, &ChatClient::textMessageReceived, this, &Widget::onTextMessageReceived);
    connect(&client_, &ChatClient::groupResponseReceived, this, &Widget::onGroupResponseReceived);
    
    LoginDlg* loginDlg = new LoginDlg;
    connect(loginDlg, &LoginDlg::sendLoginMessageRequest, this, &Widget::onSendLoginMessageRequest);
    connect(loginDlg, &LoginDlg::sendRegistMessageRequestAsProxy, this, &Widget::onSendRegistMessageRequest);
    loginDlg->setAttribute(Qt::WA_DeleteOnClose);
    loginDlg->show();
    int status = loginDlg->exec();
    if (status == QDialog::Accepted)
    {
        ChatLogInfo()<<"wait for server..";
    }
    else if (status == QDialog::Rejected)
    {
        ChatLogInfo()<<"close..";
        this->hasLogin_ = false;
    }
}

void Widget::onLoginResponseReceived(bool success, const QString &username)
{
    ChatLogInfo()<<"onLoginResponseReceived";
    if (success)
    {
        this->hasLogin_ = true;
        this->username_ = username.toStdString();
        this->setWindowTitle(QString("WeChat[%1]").arg(username));
        std::string command = "group query";
        ChatLogInfo()<<"command: "<<QString::fromStdString(command);
        client_.send(command);
        this->show();
    }
    else
    {
        this->hasLogin_ = false;
        // 重新尝试登录
        LoginDlg* loginDlg = new LoginDlg;
        connect(loginDlg, &LoginDlg::sendLoginMessageRequest, this, &Widget::onSendLoginMessageRequest);
        connect(loginDlg, &LoginDlg::sendRegistMessageRequestAsProxy, this, &Widget::onSendRegistMessageRequest);
        loginDlg->setAttribute(Qt::WA_DeleteOnClose);
        loginDlg->show();
        int status = loginDlg->exec();
        if (status == QDialog::Accepted)
        {
            ChatLogInfo()<<"wait for server..";
        }
        else if (status == QDialog::Rejected)
        {
            ChatLogInfo()<<"close..";
            this->hasLogin_ = false;
            this->close();
        }
    }
}

void Widget::onRegistResponseReceived(bool success)
{
    if (success)
    {
        QMessageBox::information(this, "注册成功", "注册成功，请登录");
    }
    else
    {
        QMessageBox::information(this, "注册失败", "注册失败，请重试");
    }
}

static int stackWidgetIndex = 0;

void Widget::Add_Group_Item(const std::string& group)
{
    ui->listWidget_info->addItem(QString("[群][%1]").arg(group.c_str()));

    ui->stackedWidget_Msg->setCurrentIndex(stackWidgetIndex++);
    if(stackWidgetIndex >=ui->stackedWidget_Msg->count())
    {
        QWidget* page = new QWidget();
        page->setObjectName(QString("page%1").arg(stackWidgetIndex));
        ui->stackedWidget_Msg->addWidget(page);
        ChatLogInfo()<<"stackedWidget_Msg account:"<<ui->stackedWidget_Msg->count()<<",stackWidgetIndex:"<<stackWidgetIndex;
    }
    QListWidget* listWidget = new QListWidget;
    QGridLayout* mainLayout = new QGridLayout;
    ui->stackedWidget_Msg->currentWidget()->setLayout(mainLayout);
    listWidget->setStyleSheet("QListWidget{"\
                              "background-color: rgb(247,247,247);"\
                              "border-style: none;"\
                              "}");
    mainLayout->addWidget(listWidget);
    mainLayout->setMargin(0); //设置外边距
    mainLayout->setSpacing(0);//设置内边距

    m_chatWigetMapGroup.insert(std::make_pair(group,listWidget));
    chatWidgetInfo* chatInfo = new chatWidgetInfo;
    chatInfo->m_account = group;
    chatInfo->m_type = TYPE_GROUP_CHAT;

    m_chatWidgetInfoList.push_back(chatInfo);

    ui->listWidget_info->setCurrentRow(stackWidgetIndex - 1);
}

void Widget::Add_Friend_Item(const std::string& username)
{
    ui->listWidget_info->addItem(QString("[好友][%1]").arg(username.c_str()));

    // 记录当前 stackedWidget_Msg 的index 和 listWidget_info的row
    int index = ui->stackedWidget_Msg->currentIndex();
    int row = ui->listWidget_info->currentRow();

    ui->stackedWidget_Msg->setCurrentIndex(stackWidgetIndex++);
    if(stackWidgetIndex >=ui->stackedWidget_Msg->count())
    {
        QWidget* page = new QWidget();
        page->setObjectName(QString("page%1").arg(stackWidgetIndex));
        ui->stackedWidget_Msg->addWidget(page);
        ChatLogInfo()<<"stackedWidget_Msg account:"<<ui->stackedWidget_Msg->count()<<",stackWidgetIndex:"<<stackWidgetIndex;
    }
    QListWidget* listWidget = new QListWidget;
    QGridLayout* mainLayout = new QGridLayout;
    ui->stackedWidget_Msg->currentWidget()->setLayout(mainLayout);
    listWidget->setStyleSheet("QListWidget{"\
                              "background-color: rgb(247,247,247);"\
                              "border-style: none;"\
                              "border-top:1px solid #D6D6D6;"\
                              "}");
    mainLayout->addWidget(listWidget);
    mainLayout->setMargin(0); //设置外边距
    mainLayout->setSpacing(0);//设置内边距

    m_chatWigetMapPrivate.insert(std::make_pair(username,listWidget));
    chatWidgetInfo* chatInfo = new chatWidgetInfo;
    chatInfo->m_account = username;
    chatInfo->m_type = TYPE_PRIVATE_CHAT;

    m_chatWidgetInfoList.push_back(chatInfo);
    //ui->listWidget_info->setCurrentRow(stackWidgetIndex - 1);
    ui->stackedWidget_Msg->setCurrentIndex(index);
    ui->listWidget_info->setCurrentRow(row);
}

void Widget::onTextMessageReceived(bool is_group, const QString &group, const QString &sender, const QString &content)
{
    if(is_group)
    {
        mapChatWidget::iterator iter = m_chatWigetMapGroup.find(group.toStdString());
        if(iter==m_chatWigetMapGroup.end()){
            Add_Group_Item(group.toStdString());
            iter = m_chatWigetMapGroup.find(group.toStdString());
            if(iter==m_chatWigetMapGroup.end()){
                QMessageBox::information(this, "错误", "添加群聊失败");
            }
        }
        QListWidget* chatWidget = iter->second;
        QListWidgetItem* item = new QListWidgetItem;
        //这里转码，中文名称显示乱码
        item->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + QString("[%1]%2").arg(sender).arg(content));
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        chatWidget->addItem(item);
    }
    else
    {
        mapChatWidget::iterator iter = m_chatWigetMapPrivate.find(sender.toStdString());
        if(iter==m_chatWigetMapPrivate.end()){
            Add_Friend_Item(sender.toStdString());
            iter = m_chatWigetMapPrivate.find(sender.toStdString());
            if(iter==m_chatWigetMapPrivate.end()){
                QMessageBox::information(this, "错误", "添加私聊失败");
            }
        }
        QListWidget* chatWidget = iter->second;
        QListWidgetItem* item = new QListWidgetItem;
        //这里转码，中文名称显示乱码
        item->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + QString("[%1]%2").arg(sender).arg(content));
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        chatWidget->addItem(item);
    }
}

void Widget::onGroupResponseReceived(bool success, const QString& operation, const QString& error_message, const QStringList &groups)
{
    if(!success)
    {
        QMessageBox::information(this, "错误", operation+"失败： "+error_message);
    }
    // 遍历groups，检查m_chatWigetMapGroup中是否存在，不存在则执行Add_Group_Item
    for(const auto &group : groups)
    {
        auto iter = m_chatWigetMapGroup.find(group.toStdString());
        if(iter==m_chatWigetMapGroup.end()){
            Add_Group_Item(group.toStdString());
            iter = m_chatWigetMapGroup.find(group.toStdString());
            if(iter==m_chatWigetMapGroup.end()){
                QMessageBox::information(this, "错误", "添加群聊失败");
            }
        }
    }
}


void Widget::on_pushBtn_send_clicked()
{
    ChatLogInfo()<<"on_pushBtn_send_clicked";
    if(ui->textEdit->toPlainText().isEmpty()){
        ChatLogInfo()<<"Msg is null";
        return;
    }
    std::string msg_to_send = ui->textEdit->toPlainText().toStdString();
    int currentRow = ui->listWidget_info->currentRow();
    if(currentRow < 0){
        ChatLogInfo()<<"未选择聊天窗口";
        return;
    }
    int isfind = 0;
    int currentIndex = 0;
    QListWidget* chatWidget = NULL;
    chatWidgetInfo* chatInfo = NULL;
    for(listChatWidgetInfo::iterator iter = m_chatWidgetInfoList.begin();iter!=m_chatWidgetInfoList.end();iter++,currentIndex++){
        if(currentIndex == currentRow){
            chatInfo = *iter;
            isfind = 1;
        }
        else {

        }
    }
    if(isfind){
    }
    else {
        ChatLogInfo()<<"---------notfind----------";
        return ;
    }
    ui->textEdit->clear();
    if(chatInfo->m_type == TYPE_GROUP_CHAT){
        ChatLogInfo()<<"群聊";
        std::string command = "send group " + chatInfo->m_account + " " + msg_to_send;
        ChatLogInfo()<<"command:"<<command.c_str();
        client_.send(command);
    }
    else{
        ChatLogInfo()<<"私聊";
        std::string command = "send user " + chatInfo->m_account + " " + msg_to_send;
        ChatLogInfo()<<"command:"<<command.c_str();
        client_.send(command);
        mapChatWidget::iterator iter = m_chatWigetMapPrivate.find(chatInfo->m_account);
        if(iter!=m_chatWigetMapPrivate.end()){
            chatWidget = iter->second;
            QString msg;
            msg.sprintf("[%s]:%s", username_.c_str(), msg_to_send.c_str());
            chatWidget->addItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + msg);
        }
        else {
            ChatLogInfo()<<"not find..";
        }

    }
}


void Widget::on_pushButton_addFriend_clicked()
{
    AddFriendDialog* addFriendDlg = new AddFriendDialog();
    connect(&client_, &ChatClient::searchResponseReceived, addFriendDlg, &AddFriendDialog::onSearchResponseReceived);
    connect(addFriendDlg, &AddFriendDialog::sendSearchMessageRequest, this, &Widget::onSendSearchMessageRequest);

    addFriendDlg->show();
    if(addFriendDlg->exec() == QDialog::Accepted)
    {
        std::string username = addFriendDlg->getFriendName();
        Add_Friend_Item(username);
    }
}

void Widget::onSendSearchMessageRequest(const QString &pattern)
{
    ChatLogInfo()<<"onSendSearchMessageRequest";
    std::string command = "search " + pattern.toStdString();
    ChatLogInfo()<<"command:"<<command.c_str();
    client_.send(command);
}
void Widget::onSendLoginMessageRequest(const QString &username, const QString &password)
{
    ChatLogInfo()<<"onSendLoginMessageRequest";
    std::string command = "login " + username.toStdString() + " " + password.toStdString();
    ChatLogInfo()<<"command:"<<command.c_str();
    client_.send(command);
}

void Widget::onSendRegistMessageRequest(const QString &username, const QString &password)
{
    ChatLogInfo()<<"onSendRegistMessageRequest";
    std::string command = "register " + username.toStdString() + " " + password.toStdString();
    ChatLogInfo()<<"command:"<<command.c_str();
    client_.send(command);
}

void Widget::on_listWidget_info_itemClicked(QListWidgetItem *item)
{
    int currentRow = ui->listWidget_info->currentRow();
    ui->stackedWidget_Msg->setCurrentIndex(currentRow);
    ChatLogInfo()<<item->text()<<"current Row clicked.."<<currentRow;

}

void Widget::on_pushBtn_close_clicked()
{
    this->close();
}

void Widget::on_pushBtn_hide_clicked()
{
    QWidget* pWindow = this->window();
    if(pWindow->isTopLevel())
        pWindow->showMinimized();
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    mouseWindowTopLeft = event->pos();
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    //窗口移动
    if (event->buttons() & Qt::LeftButton)
    {
        mouseDeskTopLeft = event->globalPos();
        windowDeskTopLeft = mouseDeskTopLeft - mouseWindowTopLeft;  //矢量计算
        this->move(windowDeskTopLeft);     //移动到目的地
    }
}

void Widget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if(m_isfull){
        //取消全屏
        m_isfull = false;
        ui->centerWidget->setGeometry(m_rect);

        ui->centerWidget->move(QApplication::desktop()->screen()->rect().center() - ui->centerWidget->rect().center());
    }
    else {
        m_isfull = true;
        m_rect = ui->centerWidget->rect();
        setGeometry(QGuiApplication::primaryScreen()->availableGeometry()); // 不包含windows任务栏区域
        ui->centerWidget->setGeometry(this->rect());
    }
}


void Widget::on_pushBtn_max_clicked()
{
//    this->showFullScreen(); //全屏
    if(m_isfull){
        //取消全屏
        m_isfull = false;
        ui->centerWidget->setGeometry(640,480,m_rect.width(),m_rect.height());
        ui->centerWidget->move(QApplication::desktop()->screen()->rect().center() - ui->centerWidget->rect().center());
    }
    else {
        m_isfull = true;
        m_rect = ui->centerWidget->rect();
        setGeometry(QGuiApplication::primaryScreen()->availableGeometry()); // 不包含windows任务栏区域
        ui->centerWidget->setGeometry(this->rect());
    }
//    ui->centerWidget->showMaximized();
//    this->showMaximized();
}
