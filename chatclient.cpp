#include "chatclient.h"
#include "common.h"

using namespace std::placeholders;

ChatClient::ChatClient(EventLoop *loop, const InetAddress &serverAddr, QObject *parent)
    : loop_(loop),
      client_(loop, serverAddr, "ChatClient"),
      dispatcher_(std::bind(&ChatClient::onUnknownMessageType, this, _1, _2, _3)),
      codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1, _2, _3)),
      username_("guest")
{
    dispatcher_.registerMessageCallback<chat::LoginResponse>(
        std::bind(&ChatClient::onLoginResponse, this, _1, _2, _3));
    dispatcher_.registerMessageCallback<chat::LogoutResponse>(
        std::bind(&ChatClient::onLogoutResponse, this, _1, _2, _3));
    dispatcher_.registerMessageCallback<chat::RegisterResponse>(
        std::bind(&ChatClient::onRegisterResponse, this, _1, _2, _3));
    dispatcher_.registerMessageCallback<chat::TextMessage>(
        std::bind(&ChatClient::onTextMessage, this, _1, _2, _3));
    dispatcher_.registerMessageCallback<chat::TextMessageResponse>(
      std::bind(&ChatClient::onTextMessageResponse, this, _1, _2, _3));
    dispatcher_.registerMessageCallback<chat::SearchResponse>(
      std::bind(&ChatClient::onSearchResponse, this, _1, _2, _3));
    dispatcher_.registerMessageCallback<chat::GroupResponse>(
      std::bind(&ChatClient::onGroupResponse, this, _1, _2, _3));
    client_.setConnectionCallback(
        std::bind(&ChatClient::onConnection, this, _1));
    client_.setMessageCallback(
        std::bind(&ProtobufCodec::onMessage, &codec_, _1, _2, _3));
    client_.enableRetry();
    std::vector<std::string> words = WordsWeHate;
    for (std::string word : words) {
        ac_.insert(word);
    }
    ac_.build();
}

void ChatClient::connect()
{
    client_.connect();
}

void ChatClient::disconnect()
{
    client_.disconnect();
}

void ChatClient::send(const std::string &line)
{
    MutexLockGuard lock(connection_mutex_);
    if (connection_)
    {
        processCommand(line);
    }
}

void ChatClient::onConnection(const TcpConnectionPtr &conn)
{
  ChatLogInfo() << QString::fromStdString(conn->localAddress().toIpPort()) << " -> "
            << QString::fromStdString(conn->peerAddress().toIpPort()) << " is "
            << (conn->connected() ? "UP" : "DOWN");

  MutexLockGuard lock(connection_mutex_);
  if (conn->connected())
  {
    connection_ = conn;
  }
  else
  {
    connection_.reset();
  }
}

void ChatClient::onTextMessageResponse(const TcpConnectionPtr &conn,
                            const TextMessageResponsePtr &message,
                            Timestamp)
{
    ChatLogInfo() << "onTextMessageResponse: " << QString::fromStdString(message->GetTypeName());

    if (message->success())
    {
        ChatLogInfo() << "Message sent successfully";
    }
    else
    {
        ChatLogInfo() << "Failed to send message: " << QString::fromStdString(message->error_message());
    }
}

void ChatClient::onLoginResponse(const TcpConnectionPtr &conn,
                       const LoginResponsePtr &message,
                       Timestamp)
{
    ChatLogInfo() << "onLoginResponse: " << QString::fromStdString(message->GetTypeName());

    if (message->success())
    {
        ChatLogInfo() << "Login succeeded";
        {
            MutexLockGuard lock(username_mutex_);
      username_ = message->username();
        }
      emit loginResponseReceived(true, message->username());
    }
    else
    {
        ChatLogInfo() << "Login failed: " << QString::fromStdString(message->error_message());
      emit loginResponseReceived(false, message->error_message());
    }
}

void ChatClient::onLogoutResponse(const TcpConnectionPtr &conn,
                        const LogoutResponsePtr &message,
                        Timestamp)
{
    ChatLogInfo() << "onLogoutResponse: " << QString::fromStdString(message->GetTypeName());

    if (message->success())
    {
        ChatLogInfo() << "Logout succeeded";
        // MutexLockGuard lock(username_mutex_);
        // username_ = "guest";
    }
    else
    {
        ChatLogInfo() << "Logout failed: " << QString::fromStdString(message->error_message());
    }
}

void ChatClient::onRegisterResponse(const TcpConnectionPtr &conn,
                          const RegisterResponsePtr &message,
                          Timestamp)
{
    ChatLogInfo() << "onRegisterResponse: " << QString::fromStdString(message->GetTypeName());

    if (message->success())
    {
        ChatLogInfo() << "Register succeeded";
        emit registerResponseReceived(true);
    }
    else
    {
        ChatLogInfo() << "Register failed: " << QString::fromStdString(message->error_message());
        emit registerResponseReceived(false);
    }
}

void ChatClient::onSearchResponse(const TcpConnectionPtr &conn,
                        const SearchResponsePtr &message,
                        Timestamp)
{
    ChatLogInfo() << "onSearchResponse: " << QString::fromStdString(message->GetTypeName());
    std::vector<std::string> usernames;
    for (const auto &username : message->usernames())
    {
        usernames.push_back(username);
    }
    emit searchResponseReceived(usernames);
}

void ChatClient::onGroupResponse(const TcpConnectionPtr &conn,
                      const GroupResponsePtr &message,
                      Timestamp)
{
    ChatLogInfo() << "onGroupResponse: " << QString::fromStdString(message->GetTypeName());

    if (message->success())
    {
        std::string operation;
        switch (message->operation()) {
            case chat::GroupOperation::CREATE:
                operation = "Create group";
                break;
            case chat::GroupOperation::JOIN:
                operation = "Join group";
                break;
            case chat::GroupOperation::LEAVE:
                operation = "Leave group";
                break;
            case chat::GroupOperation::QUERY:
                operation = "Query groups";
                break;
            default:
                operation = "Unknown command";
                break;
        }
        ChatLogInfo() << QString::fromStdString(operation) << " succeeded";
    }
    else
    {
        ChatLogInfo() << "Group operation failed: " << QString::fromStdString(message->error_message());
    }
      std::vector<std::string> groups;
    if (message->joined_groups_size() > 0)
    {
        for (const auto &group : message->joined_groups())
        {
                groups.push_back(group);
        }
    }
        emit groupResponseReceived(message->success(), message->operation(), message->error_message(), groups);
}

void ChatClient::onTextMessage(const TcpConnectionPtr &conn,
                     const TextMessagePtr &message,
                     Timestamp)
{
    ChatLogInfo() << "onTextMessage: " << QString::fromStdString(message->GetTypeName());
    bool is_group = message->target_type() == chat::TargetType::GROUP;
    std::string group;
    if (message->target_type() == chat::TargetType::USER){
        group = "private";
    } 
    else if (message->target_type() == chat::TargetType::GROUP)
    {
        group = message->target();
    }
    emit textMessageReceived(is_group, group, message->sender(), ac_.filter(message->content()));
}

void ChatClient::onUnknownMessageType(const TcpConnectionPtr &conn,
                            const MessagePtr &message,
                            Timestamp)
{
    ChatLogInfo() << "onUnknownMessageType: " << QString::fromStdString(message->GetTypeName());
}