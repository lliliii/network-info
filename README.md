# Linux Network Information
- 리눅스의 네트워크 정보는 아래의 디렉토리들에 존재함.
  - /proc/net: 커널에 의해 동적으로 생성되며 시스템의 네트워크 상태 및 연결에 대한 정보를 제공함. (TCP/UDP 소켓, 네트워크 인터페이스 통계, ARP 테이블 정보가 존재함) 실제 데이터가 아닌 커널의 상태를 나타내는 파일들이 존재함.
    - 주요 정보는 다음과 같다.
      - /proc/net/route: 라우팅 테이블
      - /proc/net/device: 네트워크 인터페이스의 실시간 통계(bytes, packets, errs, drop, fifo 등)
      - /proc/net/fib-trie: 
      - /proc/net/arp: ARP 캐시 테이블 
  - /sys/class/net/*: 커널이 부팅되고 네트워크 드라이버가 로드될 때 생성되며 명시적으로 로드하여 생성 할 수도 있음. (맥 어드레스, IP 주소, 네트워크 마스크, MTU 등이 존재함) 각 인터페이스의 속성을 나타내는 파일들이 포함됨.
      - 해당 디렉토리마다 네트워크 인터페이스 명으로 된 디렉토리들이 존재하며 주요 정보는 다음과 같다.
        - /sys/class/net/*/address: MAC 어드레스 주소
  - 위 두개를 잘 조합하여 네트워크 정보(MAC주소, IP주소, 라우팅테이블, 게이트웨이)를 알아낼 수 있음.
  - route를 읽어 디바이스 명에 맞는 라우팅 정보를 가져와 fib_trie를 파싱하여 있는 host LOCAL을 찾아 매핑한 뒤, 디바이스의 디렉토리를 찾아가서 mac address 주소를 가져오면 가능함.
  - 다른 프로그램도 이렇게 사용하는지 궁금하여 확인 해본 결과 net-tools의 애플리케이션들도 /proc/net 디렉토리에 있는 파일들을 활용하기도 했음.
  - 일부 정보는 디바이스 드라이버 관련 시스템 콜인 ioctl을 이용하여 ifreq 구조체를 통해 ioctl 매크로로 가져오기도 했음.

## ioctl / ifreq
- ioctl 함수는 "input/output control"의 약자로, Unix 및 Unix 계열 시스템에서 장치와 상호작용하기 위해 사용되는 시스템 호출(system call)임.
- 이 함수는 일반적으로 장치 드라이버나 커널 모듈과 통신하여 장치의 동작을 제어하거나 상태를 조회하는 데 사용됨.

- ifreq 구조체
```
struct ifreq {
char ifr_name[IFNAMSIZ]; /* Interface name */
      union {
      struct sockaddr ifr_addr;
      struct sockaddr ifr_dstaddr;
      struct sockaddr ifr_broadaddr;
      struct sockaddr ifr_netmask;
      struct sockaddr ifr_hwaddr;
      short           ifr_flags;
      int             ifr_ifindex;
      int             ifr_metric;
      int             ifr_mtu;
      struct ifmap    ifr_map;
      char            ifr_slave[IFNAMSIZ];
      char            ifr_newname[IFNAMSIZ];
      char           *ifr_data;
    };
};
```

- 주요 매크로 목록 (ioctl.h)
```
#define SIOCGIFNAME	0x8910		/* get iface name		*/
#define SIOCGIFCONF	0x8912		/* get iface list		*/
#define SIOCGIFFLAGS	0x8913		/* get flags			*/
#define SIOCGIFADDR	0x8915		/* get PA address		*/
#define SIOCGIFDSTADDR	0x8917		/* get remote PA address	*/
#define SIOCGIFBRDADDR	0x8919		/* get broadcast PA address	*/
#define SIOCGIFNETMASK	0x891b		/* get network PA mask		*/
#define SIOCGIFMETRIC	0x891d		/* get metric			*/
#define SIOCGIFMEM	0x891f		/* get memory address (BSD)	*/
#define SIOCGIFMTU	0x8921		/* get MTU size			*/
#define SIOCGIFENCAP	0x8925		/* get/set encapsulations       */
#define SIOCGIFHWADDR	0x8927		/* Get hardware address		*/
#define SIOCGIFSLAVE	0x8929		/* Driver slaving support	*/
#define SIOCGIFINDEX	0x8933		/* name -> if_index mapping	*/
#define SIOCGIFCOUNT	0x8938		/* get number of devices */
#define SIOCGIFBR	0x8940		/* Bridging support		*/
#define SIOCGIFTXQLEN	0x8942		/* Get the tx queue length	*/
```

# Reference
- https://stackoverflow.com/questions/5281341/get-local-network-interface-addresses-using-only-proc
- https://stackoverflow.com/questions/43124364/how-to-get-interface-name-from-ipv4-ip-address-using-only-fs-and-bash-in-linux
- https://github.com/giftnuss/net-tools
