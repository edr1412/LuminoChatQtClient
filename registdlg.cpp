#include "registdlg.h"
#include "ui_registdlg.h"

#include <QGraphicsDropShadowEffect>
#include <QMessageBox>

RegistDlg::RegistDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegistDlg)
{
    ui->setupUi(this);
    Init();
}

RegistDlg::~RegistDlg()
{
    delete ui;
}

void RegistDlg::mousePressEvent(QMouseEvent *event)
{
    mouseWindowTopLeft = event->pos();
}

void RegistDlg::mouseMoveEvent(QMouseEvent *event)
{

    //窗口移动
    if (event->buttons() & Qt::LeftButton)
    {
        mouseDeskTopLeft = event->globalPos();
        windowDeskTopLeft = mouseDeskTopLeft - mouseWindowTopLeft;  //矢量计算
        this->move(windowDeskTopLeft);     //移动到目的地
    }
}


void RegistDlg::Init()
{
    this->setWindowTitle("WeChat 注册");
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint); // 最小化按钮
    setWindowFlags(windowFlags() | Qt::WindowContextHelpButtonHint); // 帮助按钮

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
}

void RegistDlg::on_pushBtn_regist_clicked()
{
    std::string username = ui->lineEdit_username->text().toStdString();
    std::string password = ui->lineEdit_password->text().toStdString();
    //strncpy(m_userInfo.m_password,ui->lineEdit_password->text().toStdString().c_str(),ui->lineEdit_password->text().size());
    if(username.empty() || password.empty())
    {
        QMessageBox::warning(this,"警告","账号或密码不能为空！");
        return;
    }
    emit sendRegistMessageRequest(username,password);
    return accept();    //Closes the dialog and emits the accepted() signal.
}


void RegistDlg::on_pushBtn_hide_clicked()
{
    QWidget* pWindow = this->window();
    if(pWindow->isTopLevel())
        pWindow->showMinimized();
}

void RegistDlg::on_pushBtn_close_clicked()
{
   this->close();
}
