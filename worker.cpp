#include "worker.h"
#define WIN32_LEAN_AND_MEAN
#include <winsock.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <wlanapi.h>
#include <iostream>
#include <chrono>
#include <thread>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "wlanapi.lib")

Worker::Worker(QObject* parent) : QObject(parent) {}

void Worker::stop() {
    running = false;
}

double Worker::pingHost(const char* ip)
{
    HANDLE hIcmp = IcmpCreateFile();
    if (hIcmp == INVALID_HANDLE_VALUE)
        return -1;

    char sendData[] = "ping";
    DWORD replySize = sizeof(ICMP_ECHO_REPLY) + sizeof(sendData);
    std::vector<char> reply(replySize);

    auto start = std::chrono::high_resolution_clock::now();

    DWORD res = IcmpSendEcho(
        hIcmp,
        inet_addr(ip),
        sendData,
        sizeof(sendData),
        nullptr,
        reply.data(),
        replySize,
        1000
    );

    auto end = std::chrono::high_resolution_clock::now();

    IcmpCloseHandle(hIcmp);

    if (res == 0)
        return -1;

    return std::chrono::duration<double, std::milli>(end - start).count();
}

double Worker::getWifiSignal() {
    HANDLE hClient = nullptr;
    DWORD version = 0;

    if (WlanOpenHandle(2, nullptr, &version, &hClient) != ERROR_SUCCESS)
        return -1;

    PWLAN_INTERFACE_INFO_LIST pIfList = nullptr;

    if (WlanEnumInterfaces(hClient, nullptr, &pIfList) != ERROR_SUCCESS) {
        WlanCloseHandle(hClient, nullptr);
        return -1;
    }

    if (pIfList->dwNumberOfItems == 0) {
        WlanFreeMemory(pIfList);
        WlanCloseHandle(hClient, nullptr);
        return -1;
    }

    GUID guid = pIfList->InterfaceInfo[0].InterfaceGuid;

    PWLAN_CONNECTION_ATTRIBUTES pConnectInfo = nullptr;
    DWORD dataSize = 0;
    WLAN_OPCODE_VALUE_TYPE opCode;

    DWORD res = WlanQueryInterface(
        hClient,
        &guid,
        wlan_intf_opcode_current_connection,
        nullptr,
        &dataSize,
        (PVOID*)&pConnectInfo,
        &opCode
    );

    double signal = -1;

    if (res == ERROR_SUCCESS && pConnectInfo) {
        signal = (double)pConnectInfo->wlanAssociationAttributes.wlanSignalQuality;
    }else{
        wprintf(L"WlanQueryInterface failed with error: %u\n", res);
    }

    if (pConnectInfo)
        WlanFreeMemory(pConnectInfo);

    WlanFreeMemory(pIfList);
    WlanCloseHandle(hClient, nullptr);

    return signal;
}

void Worker::run() {
    using clock = std::chrono::steady_clock;
    auto start = clock::now();

    while (running) {
        double t = std::chrono::duration<double>(clock::now() - start).count();

        double ping = pingHost(mIp.toLatin1().data());
        double signal = getWifiSignal();

        emit newData(t, ping, signal);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}