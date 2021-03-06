/*
** SoluCache by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#include "CCache.h"
#include <unistd.h>
#include <signal.h>

static bool Interrupt = false;

solunet::ISocket *g_MainSocket = 0;

void interruptCallback(int sig)
{
	printf("Interrupt signal called.\n");
	Interrupt = true;
	solunet::ISocket *socket = solunet::createSocket();
	socket->connect("127.0.0.1", 18001);
	int action = 0;
	socket->writeBuffer(&action, 4);
	socket->dispose();
}

class CCacheThreadPackage
{
public:
  CCache *Cache;
  solunet::ISocket *Socket;
};

void *CCache::nodeThread(void *param)
{
  CCacheThreadPackage *pkg = (CCacheThreadPackage*)param;
  pkg->Cache->node(pkg->Socket);
	delete pkg;
  pthread_exit(0);
  return 0;
}

std::string readKey(solunet::ISocket *socket)
{
	std::string key;
	size_t sz = 0;
	socket->readBuffer(&sz, 8);
	for(size_t n = 0; n < sz; n++)
	{
		char c;
		socket->readBuffer(&c, 1);
		key.push_back(c);
	}
	return key;
}

std::vector<unsigned char> readData(solunet::ISocket *socket)
{
	std::vector<unsigned char> data;
	size_t sz = 0;
	socket->readBuffer(&sz, 8);
  data.reserve(sz);
  for(size_t n = 0; n < sz; n++)
  {
 	 unsigned char c;
 	 socket->readBuffer(&c, 1);
 	 data.push_back(c);
  }
	return data;
}

void writeData(solunet::ISocket *socket, std::vector<unsigned char> data)
{
	size_t sz = data.size();
	socket->writeBuffer(&sz, 8);
	for(std::vector<unsigned char>::iterator it = data.begin(); it != data.end(); ++it)
	{
		socket->writeBuffer(&(*it), 1);
	}
}

void CCache::node(solunet::ISocket *socket)
{
	socket->setThrowExceptions(true);

	try
	{
		while(true)
		{
		  int action = 0;
		  socket->readBuffer(&action, 4);

			if(action == 1)
			{
				CCachePacket packet;
				packet.T = time(0);
				packet.Expires = 0;
				size_t sz = 0;
				packet.Key = readKey(socket);
				fprintf(stdout, "Action detected: PUT: %s\n", packet.Key.c_str());
				packet.Data = readData(socket);

				M[packet.Key] = packet;
			}
			else if(action == 2)
			{
				std::string key = readKey(socket);
				if(M.count(key) > 0)
				{
					fprintf(stdout, "Action detected: GET: %s\n", key.c_str());
					CCachePacket packet = M[key];
					writeData(socket, packet.Data);
				}
				else
				{
					size_t r = 0;
					socket->writeBuffer(&r, 8);
				}
			}
		  else if(action == 3)
		  {
		    CCachePacket packet;
				packet.T = time(0);
				packet.Expires = 0;
		    packet.Key = readKey(socket);
				fprintf(stdout, "Action detected: QUEUE-PUSH: %s\n", packet.Key.c_str());
				packet.Data = readData(socket);

				while(PopSemaphor)
					sleep(1);
				PopSemaphor = true;
		    Q[packet.Key].push(packet);
				PopSemaphor = false;
		  }
		  else if(action == 4)
		  {
				std::string key = readKey(socket);

				int ret = 0;
				while(PopSemaphor)
					sleep(1);
				PopSemaphor = true;
				if(Q[key].size() == 0)
				{
					PopSemaphor = false;
					socket->writeBuffer(&ret, 4);
				}
				else
				{
					fprintf(stdout, "Action detected: QUEUE-POP: %s\n", key.c_str());
					ret = 1;
			    CCachePacket packet = Q[key].front();
					Q[key].pop();
					PopSemaphor = false;

					writeData(socket, packet.Data);
				}
		  }
			else if(action == 5)
			{
				std::string key = readKey(socket);
				if(M.count(key) > 0)
				{
					fprintf(stdout, "Action detected: EXPIRE: %s\n", key.c_str());
					time_t t;
					socket->readBuffer(&t, 8);
					M[key].Expires = time(0) + t;
				}
			}
		}
	}
	catch(int e)
	{
		fprintf(stdout, "Socket closed.\n");
	}
	//fprintf(stdout, "CLEANUP\n");
  socket->dispose();
}

void *CCache::cleanupThread(void *param)
{
  CCache *queue = (CCache*)param;
	queue->cleanup();
  pthread_exit(0);
  return 0;
}

void CCache::cleanup()
{
		time_t now;
		bool removed;
		while(!Interrupt)
		{
			removed = false;
			now = time(0);
			for(std::map<std::string, CCachePacket>::iterator it = M.begin(); it != M.end(); ++it)
			{
				if(it->second.Expires != 0 && it->second.Expires < now)
				{
					M.erase(it);
					fprintf(stdout, "Expired: %s\n", it->first.c_str());
					break;
				}
			}
			if(removed)
				continue;
			sleep(1);
		}
}

void CCache::run()
{
	fprintf(stdout, "Cache Initialize!\n");
	pthread_attr_t tattr;
	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr,PTHREAD_CREATE_DETACHED);

	pthread_t cleanupThreadId = 0;
	pthread_create(&cleanupThreadId, 0, cleanupThread, (void*)this);

  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = interruptCallback;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);

  solunet::ISocket *mainSocket = solunet::createSocket();
  mainSocket->bind(18001);
  mainSocket->listen();
  while(!Interrupt)
  {
    solunet::ISocket *socket = mainSocket->accept();

		if(Interrupt)
		{
			socket->dispose();
			break;
		}

    CCacheThreadPackage *pkg = new CCacheThreadPackage();
    pkg->Cache = this;
    pkg->Socket = socket;
    pthread_t threadId = 0;
    pthread_create(&threadId, &tattr, nodeThread, (void*)pkg);
		Threads.push_back(CCacheThread(threadId));
  }

	pthread_join(cleanupThreadId, 0);

  mainSocket->dispose();
}
