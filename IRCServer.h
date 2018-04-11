
#ifndef IRC_SERVER
#define IRC_SERVER
#include <iostream>
//#include <string>
//#include <vector>
struct USER;
struct ROOM;
struct MESSAGE;
struct IRCServer;
using namespace std;

#define PASSWORD_FILE "password.txt"
bool sortFunction (struct USER &u1, struct USER &u2);
struct USER {
	string username;
	string pass;	
};

struct ROOM{
	string roomName;
	vector <string> roomUser;
	vector <MESSAGE> roomMessages;	
};

struct MESSAGE{
	int  messageNumber;
	string messageSender;
	string messageContent;
};


class IRCServer {
	// Add any variables you need

private:
	int open_server_socket(int port);
	vector <USER> person;
	vector <ROOM> location;
//	bool sortFunction (struct USER u1, struct USER u2);
public:
	void initialize();
	bool checkPassword(int fd, const char * user, const char * password);
	void processRequest( int socket );
	void addUser(int fd, const char * user, const char * password, const char * args);
	void enterRoom(int fd, const char * user, const char * password, const char * args);
	void leaveRoom(int fd, const char * user, const char * password, const char * args);
	void sendMessage(int fd, const char * user, const char * password, const char * args);
	void getMessages(int fd, const char * user, const char * password, const char * args);
	void getUsersInRoom(int fd, const char * user, const char * password, const char * args);
	void getAllUsers(int fd, const char * user, const char * password, const char * args);
	void runServer(int port);
	void createRoom(int fd, const char * user, const char * password, const char * args);
	void listRooms(int fd, const char * user, const char * password, const char * args);
	

};

#endif
