#ifndef WIDGET_H
#define WIDGET_H

#include <QListWidget>
#include <QWidget>
#include "common.h"
#include "logindlg.h"
#include "addfrienddialog.h"
#include "chatclient.h"
#include <map>
#include <list>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

typedef enum{
    TYPE_GROUP_CHAT,TYPE_PRIVATE_CHAT
}EChatType;

struct chatWidgetInfo{
    std::string m_account;  //聊天窗口对应的聊天号
    EChatType m_type;     //聊天窗口对应的类型 群聊/私聊
};

typedef std::map<std::string,QListWidget*> mapChatWidget;
typedef std::list<chatWidgetInfo*> listChatWidgetInfo;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
protected:
    // Event handlers
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event); //双击
public:
    QPoint mouseWindowTopLeft; //鼠标相对窗口左上角的坐标         在mousePressEvent 得到
    QPoint mouseDeskTopLeft;   //鼠标相对于桌面左上角坐标         在mouseMoveEvent实时获取
    QPoint windowDeskTopLeft;  //窗口左上角相对于桌面左上角坐标    在mouseMoveEvent实时计算(矢量)获得

public slots:
    void onLoginResponseReceived(bool success, const std::string &username);
    void onRegistResponseReceived(bool success);
    void onTextMessageReceived(bool is_group, const std::string &group, const std::string &sender, const std::string &content);
    void onGroupResponseReceived(bool success, const std::string& operation, const std::string& error_message, const std::vector<std::string> &groups);
    void onSendSearchMessageRequest(const std::string &pattern);
    void onSendLoginMessageRequest(const std::string &username, const std::string &password);
    void onSendRegistMessageRequest(const std::string &username, const std::string &password);
private slots:
    void on_pushBtn_send_clicked();
    void on_pushButton_addFriend_clicked();

    void on_listWidget_info_itemClicked(QListWidgetItem *item);

    void on_pushBtn_close_clicked();

    void on_pushBtn_hide_clicked();

    void on_pushBtn_max_clicked();

private:
    void Init();
    void InitUI();
    void Add_Group_Item(const std::string& group);
    void Add_Friend_Item(const std::string& username);

public:
    int getLoginStatus(){
        return hasLogin_;
    }
#if 0
    GroupUserInfo* findUserInfo(int account);
#endif

private:
    Ui::Widget *ui;
    EventLoopThread loopThread_;
    InetAddress serverAddr_;
    ChatClient client_;
    bool        hasLogin_;
    bool        m_isfull;
    std::string username_;
    QRect       m_rect;

    mapChatWidget           m_chatWigetMapPrivate;     //用户名, QListWidget*
    mapChatWidget           m_chatWigetMapGroup;     //用户名, QListWidget*

    listChatWidgetInfo      m_chatWidgetInfoList;
};
#endif // WIDGET_H
