#include"chat_server.h"
int main()
{
    ChatServer server;
    server.listen(IP,PORT);
    return 0;
}