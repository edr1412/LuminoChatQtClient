#include "logindlg.h"
#include "ui_logindlg.h"

#include <QBitmap>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QMessageBox>

LoginDlg::LoginDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDlg)
{
    ui->setupUi(this);
    Init();
}

LoginDlg::~LoginDlg()
{
    delete ui;
}

void LoginDlg::Init()
{
    this->setWindowTitle("WeChat 登录");
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint); // 最小化按钮
    setWindowFlags(windowFlags() | Qt::WindowContextHelpButtonHint); // 帮助按钮

#if 0
    QWidget *container;
    container=new QWidget(this);
    container->setGeometry(5,5,width,height);
    container->setStyleSheet("QWidget{border-radius:4px;background:rgba(255,255,255,1);}");

    this->setWindowFlags(Qt::FramelessWindowHint);      //去掉标题栏无边框
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
    container->setGraphicsEffect(shadow);
#else
    int width = this->width()-10;
    int height = this->height()-10;
    ui->centerWidget->setGeometry(5,5,width,height);
    ui->centerWidget->setStyleSheet("QWidget{border-radius:4px;background:rgba(255,255,255,1);}");  //设置圆角

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
#endif

}

void LoginDlg::mousePressEvent(QMouseEvent *event)
{
    mouseWindowTopLeft = event->pos();
}

void LoginDlg::mouseMoveEvent(QMouseEvent *event)
{
    //窗口移动
    if (event->buttons() & Qt::LeftButton)
    {
        mouseDeskTopLeft = event->globalPos();
        windowDeskTopLeft = mouseDeskTopLeft - mouseWindowTopLeft;  //矢量计算
        this->move(windowDeskTopLeft);     //移动到目的地
    }
}



void LoginDlg::on_pushbtn_regist_clicked()
{
    ChatLogInfo()<<"on_pushbtn_regist_clicked";
    RegistDlg* registDlg = new RegistDlg();
    ChatLogInfo()<<"registDlg";
    connect(registDlg, &RegistDlg::sendRegistMessageRequest, this, &LoginDlg::onSendRegistMessageRequestAsProxy);
    registDlg->setAttribute(Qt::WA_DeleteOnClose);
    ChatLogInfo()<<"registDlg->show()";
    registDlg->show();
    ChatLogInfo()<<"registDlg->exec()";
    int status = registDlg->exec();
    if (status == QDialog::Accepted)
    {
        ChatLogInfo()<<"wait for server..";
    }
    else if (status == QDialog::Rejected)
    {
        ChatLogInfo()<<"close..";
    }
}

void LoginDlg::onSendRegistMessageRequestAsProxy(const QString &username, const QString &password)
{
    ChatLogInfo()<<"onSendRegistMessageRequestAsProxy";
    emit sendRegistMessageRequestAsProxy(username, password);
}

void LoginDlg::on_pushButton_login_clicked()
{
    std::string username = ui->lineEdit_account->text().toStdString();
    std::string password = ui->lineEdit_password->text().toStdString();
    //strncpy(m_userInfo.m_password,ui->lineEdit_password->text().toStdString().c_str(),ui->lineEdit_password->text().size());
    if(username.empty() || password.empty())
    {
        QMessageBox::warning(this,"警告","账号或密码不能为空！");
        return;
    }
    emit sendLoginMessageRequest(QString::fromStdString(username), QString::fromStdString(password));
    return accept();    //Closes the dialog and emits the accepted() signal.
}

void LoginDlg::on_pushBtn_hide_clicked()
{
    QWidget* pWindow = this->window();
    if(pWindow->isTopLevel())
        pWindow->showMinimized();
}

void LoginDlg::on_pushBtn_close_clicked()
{
    this->close();
}
