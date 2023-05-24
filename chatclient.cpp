#include "chatclient.h"

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
    std::vector<std::string> words = {"Steve Jobs", "Tim Cook", "Jony Ive", "Apple Inc.", "iPhone", "iPad", "MacBook", "iMac", "Apple Watch", "iOS", "macOS", "Safari", "Apple Music", "HomePod", "iCloud", "乔布斯", "蒂姆·库克", "强尼·艾维", "苹果公司", "苹果手机", "苹果平板", "麦金塔电脑"};
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
  ChatLogInfo() << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
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
      ChatLogInfo() << "onTextMessageResponse: " << message->GetTypeName();

      if (message->success())
      {
          ChatLogInfo() << "Message sent successfully";
      }
      else
      {
          ChatLogInfo() << "Failed to send message: " << message->error_message();
      }
  }

  void ChatClient::onLoginResponse(const TcpConnectionPtr &conn,
                       const LoginResponsePtr &message,
                       Timestamp)
  {
    LOG_INFO << "onLoginResponse: " << message->GetTypeName();

    if (message->success())
    {
      LOG_INFO << "Login succeeded";
      {
      MutexLockGuard lock(username_mutex_);
      username_ = message->username();
      }
      emit loginResponseReceived(true, message->username());
    }
    else
    {
      LOG_ERROR << "Login failed: " << message->error_message();
      emit loginResponseReceived(false, message->error_message());
    }
  }

    void ChatClient::onLogoutResponse(const TcpConnectionPtr &conn,
                        const LogoutResponsePtr &message,
                        Timestamp)
  {
    LOG_INFO << "onLogoutResponse: " << message->GetTypeName();

    if (message->success())
    {
      LOG_INFO << "Logout succeeded";
      // MutexLockGuard lock(username_mutex_);
      // username_ = "guest";
    }
    else
    {
      LOG_ERROR << "Logout failed: " << message->error_message();
    }
  }

    void ChatClient::onRegisterResponse(const TcpConnectionPtr &conn,
                          const RegisterResponsePtr &message,
                          Timestamp)
  {
    LOG_INFO << "onRegisterResponse: " << message->GetTypeName();

    if (message->success())
    {
      LOG_INFO << "Register succeeded";
      emit registerResponseReceived(true);
    }
    else
    {
      LOG_ERROR << "Register failed: " << message->error_message();
      emit registerResponseReceived(false);
    }
  }

    void ChatClient::onSearchResponse(const TcpConnectionPtr &conn,
                        const SearchResponsePtr &message,
                        Timestamp)
  {
    LOG_INFO << "onSearchResponse: " << message->GetTypeName();
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
      LOG_INFO << "onGroupResponse: " << message->GetTypeName();

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
          LOG_INFO << operation << " succeeded";
      }
      else
      {
          LOG_ERROR << "Group operation failed: " << message->error_message();
      }
      std::vector<std::string> groups;
      if (message->joined_groups_size() > 0)
      {
          for (const auto &group : message->joined_groups())
          {
                groups.push_back(group);
          }
      }
        emit groupResponseReceived(message->success(), message->operation(),groups);
  }

  void ChatClient::onTextMessage(const TcpConnectionPtr &conn,
                     const TextMessagePtr &message,
                     Timestamp)
  {
    LOG_INFO << "onTextMessage: " << message->GetTypeName();
    bool is_group = message->target_type() == chat::TargetType::GROUP;
    std::string group;
    if (message->target_type() == chat::TargetType::USER){
        group = "private";
    } 
    else if (message->target_type() == chat::TargetType::GROUP)
    {
        group = message->target();
    }
    printf("<<< %s [%s] %s\n", group.c_str(), message->sender().c_str(), ac_.filter(message->content()).c_str());
    emit textMessageReceived(is_group, group, message->sender(), message->content());
  }

  void ChatClient::onUnknownMessageType(const TcpConnectionPtr &conn,
                            const MessagePtr &message,
                            Timestamp)
  {
    LOG_INFO << "onUnknownMessageType: " << message->GetTypeName();
  }
