#include "server.h"
int main(int argc, char **argv)
{
    int port = PORT;
    if (argc > 2)
    {
        fprintf(stderr, "Usage: ./server [port]");
        return 1;
    }
    else if (argc == 2)
    {
        port = atoi(argv[1]);
    }
    Server server(port);
    server.start();
    return 0;
}