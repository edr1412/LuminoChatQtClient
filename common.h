#ifndef COMMON_H
#define COMMON_H

#include <QTcpSocket>
#include <qDebug>
#include <QDateTime>

#include <iostream>
#include <map>
using namespace std;

#define RET_OK 0
#define RET_ERROR -1
#define RET_AGAIN -2    //重新读取
#define RET_EXIT -3     //客户端退出
#define RET_END -4      //读取结束

#define MAX_SEND_LENGTH 6144

#ifndef FILENAME
#define FILENAME (__FILE__)
#endif

#ifndef FILEFUNCTION
#define FILEFUNCTION  (__FUNCTION__)
#endif

#ifndef FILELINE
#define FILELINE   (__LINE__)
#endif

#define ChatLog qDebug().noquote()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss zzz")\
                                     <<"["<<FILENAME<<":"<<FILELINE<<"]["<<FILEFUNCTION<<"]"

#define ChatLogInfo(INFO) qDebug().noquote()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss zzz")\
                                     <<"["<<FILENAME<<":"<<FILELINE<<"]["<<FILEFUNCTION<<"]"

#define LOGINFO(format, ...)                                                         \
    {                                                                              \
        qDebug("%s [ %s : %d] [%s]>>" format, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss zzz").toStdString().c_str()\
                        , FILENAME,FILELINE, FILEFUNCTION ,##__VA_ARGS__); \
    }
/**
  * 暂时用宏定义代替，后续通过读取配置文件获取
  */
#define SERVER_ADDR "172.16.61.131"
#define SERVER_PORT 1145

const std::vector<std::string> WordsWeHate = {"Steve Jobs", "Tim Cook", "Jony Ive", "Apple Inc.", "iPhone", "iPad", "MacBook", "iMac", "Apple Watch", "iOS", "macOS", "Safari", "Apple Music", "HomePod", "iCloud", "乔布斯", "蒂姆·库克", "强尼·艾维", "苹果公司", "苹果手机", "苹果平板", "麦金塔电脑"};

#endif // COMMON_H
