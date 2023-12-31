#include "tcp-server.h"

TcpServer::TcpServer(void) {
    memset(&addr, 0 ,sizeof(addr));
    memset(&addr6, 0 ,sizeof(addr6));

    addr.sin_family = AF_INET;
    addr6.sin6_family = AF_INET6;
    
    // server는 ip주소를 줄 필요가 없다. -> 위의 memset으로 해결했지만 제대로 명시하기 위해 남겨둠 
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr6.sin6_addr = in6addr_any;
    /*
    addr6.sin6_addr.__in6_u.__u6_addr32[0] = htonl(INADDR_ANY);
    addr6.sin6_addr.__in6_u.__u6_addr32[1] = htonl(INADDR_ANY);
    addr6.sin6_addr.__in6_u.__u6_addr32[2] = htonl(INADDR_ANY);
    addr6.sin6_addr.__in6_u.__u6_addr32[3] = htonl(INADDR_ANY);
    */
    // port는 12345로 고정한다.
    addr.sin_port = htons(12345);
    addr6.sin6_port = htons(12345);
}

TcpServer::TcpServer(char *port) {
    memset(&addr, 0 ,sizeof(addr));
    
    addr.sin_family = AF_INET;
    int port_ = std::stoi(port);
    addr.sin_port = htons(port_);
}

// TcpServer::~TcpServer(void) {
//     close(listen_sock_);
// }

void TcpServer::thread_sock(int &client_sock) {
    // addrlen = sizeof(client_addr)
    // getpeername(client_sock, (struct sockaddr *)&client_addr, &addrlen);
    while (true) {
        int retval = recv(client_sock, buf, BUFSIZ, 0); 
        if (retval == -1) {
            fprintf(stderr, "recv fail\n");
            break;
        }else if (retval == 0) {
            // 입력이 아무것도 없을 때
            break;
        }
        // 받은 데이터 출력
        buf[retval + 1] = '\0';
        printf("\n[TCP %s:%d] %s\n",ipv4, ntohs(client_addr.sin_port), buf);
        // send -> 데이터 보내기, client가 보낸만큼 다시 보내 줄 것
        retval = send(client_sock, buf, retval, 0);
        if (retval == -1) {
            fprintf(stderr, "send error\n");
            break;
        }
    }
    close(client_sock);
}

int TcpServer::run(void) {
    listen_sock_ = socket(addr.sin_family, SOCK_STREAM, 0);
    if (listen_sock_ == -1) {
        fprintf(stderr, "Socket creation error\n");
        return -1;
    }
/*
bind() - 주소정보 할당

○ 함수원형
  int bind(int sockfd, struct sockaddr *myaddr, int addrlen)

○ 리턴 값
  성공 시 : 0, 실패 시 : -1

- sockfd : 주소를 할당하고자 하는 소켓의 파일 디스크립터
- myaddr : 할당하고자 하는 주소 정보를 지니고 있는 sockaddr 구조체 변수의 포인터 인자 값
- addrlen : 인자로 전달된 주소 정보 구조체의 길이

=> 함수 호출이 성공하면 sockfd가 가리키는 소켓에 myaddr이 가리키는 주소 정보가 할당된다.

*/
    if (bind(listen_sock_, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        fprintf(stderr, "Socket bind error\n");
        return -1;
    }
/*
listen 함수호출을 통해서 "연결 요청 대기상태"
*/
    if (listen(listen_sock_, SOMAXCONN) == -1) {
        fprintf(stderr, "Socket bind error\n");
        return -1;
    }
/*
accept: 클라이언트 접속 요청 수락 함수
*/
    while(true) {
        socklen_t addrlen = sizeof(client_addr);
        client_sock_ = accept(listen_sock_, (struct sockaddr *)&client_addr, &addrlen);
        if (client_sock_ == -1) {
            fprintf(stderr, "accept error\n");
            return -1;
        }

        // 접속한 client 정보
        printf("%d\n",client_addr.sin_family);
        printf("%d\n",client_addr.sin_addr.s_addr);
        inet_ntop(AF_INET, &client_addr.sin_addr, ipv4, sizeof(ipv4));
        printf("\n[TCP 서버] client 정보: IP: %s, Port:%d\n", ipv4, ntohs(client_addr.sin_port));
        
        // class 내부 함수에서 정의한 함수를 다른 class 내부 함수에서 쓰려면
        // &TcpServer::thread_sock으로 사용하고 두번째 인자로 참조하려는 클래스 변수명을 넣어줘야함 지금은 this
        std::thread t(&TcpServer::thread_sock, this, std::ref(client_sock_));
        // join으로 하면 안되고 detach로 해야 동시에 처리된다..! detach를 사용하면 스레드가 백그라운드에서 독립적으로 실행되므로 메인 스레드는 계속해서 다른 작업을 수행할 수 있습니다. 이는 네트워크 프로그래밍에서 여러 클라이언트의 동시 접속 또는 비동기 작업을 처리하는 데 유용합니다.
        t.detach();
        
    }
    close(listen_sock_);
    return 0;
}
// only ipv4
bool TcpServer::Setaddr(void) const {
    if (addr.sin_family != AF_INET) {
        fprintf(stderr, "ipv4 family is error\n");
        return false;
    }
    if (!addr.sin_port) {
        fprintf(stderr, "please input port\n");
        return false;
    }
    return true;
}
// unfinished
bool TcpServer::Setaddr6(void) const {
    if (addr6.sin6_family != AF_INET6) {
        fprintf(stderr, "ipv6 family is error\n");
        return false;
    }
    if (!addr6.sin6_port) {
        fprintf(stderr, "please input port\n");
        return false;
    }
    return true;
}