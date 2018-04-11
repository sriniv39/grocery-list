const char * usage =
"                                                               \n"
"IRCServer:                                                   \n"
"                                                               \n"
"Simple server program used to communicate multiple users       \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   IRCServer <port>                                          \n"
"                                                               \n"
"Where 1024 < port < 65536.                                     \n"
"                                                               \n"
"In another window type:                                        \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where talk-server      \n"
"is running. <port> is the port number you used when you run    \n"
"daytime-server.                                                \n"
"                                                               \n";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include <algorithm>

#include "IRCServer.h"

//#include <iostream>
//#include <string>
//#include <vector>

using namespace std;

int QueueLength = 5;

//test

int
IRCServer::open_server_socket(int port) {

	// Set the IP address and port for this server
	struct sockaddr_in serverIPAddress; 
	memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
	serverIPAddress.sin_family = AF_INET;
	serverIPAddress.sin_addr.s_addr = INADDR_ANY;
	serverIPAddress.sin_port = htons((u_short) port);

	// Allocate a socket
	int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
	if ( masterSocket < 0) {
		perror("socket");
		exit( -1 );
	}

	// Set socket options to reuse port. Otherwise we will
	// have to wait about 2 minutes before reusing the sae port number
	int optval = 1; 
	int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, 
			(char *) &optval, sizeof( int ) );

	// Bind the socket to the IP address and port
	int error = bind( masterSocket,
			(struct sockaddr *)&serverIPAddress,
			sizeof(serverIPAddress) );
	if ( error ) {
		perror("bind");
		exit( -1 );
	}

	// Put socket in listening mode and set the 
	// size of the queue of unprocessed connections
	error = listen( masterSocket, QueueLength);
	if ( error ) {
		perror("listen");
		exit( -1 );
	}

	return masterSocket;
}

	void
IRCServer::runServer(int port)
{
	int masterSocket = open_server_socket(port);

	initialize();

	while ( 1 ) {

		// Accept incoming connections
		struct sockaddr_in clientIPAddress;
		int alen = sizeof( clientIPAddress );
		int slaveSocket = accept( masterSocket,
				(struct sockaddr *)&clientIPAddress,
				(socklen_t*)&alen);

		if ( slaveSocket < 0 ) {
			perror( "accept" );
			exit( -1 );
		}

		// Process request.
		processRequest( slaveSocket );		
	}
}

	int
main( int argc, char ** argv )
{
	// Print usage if not enough arguments
	if ( argc < 2 ) {
		fprintf( stderr, "%s", usage );
		exit( -1 );
	}

	// Get the port from the arguments
	int port = atoi( argv[1] );

	IRCServer ircServer;

	// It will never return
	ircServer.runServer(port);

}

//
// Commands:
//   Commands are started y the client.
//
//   Request: ADD-USER <USER> <PASSWD>\r\n
//   Answer: OK\r\n or DENIED\r\n
//
//   REQUEST: GET-ALL-USERS <USER> <PASSWD>\r\n
//   Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//
//   REQUEST: CREATE-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LIST-ROOMS <USER> <PASSWD>\r\n
//   Answer: room1\r\n
//           room2\r\n
//           ...
//           \r\n
//
//   Request: ENTER-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LEAVE-ROOM <USER> <PASSWD>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: SEND-MESSAGE <USER> <PASSWD> <MESSAGE> <ROOM>\n
//   Answer: OK\n or DENIED\n
//
//   Request: GET-MESSAGES <USER> <PASSWD> <LAST-MESSAGE-NUM> <ROOM>\r\n
//   Answer: MSGNUM1 USER1 MESSAGE1\r\n
//           MSGNUM2 USER2 MESSAGE2\r\n
//           MSGNUM3 USER2 MESSAGE2\r\n
//           ...\r\n
//           \r\n
//
//    REQUEST: GET-USERS-IN-ROOM <USER> <PASSWD> <ROOM>\r\n
//    Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//

	void
IRCServer::processRequest( int fd )
{
	// Buffer used to store the comand received from the client
	const int MaxCommandLine = 1024;
	char commandLine[ MaxCommandLine + 1 ];
	int commandLineLength = 0;
	int n;

	// Currently character read
	unsigned char prevChar = 0;
	unsigned char newChar = 0;

	//
	// The client should s/initiend COMMAND-LINE\n
	// Read the name of the client character by character until a
	// \n is found.
	//

	// Read character by character until a \n is found or the command string is full.
	while ( commandLineLength < MaxCommandLine &&
			read( fd, &newChar, 1) > 0 ) {

		if (newChar == '\n' && prevChar == '\r') {
			break;
		}

		commandLine[ commandLineLength ] = newChar;
		commandLineLength++;

		prevChar = newChar;
	}

	// Add null character at the end of the string
	// Eliminate last \r
	commandLineLength--;
	commandLine[ commandLineLength ] = 0;

	printf("RECEIVED: %s\n", commandLine);

	printf("The commandLine has the following format:\n");
	printf("COMMAND <user> <password> <arguments>. See below.\n");
	printf("You need to separate the commandLine into those components\n");
	printf("For now, command, user, and password are hardwired.\n");
	//do string toke to unhardwhire input to null terminating character
	const char * command;
	const char * user;
	const char * password;
	const char * args;

	//char * command;
	const char s[2] = " ";
	command  = strtok(commandLine, s);
	user = strtok(NULL, s);
	password = strtok(NULL, s);
	args = strtok(NULL, "\0");


	//	char * token = strtok(commandLine,"\0");
	printf("command=%s\n", command);
	printf("user=%s\n", user);
	printf( "password=%s\n", password );
	printf("args=%s\n", args);

	if (!strcmp(command, "ADD-USER")) {
		addUser(fd, user, password, args);
	}
	else if (!strcmp(command, "CREATE-ROOM")) {
		createRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "ENTER-ROOM")) {
		enterRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "LEAVE-ROOM")) {
		leaveRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "SEND-MESSAGE")) {
		sendMessage(fd, user, password, args);
		
	}
	else if(!strcmp(command, "LIST-ROOMS")){
		listRooms(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-MESSAGES")) {
		getMessages(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-USERS-IN-ROOM")) {
		getUsersInRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-ALL-USERS")) {
		getAllUsers(fd, user, password, args);
	}
	else {
		const char * msg =  "UNKNOWN COMMAND\r\n";
		write(fd, msg, strlen(msg));
	}

	// Send OK answer
	//const char * msg =  "OK\n";
	//write(fd, msg, strlen(msg));

	close(fd);	
}

	void
IRCServer::initialize()
{
	// Open password file

	// Initialize users in room

	// Initalize message list

}

bool
IRCServer::checkPassword(int fd, const char * user, const char * password) {
	// Here check the password
	//string name = string(user) + "\r\n";
	//write(fd, name.c_str(), strlen(name.c_str()));
	//	string word = string(password) + "\r\n";
	//	write(fd, word.c_str(), strlen(word.c_str()));

	for(int i = 0; i < person.size(); i++){
		string str = person[i].username + "\r\n";
	//	write(fd, str.c_str(), strlen(str.c_str()));
		string pass = person[i].pass + "\r\n";
	//	write(fd, pass.c_str(), strlen(pass.c_str()));

		if(!(person[i].username.compare(user)) && !(person[i].pass.compare(password))){
			return true;

		}

	}


	return false;
}

	void
IRCServer::addUser(int fd, const char * user, const char * password, const char * args)
{
	//method works
	
	for(int i = 0; i < person.size(); i++){
		if(person[i].username.compare(user) == 0){
			const char * deny = "DENIED\r\n";
			write(fd, deny, strlen(deny));
			return;
		}
		
		// 	USER input;
        	//	input.username = user;
        	//	input.pass = password;
		//	person.push_back(input);
        	//	const char * msg =  "OK\r\n";
        	//	write(fd, msg, strlen(msg));
        	//	return;

	

	}

	USER input;
	input.username = user;
	input.pass = password;


//	for(int j = 0; j < person.size(); j++){
//		if(person[j].username.compare(user) != 0){
//			person.push_back(input);
      //			const char * msg =  "OK\r\n";
    //    		write(fd, msg, strlen(msg));
  //      		return; 
//		}
//	}


	person.push_back(input);
	const char * msg =  "OK\r\n";
	write(fd, msg, strlen(msg));
	return;		
}

void IRCServer::createRoom(int fd, const char * user, const char * password, const char * args){
	
	//method works
	
	if(checkPassword(fd, user, password)){
		for(int i = 0; i < location.size(); i++){
			if(location[i].roomName.compare(args) == 0){
				const char * deny = "DENIED\r\n";
				write(fd, deny, strlen(deny));
				return;		
			}
		}
		ROOM room;
		string nameRoom(args);
		room.roomName = nameRoom;
	
		location.push_back(room);
		const char * msg = "OK\r\n";
		write(fd, msg, strlen(msg)); 
		return;

	}
	const char * msg = "ERROR (Wrong password)\r\n";
	write(fd, msg, strlen(msg));

}

	void
IRCServer::enterRoom(int fd, const char * user, const char * password, const char * args)
{
	//method works

	if(checkPassword(fd, user, password)){
		for(int i = 0; i < location.size(); i++){
			if(location[i].roomName.compare(args) == 0){
			/*	for(int j = 0; j < person.size(); j++){
					if(person[i].username.compare(user) == 0){
						const char * msg =  "OK\r\n";
						write(fd, msg, strlen(msg));
						return;
					}
				} */

				std::vector<string>::iterator it;
                                string uservalue(user);
                                it = find(location[i].roomUser.begin(), location[i].roomUser.end(), uservalue);
                                if(it == location[i].roomUser.end()){
					
					location[i].roomUser.push_back(user);
					const char * msg =  "OK\r\n";
					write(fd, msg, strlen(msg));
					return;
				}
				else{
					const char * msg =  "OK\r\n";
                                        write(fd, msg, strlen(msg));
                                        return;

				}

			}
		}
		const char * deny = "ERROR (No room)\r\n";
		write(fd, deny, strlen(deny));
		return;
	}
	const char * msg = "ERROR (Wrong password)\r\n";
	write(fd, msg, strlen(msg));
}



	void
IRCServer::leaveRoom(int fd, const char * user, const char * password, const char * args)
{ 
	//method works
	
	if(checkPassword(fd, user, password)){
                 for(int i = 0; i < location.size(); i++){
                         if(location[i].roomName.compare(args) == 0){
                                
					 vector <string> totalUser = location[i].roomUser;
                                 	for(int j = 0; j < totalUser.size(); j++){
                                         	if(totalUser[j].compare(user) == 0){
                                                	location[i].roomUser.erase(location[i].roomUser.begin() + j); 
							const char * msg =  "OK\r\n";
                            	                    	write(fd, msg, strlen(msg)); 
	                                            	return;
	                                    	}
					
						
					}
					const char * msg =  "ERROR (No user in room)\r\n";
					write(fd, msg, strlen(msg));
					return;
                          }
 
                                // location[i].roomUser.erase(user);
                            //     const char * msg =  "ERROR (No user in room)\r\n";
                              //   write(fd, msg, strlen(msg));
                                //  return;
 
                         }
                 
		
                 
         }
         const char * msg = "ERROR (Wrong password)\r\n";
         write(fd, msg, strlen(msg));
}

	void
IRCServer::sendMessage(int fd, const char * user, const char * password, const char * args)
{
	//printf("here\n");
	char* arg = strdup(args);
	const char s[2] = " ";
	char* room  = strtok(arg, s);
	char* ls  = strtok(NULL, "\0");
	if(checkPassword(fd, user, password)){
		for(int i = 0; i < location.size(); i++){
			if(location[i].roomName.compare(room) == 0){
			//	if(person[i].username.compare(user) == 0){
				
				std::vector<string>::iterator it;
				string uservalue(user);
				it = find (location[i].roomUser.begin(), location[i].roomUser.end(), uservalue);
				if(it != location[i].roomUser.end()){ 
					ROOM r = location[i];
					if(r.roomMessages.size() == 100){
						r.roomMessages.erase(r.roomMessages.begin() + 1);						
					}
					int messageNum;
					if(r.roomMessages.size() == 0){
						messageNum = 0; 
					}
					else
					{
						int rsize = r.roomMessages.size();
						MESSAGE last = r.roomMessages[rsize-1];
						messageNum = last.messageNumber + 1;	
					}
			
					MESSAGE m;
					m.messageNumber = messageNum;
					string content(ls);
					m.messageContent = content;
					string sender(user);
					m.messageSender = sender;
					location[i].roomMessages.push_back(m);
					const char * msg =  "OK\r\n";
                                        write(fd, msg, strlen(msg));
                                        return;		
				}
				else{
					const char * errusr = "ERROR (user not in room)\r\n";
					write(fd, errusr, strlen(errusr));
				}
									
			//	}
				
			}
		}	
	}else{
                 const char * errmsg1 = "ERROR (Wrong password)\r\n";
                 write(fd, errmsg1, strlen(errmsg1));
         }

}

	void
IRCServer::getMessages(int fd, const char * user, const char * password, const char * args)
{
	char* arg = strdup(args);
	const char s[2] = " ";
	char* mNum  = strtok(arg, s);
	int mNumber = atoi(mNum);
	//char* msgBegin  = strtok(NULL, s);
	char* room = strtok(NULL, "\0");
	bool messages = false;
	//const char * errmsg = "initial)\r\n";
        //write(fd, errmsg, strlen(errmsg));
	if(checkPassword(fd, user, password)){
	//	write(fd, room, strlen(room));
	//	const char * errmsg = "password check)\r\n";
          //      write(fd, errmsg, strlen(errmsg));
		for(int i = 0; i < location.size(); i++){
                	//write(fd, location[i].roomName.c_str(), strlen(location[i].roomName.c_str()));
			//fsync(fd);
			//std::vector<string>::iterator it;
			//string uservalue(user);
			//it = find(location[i].roomUser.begin(), location[i].roomUser.end(), uservalue);
			if(location[i].roomName.compare(room) == 0){
				std::vector<string>::iterator it;
				string uservalue(user);
				it = find (location[i].roomUser.begin(), location[i].roomUser.end(), uservalue);
				if(it != location[i].roomUser.end()){

					ROOM r = location[i];
					char msg[100];
				//sprintf(msg, "%d\n", r.roomMessages.size());
				//write(fd, msg, strlen(msg));
				
					for(int j = (mNumber + 1); j < r.roomMessages.size();j++){
						messages = true;
						int messageNum = r.roomMessages[j].messageNumber;
					//const char * errmsg111 = "message forloop\r\n";
                			//write(fd, errmsg, strlen(errmsg111));
						string u = r.roomMessages[j].messageSender;
						string message = r.roomMessages[j].messageContent;
				//	for(int k = msgBegin; k < 100; k++){
						char buffer [250];
						int n = sprintf (buffer, "%d %s %s\r\n", messageNum, u.c_str(), message.c_str());
						write(fd, buffer, strlen(buffer));
				//	}
				}
				if(!messages){
					const char * errusr6 = "NO-NEW-MESSAGES\r\n";
                                          write(fd, errusr6, strlen(errusr6));
                                         return;
				}
				}
				else{
					const char * errusr = "ERROR (User not in room)\r\n";
                                         write(fd, errusr, strlen(errusr));
					return;
				}
			}
		}
		const char * line = "\r\n";
		write(fd, line, strlen(line));
		//	const char * errmsg8 = "ERROR (No Messages)\r\n"; //where do i put this
         	//	write(fd, errmsg8, strlen(errmsg8));
	}else{
		const char * errmsg1 = "ERROR (Wrong password)\r\n";
         	write(fd, errmsg1, strlen(errmsg1));
	}

}

	void
IRCServer::getUsersInRoom(int fd, const char * user, const char * password, const char * args)
{	
	//method works  
	
	char* arg = strdup(args);
        const char s[2] = " ";

        //char* mNum  = strtok(arg, s);
        //char* room  = strtok(arg, s);
	if(checkPassword(fd, user, password)){
		for(int i = 0; i < location.size(); i++){
			if(location[i].roomName.compare(arg) == 0){
				vector <string> totalUsersInRoom = location[i].roomUser;
				std::sort (totalUsersInRoom.begin(), totalUsersInRoom.end());
				for(int j = 0; j < totalUsersInRoom.size(); j++){
					//std::vector<string>::iterator it;
					//string uservalue(user);
					//it = find(location[i].roomUser.begin(), location[i].roomUser.end(), uservalue);
					//if(it != location[i].roomUser.end()){	
				
							write(fd, totalUsersInRoom[j].c_str(), totalUsersInRoom[j].length());
							write(fd, "\r\n", 2);
					//}
					//	}
					//}
				}
			}
		//	write(fd, "\r\n", 2);
                //	return;
		}
		write(fd, "\r\n", 2);
		return;
	}
	const char * msg = "ERROR (Wrong password)\r\n";
	write(fd, msg, strlen(msg));
}

bool sortUserFunction (struct USER &u1, struct USER &u2){
	//added method, sorts users for getAllUsers
	return u1.username.compare(u2.username);
}
void
IRCServer::getAllUsers(int fd, const char * user, const char * password,const  char * args)
{
	//method works

	if(checkPassword(fd, user, password)){
		std::sort(person.begin(), person.end(),sortUserFunction);
		vector <string> temp;
		for(int j = 0; j < person.size(); j++){
			temp.push_back(person[j].username);
			//string totalUsers = person[j].username + "\r\n"; //location.roomuser
			//vector <string> totalUsers = person[j].username;
			//std::sort (totalUsers.begin(), totalUsers.end());
			//write(fd, totalUsers, (size_t)strlen(totalUsers));
		}	
		std::sort(temp.begin(), temp.end());
		for(int k = 0; k < temp.size(); k++){
	
		
			write(fd, temp[k].c_str(), temp[k].length());
				//write(fd, totalUsers.c_str(), totalUsers.length());
			write(fd, "\r\n", 2);
				
		}
		write(fd, "\r\n", 2);
		return;
	}	
	const char * msg = "ERROR (Wrong password)\r\n";
        write(fd, msg, strlen(msg));	
}

bool sortRoomFunction(struct ROOM &r1, struct ROOM &r2){
	return r1.roomName.compare(r2.roomName);
}


void
IRCServer::listRooms(int fd, const char * user, const char * password, const char * args){
	if(checkPassword(fd, user, password)){
		std::sort(location.begin(), location.end(), sortRoomFunction);
		vector <string> temp;
		for(int j = 0; j < location.size(); j++){
			temp.push_back(location[j].roomName);
		}
		std::sort(temp.begin(), temp.end());
		for(int k = 0; k < temp.size(); k++){
			write(fd, temp[k].c_str(), temp[k].length());
			write(fd, "\r\n", 2);
		}
		write(fd, "\r\n", 2);
		return;
	}
	const char * msg = "ERROR (Wrong password)\r\n";
	write(fd, msg, strlen(msg));
}

