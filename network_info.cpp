#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <vector>
#include <tuple>
#include <sstream>


class RoutingTable{
    public:
        std::string interfaceName;
        std::string genmask;
        std::string gateway;
        std::string destination;
};

class NetworkInterface{
    public:
        std::string name;
        std::string ipAddress;
        std::string macAddress;
        std::string broadcastAddress;
};

/*
* Iface   Destination     Gateway         Flags   RefCnt  Use     Metric  Mask            MTU     Window  IRTT                                                       
* ens33   00000000        0217A8C0        0003    0       0       100     00000000        0       0       0                                                                            
* ens33   0000FEA9        00000000        0001    0       0       1000    0000FFFF        0       0       0                                                                           
* docker0 000011AC        00000000        0001    0       0       0       0000FFFF        0       0       0                                                                            
* ens33   0017A8C0        00000000        0001    0       0       100     00FFFFFF        0       0       0                                                                            
*/
std::vector<RoutingTable> getRoutingTableInfo(){
    std::vector<RoutingTable> routingTable;
    RoutingTable row;
    std::ifstream procNetRoute("/proc/net/route");
    std::string line;
    if(procNetRoute.is_open()){
        while (std::getline(procNetRoute, line)) {
            if (line.find("Iface") != std::string::npos)
                continue;
            std::istringstream iss(line);
            std::string iface, dest, gw, flags, _refcnt, _use, _metric, mask, mtu, _window, _irtt;
            iss >> iface >> dest >> gw >> flags >> _refcnt >> _use >> _metric >> mask >> _window >> _irtt;
            if (!iface.empty() && !dest.empty() && !gw.empty() &&! mask.empty()){
                row.interfaceName = iface;
                row.destination = inet_ntoa({static_cast<unsigned int>(std::stoul(dest, 0, 16))});
                row.gateway = inet_ntoa({static_cast<unsigned int>(std::stoul(gw, 0, 16))});
                row.genmask = inet_ntoa({static_cast<unsigned int>(std::stoul(mask, 0, 16))});
                
                routingTable.push_back(row);
            }
        }
    }
    return routingTable;
}

std::vector<NetworkInterface> getInerfacesInfo() {
    std::vector<NetworkInterface> networkInterfaces;

    int sockfd;
    struct ifreq ifr;
    struct ifconf ifc;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to open socket\n";
        return networkInterfaces;
    }

    char buffer[4096];
    ifc.ifc_buf = buffer;
    ifc.ifc_len = sizeof(buffer);

    if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
        std::cerr << "Failed to get interface list\n";
        close(sockfd);
        return networkInterfaces;
    }

    int interface_len = ifc.ifc_len / sizeof(struct ifreq);
    for (int i = 0; i < interface_len; ++i) {
        NetworkInterface interface;

        //IP Address
        std::memset(&ifr, 0, sizeof(ifr));
        std::strcpy(ifr.ifr_name, ifc.ifc_req[i].ifr_name);

        interface.name = ifr.ifr_name;
        if (ioctl(sockfd, SIOCGIFADDR, &ifr) >= 0) {
            struct sockaddr_in* ipAddr = (struct sockaddr_in*)&ifr.ifr_addr;
            interface.ipAddress = inet_ntoa(ipAddr->sin_addr);
        }

        // Get MAC address
        if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) >= 0) {
            unsigned char* mac = (unsigned char*)ifr.ifr_hwaddr.sa_data;
            char buffer[18];
            snprintf(buffer, sizeof(buffer), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            interface.macAddress = buffer;
        }

        // Get broadcast address
        if (ioctl(sockfd, SIOCGIFBRDADDR, &ifr) >= 0) {
            struct sockaddr_in* broadAddr = (struct sockaddr_in*)&ifr.ifr_broadaddr;
            interface.broadcastAddress = inet_ntoa(broadAddr->sin_addr);
        }
        networkInterfaces.push_back(interface);
    }
    close(sockfd);
    return networkInterfaces;
}

int main() {
    std::vector<NetworkInterface> interfaces = getInerfacesInfo();
    std::vector<RoutingTable> routingTable = getRoutingTableInfo();

    std::cout << "Network Interfaces\n";
    std::cout << "InterfacesName\tIP Address\tMAC Address\tBroadcast Address\n";
    for (const auto& interface : interfaces){
        std::cout << interface.name << "\t" << interface.ipAddress << "\t" << interface.macAddress << "\t" << interface.broadcastAddress << "\n";
    }
    std::cout << "\n";

    std::cout << "Routing Table\n";
    std::cout << "InterfaceName\tGateway\tGenmask\tDestination\n";
    for (const auto& row : routingTable){
        std::cout << row.destination << "\t" << row.gateway << "\t" << row.genmask << "\t" << row.interfaceName << "\n";
    }
    return 0;
}
