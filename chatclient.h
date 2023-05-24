#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QObject>
#include "codec.h"
#include "dispatcher.h"
#include "chat.pb.h"
#include "ACAutomaton.h"

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/TcpClient.h>

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

using LoginRequestPtr = std::shared_ptr<chat::LoginRequest>;
using RegisterRequestPtr = std::shared_ptr<chat::RegisterRequest>;
using GroupRequestPtr = std::shared_ptr<chat::GroupRequest>;
using TextMessagePtr = std::shared_ptr<chat::TextMessage>;
using TextMessageResponsePtr = std::shared_ptr<chat::TextMessageResponse>;
using LoginResponsePtr = std::shared_ptr<chat::LoginResponse>;
using LogoutResponsePtr = std::shared_ptr<chat::LogoutResponse>;
using RegisterResponsePtr = std::shared_ptr<chat::RegisterResponse>;
using SearchResponsePtr = std::shared_ptr<chat::SearchResponse>;
using GroupResponsePtr = std::shared_ptr<chat::GroupResponse>;

class ChatClient : public QObject
{
    Q_OBJECT
public:
    ChatClient(EventLoop *loop, const InetAddress &serverAddr, QObject *parent = nullptr);
    void connect();
    void disconnect();
    void send(const std::string &line);

signals:
    void loginResponseReceived(bool success, const std::string &username);
    void logoutResponseReceived(bool success);
    void registerResponseReceived(bool success);
    void textMessageReceived(bool is_group, const std::string &group, const std::string &sender, const std::string &content);
    void textMessageResponseReceived(bool success);
    void searchResponseReceived(const std::vector<std::string> &usernames);
    void groupResponseReceived(bool success, const std::string& operation, const std::vector<std::string> &groups);

private:
    void processCommand(const std::string &line);
    void onConnection(const TcpConnectionPtr &conn);
    void onTextMessageResponse(const TcpConnectionPtr &conn, const TextMessageResponsePtr &message, Timestamp);
    void onLoginResponse(const TcpConnectionPtr &conn, const LoginResponsePtr &message, Timestamp);
    void onLogoutResponse(const TcpConnectionPtr &conn, const LogoutResponsePtr &message, Timestamp);
    void onRegisterResponse(const TcpConnectionPtr &conn, const RegisterResponsePtr &message, Timestamp);
    void onSearchResponse(const TcpConnectionPtr &conn, const SearchResponsePtr &message, Timestamp);
    void onGroupResponse(const TcpConnectionPtr &conn, const GroupResponsePtr &message, Timestamp);
    void onTextMessage(const TcpConnectionPtr &conn, const TextMessagePtr &message, Timestamp);
    void onUnknownMessageType(const TcpConnectionPtr &conn, const MessagePtr &message, Timestamp);

    EventLoop *loop_;
    TcpClient client_;
    ProtobufDispatcher dispatcher_;
    ProtobufCodec codec_;
    MutexLock connection_mutex_;
    TcpConnectionPtr connection_;
    MutexLock username_mutex_;
    std::string username_;
    ACAutomaton ac_;
};

#endif // CHATCLIENT_H
