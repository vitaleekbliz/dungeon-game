#include "Network.h"

Network& Network::Get()
{
	static Network instance;
	return instance;
}

std::unordered_set<int> Network::getActivePlayersListID()
{
    std::unordered_set<int> vector;

#ifdef SERVER
    for (auto it = m_connected_clients.begin(); it != m_connected_clients.end(); it++) {
        vector.insert(it->first);
    }
#endif
    return vector;
}

bool Network::Init()
{
    if (SDL_Init(0) == -1) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        throw "Can't init sdl"; 
        return false;
    }

    if (SDLNet_Init() == -1) {
        std::cerr << "SDLNet_Init failed: " << SDLNet_GetError() << std::endl;
        SDL_Quit();
        throw "Can't init sdl_net";
        return false;
    }

    
#ifdef SERVER
    if (SDLNet_ResolveHost(&m_serverIP, NULL, m_serverPort) == -1)
#endif
#ifdef CLIENT
    if (SDLNet_ResolveHost(&m_serverIP, m_serverIPString, m_serverPort) == -1)
#endif
    {
        std::cerr << "SDLNet_ResolveHost failed: " << SDLNet_GetError() << std::endl;
        SDLNet_Quit();
        SDL_Quit();
        throw "Can't resolve host!";
        return false;
    }

    m_serverSocket = SDLNet_TCP_Open(&m_serverIP);
    if (!m_serverSocket) {
        std::cerr << "SDLNet_TCP_Open failed: " << SDLNet_GetError() << std::endl;
        SDLNet_Quit();
        SDL_Quit();
        throw "Cant open tcp channel!";
        return false;
    }

    m_receive_message = new std::thread([this]() { Receive_messages(); });
    m_message_handler = new MessageHandler;

    return true;
}

void Network::Clear()
{
    delete m_message_handler;
    //TODO join thread somehow
    if(m_receive_message != nullptr)
        delete m_receive_message;

    SDLNet_TCP_Close(m_serverSocket);
    SDLNet_Quit();
    //concern SDL_QUIT should be called on server in this case
    //might case error if still using sdl after network shut down for main menu e.x.;
    if (server)
        SDL_Quit();
}

void Network::Send_message(int size, int* buffer, int client_id)
{
#ifdef SERVER
    if (client_id == -1)
        for (auto it = m_connected_clients.begin(); it != m_connected_clients.end(); it++) {
            buffer[3] = it->first;

            SDLNet_TCP_Send(it->second, buffer, size);
        }
    else
        if(m_connected_clients.find(client_id)!=m_connected_clients.end())
            SDLNet_TCP_Send(m_connected_clients[client_id], buffer, size);
#endif

#ifdef CLIENT
        SDLNet_TCP_Send(m_serverSocket, buffer, size);
#endif
}

void Network::Add_message_to_que(int size, int* buffer)
{
    size /= sizeof(int);
    ///TEMP
    /*std::cout << "I want to create message : ";
    for (int i = 0; i < size; i++)
        std::cout << buffer[i] << ' ';
    std::cout << '\n';*/
    ///

    for (int i = 0; i < size - 1; i++) {
        if (buffer[i] == Header.h1 && buffer[i + 1] == Header.h2) {
            for (int j = i + 2; j < size - 1; j++) {
                if (buffer[j] == Footer.f1 && buffer[j + 1] == Footer.f2) {

                    Message* mesG = new Message(j - i + 2, buffer + i);
                    m_message_handler->Add_message_to_que(mesG);

                    //j+1 because i++ it means i = j+2 in the next itr  
                    i = j + 1;
                    break;
                }
            }
        }
    }
    
}

void Network::Receive_messages()
{
#ifdef CLIENT
    // Communication with the server

    while (true) {

        int size = SDLNet_TCP_Recv(m_serverSocket, m_buffer, m_buffer_size);
        if (size < 0) {
            std::cerr << "SDLNet_TCP_Recv failed: " << SDLNet_GetError() << std::endl;
            break;
        }
        else if (size > 0) {
            Add_message_to_que(size, m_buffer);
        }
        else
            std::cout << "Size = 0";

        //SDL_Delay(20);
    }

#endif


#ifdef SERVER

    while (true) {
        // Create a set of sockets to monitor for activity
        SDLNet_SocketSet socketSet = SDLNet_AllocSocketSet(m_connected_clients.size() + 1);
        SDLNet_TCP_AddSocket(socketSet, m_serverSocket);

        // Add all connected clients' sockets to the set
        for (const auto& clientPair : m_connected_clients) {
            SDLNet_TCP_AddSocket(socketSet, clientPair.second);
        }

        // Wait for activity on any socket in the set
        int numReady = SDLNet_CheckSockets(socketSet, -1);

        if (SDLNet_SocketReady(m_serverSocket)) {
            TCPsocket clientSocket = SDLNet_TCP_Accept(m_serverSocket);
            if (clientSocket) {
                ++id;

                int buf_size = 6;

                int* buf = new int[buf_size];

                buf[0] = Header.h1;
                buf[1] = Header.h2;
                buf[2] = ESTEBLISH_CONNECTION_ID;
                buf[3] = id;
                buf[4] = Footer.f1;
                buf[5] = Footer.f2;

                SDLNet_TCP_Send(clientSocket, buf, sizeof(int) * buf_size);
                delete[] buf;

                m_connected_clients[id] = clientSocket;

            }
        }

        // Check for activity on connected clients' sockets
        for (auto it = m_connected_clients.begin(); it != m_connected_clients.end(); it++) {
            if (SDLNet_SocketReady(it->second)) {
                

                int size =  SDLNet_TCP_Recv(it->second, m_buffer, m_buffer_size);
                if (size < 0) {
                    SDLNet_TCP_Close(it->second);
                    m_connected_clients.erase(it);
                }else 
                    Add_message_to_que(size, m_buffer);
            }
        }
        SDLNet_FreeSocketSet(socketSet);
    }
#endif
}


int Network::GetID()
{
    return id;
}
 
MessageHandler* Network::GetMessageHandler()
{
    return m_message_handler;
}

void Network::getClientID()
{
    if (!server) {
        while (Network::Get().GetID() == 0) {
            auto& m = Network::Get().GetMessageHandler()->Get_Message_que_by_protocol(ESTEBLISH_CONNECTION_ID);
            if (m.size() > 0) {
                id = m[0]->client_id;
                
            }
        }

        Network::Get().GetMessageHandler()->Clear_by_protocol(ESTEBLISH_CONNECTION_ID);
    }
}