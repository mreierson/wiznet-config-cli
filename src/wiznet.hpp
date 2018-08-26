#pragma once

#include <string>
#include <vector>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/optional.hpp>

struct WiznetPacket
{
	uint8_t macAddress[6];
	uint8_t opMode;
	uint8_t ipAddress[4];
	uint8_t subnetMask[4];
	uint8_t gatewayAddress[4];
	uint8_t serverPort[2];
	uint8_t remoteHostIp[4];
	uint8_t remoteHostPort[2];
	uint8_t baud;
	uint8_t dataBits;
	uint8_t parity;
	uint8_t stopBits;
	uint8_t flowControl;
	uint8_t specialChar;
	uint8_t packingLength[2];
	uint8_t packingInterval[2];
	uint8_t inactivity[2];
	uint8_t printDebug;
	uint8_t version[2];
	uint8_t dhcp;
	uint8_t udp;
	uint8_t connStatus;
	uint8_t ipOrDomain;
	uint8_t dnsAddress[4];
	uint8_t remoteHostDomain[32];
	uint8_t serialConfigFlag;
	uint8_t serialConfigString[3];
	uint8_t ppoeId[32];
	uint8_t ppoePassword[32];
	uint8_t passwordFlag;
	uint8_t password[8];
};

struct WiznetDevice
{
	std::string macAddress;
	std::string ipAddress;
	std::string netmask;
	std::string gateway;
	uint16_t serverPort;
	bool dhcpFlag;
	uint32_t baudRate;
	uint8_t dataBits;
	uint8_t stopBits;
	std::string parity;
	std::string operatingMode;
	uint16_t inactivityTimeout;
	bool clientConnected;
};

struct WiznetConfig
{
	std::string macAddress;
	boost::optional<std::string> ipAddress;
	boost::optional<std::string> netmask;
	boost::optional<std::string> gateway;
	boost::optional<bool> dhcpFlag;
	boost::optional<uint16_t> serverPort;
	boost::optional<uint32_t> baudRate;
	boost::optional<uint8_t> operatingMode;
	boost::optional<uint16_t> inactivity;
};

class Wiznet {
public:

	const static uint16_t LOCAL_PORT = 5001;
	const static uint16_t REMOTE_PORT = 1460;

	const static uint8_t CLIENT_MODE = 0x00;
	const static uint8_t MIXED_MODE = 0x01;
	const static uint8_t SERVER_MODE = 0x02;

	const static uint8_t BAUD_4800 = 0xE8;
	const static uint8_t BAUD_9600 = 0xF4;
	const static uint8_t BAUD_19200 = 0xFA;
	const static uint8_t BAUD_38400 = 0xFD;
	const static uint8_t BAUD_57600 = 0xFE;
	const static uint8_t BAUD_115200 = 0xFF;
	const static uint8_t BAUD_230400 = 0xBB;

	static std::vector<WiznetDevice> discoveryAll(boost::asio::io_service &ios);
	static bool assignConfig(boost::asio::io_service &ios, WiznetConfig &config);
	static bool resetDevice(boost::asio::io_service &ios, WiznetConfig &config);

private:

	static std::vector<WiznetPacket> findAllDevices(boost::asio::io_service &ios);

	static WiznetDevice packetToObject(const WiznetPacket &packet);	

};
