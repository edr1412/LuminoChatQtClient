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
    std::string operation;
    if (message->success())
    {
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
        emit groupResponseReceived(message->success(), operation, message->error_message(), groups);
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

  void ChatClient::processCommand(const std::string &line)
  {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;
    MutexLockGuard lock(username_mutex_);

    if (cmd == "register")
    {
      std::string username, password;
      iss >> username >> password;
      chat::RegisterRequest request;
      request.set_username(username);
      request.set_password(password);
      codec_.send(connection_, request);
    }
    else if (cmd == "login")
    {
      std::string username, password;
      iss >> username >> password;
      // 如果再次登录自己，则退出
      if (username == username_) {
          return;
      }
      // 如果已经登录，要登录其他用户，则先登出
      if (username_ != "guest") {
          chat::LogoutRequest request;
          request.set_username(username_);
          codec_.send(connection_, request);
          username_ = "guest";
      }
      chat::LoginRequest request;
      request.set_username(username);
      request.set_password(password);
      codec_.send(connection_, request);
    }
    else if (cmd == "send")
    {
      std::string target_type, target, content;
      iss >> target_type >> target;
      std::getline(iss, content);
      chat::TextMessage textMessage;
      textMessage.set_sender(username_);
      textMessage.set_content(content);
      if (target_type == "user")
      {
        textMessage.set_target_type(chat::TargetType::USER);
      }
      else if (target_type == "group")
      {
        textMessage.set_target_type(chat::TargetType::GROUP);
      }
      else
      {
        LOG_ERROR << "Unknown target type: " << target_type;
        LOG_INFO << "Usage: send <user|group> <target> <content>";
        return;
      }
      textMessage.set_target(target);
      codec_.send(connection_, textMessage);
    }
    else if (cmd == "search")
    {
      std::string keyword;
      iss >> keyword;
      chat::SearchRequest request;
      request.set_keyword(keyword);
      request.set_online_only(false);
      codec_.send(connection_, request);
    }
    else if (cmd == "search-online")
    {
      std::string keyword;
      iss >> keyword;
      chat::SearchRequest request;
      request.set_keyword(keyword);
      request.set_online_only(true);
      codec_.send(connection_, request);
    }
    else if (cmd == "group")
    {
      // 如果还没有登录，则不能操作群组
      if (username_ == "guest") {
        LOG_ERROR << "Please login first";
        return;
      }
      std::string operation, group_name;
      iss >> operation >> group_name;
      chat::GroupRequest request;
      request.set_group_name(group_name);
      request.set_username(username_);

      if (operation == "create")
      {
        request.set_operation(chat::GroupOperation::CREATE);
      }
      else if (operation == "join")
      {
        request.set_operation(chat::GroupOperation::JOIN);
      }
      else if (operation == "leave")
      {
        request.set_operation(chat::GroupOperation::LEAVE);
      }
      else if (operation == "query")
      {
        request.set_operation(chat::GroupOperation::QUERY);
      }
      else
      {
        LOG_ERROR << "Unknown group operation: " << operation;
        LOG_INFO << "Usage: group <create|join|leave|query> <groupname>";
        return;
      }
      codec_.send(connection_, request);
    }
    else if (cmd == "logout")
    {
      if (username_ != "guest") {
          chat::LogoutRequest request;
          request.set_username(username_);
          codec_.send(connection_, request);
          username_ = "guest";
      } else {
          LOG_ERROR << "You are not logged in!";
      }
    }
    else
    {
      LOG_ERROR << "Unknown command: " << cmd;
    }
  }