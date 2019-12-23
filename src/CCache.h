#ifndef __C_CACHE_INCLUDED__
#define __C_CACHE_INCLUDED__
#include <solusek/solusek.h>
#include <queue>

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

class CCache
{
private:
  bool PopSemaphor, ISemaphor;
  std::map<std::string, CCachePacket> M;
  std::map<std::string, std::queue<CCachePacket> > Q;
  std::map<std::string, CCacheIsSet> S;
  std::map<std::string, std::vector<CCachePacket> > I;
  std::map<std::string, time_t> T;
public:
  CCache() : PopSemaphor(false), ISemaphor(false) {}

  void run();

  static void *nodeThread(void *param);
  void node(solusek::INetHandlerSocket *socket);

  static void *cleanupThread(void *param);
  void cleanup();
};

#endif
