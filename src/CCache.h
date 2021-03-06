/*
** SoluCache by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_CACHE_INCLUDED__
#define __C_CACHE_INCLUDED__
#include <solunet.h>
#include <queue>
#include <map>

class CCachePacket
{
public:
  std::string Key;
  std::vector<unsigned char> Data;
  time_t T;
  time_t Expires;
};

class CCacheIsSet
{
public:
  bool B;
  CCacheIsSet() : B(false) { }
};

class CCacheThread
{
public:
  pthread_t TID;
  bool Done;

  CCacheThread(pthread_t tid)
  {
    TID = tid;
    Done = false;
  }
};

class CCache
{
private:
  bool PopSemaphor, ISemaphor;
  std::vector<CCacheThread> Threads;
  std::map<std::string, CCachePacket> M;
  std::map<std::string, std::queue<CCachePacket> > Q;
  std::map<std::string, CCacheIsSet> S;
  std::map<std::string, std::vector<CCachePacket> > I;
  std::map<std::string, time_t> T;
public:
  CCache() : PopSemaphor(false), ISemaphor(false) {}

  void run();

  static void *nodeThread(void *param);
  void node(solunet::ISocket *socket);

  static void *cleanupThread(void *param);
  void cleanup();
};

#endif
