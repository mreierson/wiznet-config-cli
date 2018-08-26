#include "wiznet.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/format.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <sstream>

const uint8_t Wiznet::CLIENT_MODE;
const uint8_t Wiznet::MIXED_MODE;
const uint8_t Wiznet::SERVER_MODE;

std::vector<WiznetDevice> Wiznet::discoveryAll(boost::asio::io_service &ios)
{
	std::vector<WiznetPacket> packets = findAllDevices(ios);

	std::vector<WiznetDevice> devices;
	for (const auto &packet : packets) {
		devices.push_back(packetToObject(packet));
	}

	return devices;
}

bool Wiznet::assignConfig(boost::asio::io_service &ios, WiznetConfig &config)
{
	std::vector<WiznetPacket> packets = findAllDevices(ios);

	for (auto &packet : packets) {
		WiznetDevice device = packetToObject(packet);
		if (device.macAddress == config.macAddress) {

			if (config.operatingMode) {
				packet.opMode = *config.operatingMode;
			}

			if (config.ipAddress) {
				std::vector<std::string> tokens;
				boost::split(tokens, *config.ipAddress, boost::is_any_of("."));
				packet.ipAddress[0] = std::stoul(tokens[0], nullptr, 10);
				packet.ipAddress[1] = std::stoul(tokens[1], nullptr, 10);
				packet.ipAddress[2] = std::stoul(tokens[2], nullptr, 10);
				packet.ipAddress[3] = std::stoul(tokens[3], nullptr, 10);
			}

			if (config.netmask) {
				std::vector<std::string> tokens;
				boost::split(tokens, *config.netmask, boost::is_any_of("."));
				packet.subnetMask[0] = std::stoul(tokens[0], nullptr, 10);
				packet.subnetMask[1] = std::stoul(tokens[1], nullptr, 10);
				packet.subnetMask[2] = std::stoul(tokens[2], nullptr, 10);
				packet.subnetMask[3] = std::stoul(tokens[3], nullptr, 10);
			}

			if (config.gateway) {
				std::vector<std::string> tokens;
				boost::split(tokens, *config.gateway, boost::is_any_of("."));
				packet.gatewayAddress[0] = std::stoul(tokens[0], nullptr, 10);
				packet.gatewayAddress[1] = std::stoul(tokens[1], nullptr, 10);
				packet.gatewayAddress[2] = std::stoul(tokens[2], nullptr, 10);
				packet.gatewayAddress[3] = std::stoul(tokens[3], nullptr, 10);
			}

			if (config.dhcpFlag) {
				packet.dhcp = *config.dhcpFlag ? 0x01 : 0x00;
			}

			if (config.serverPort) {
				packet.serverPort[0] = (*config.serverPort >> 8) & 0x00FF;
				packet.serverPort[1] = (*config.serverPort) & 0x00FF;
			}

			if (config.baudRate) {
				switch (*config.baudRate) {
				case 4800:
					packet.baud = BAUD_4800;
					break;
				case 9600:
					packet.baud = BAUD_9600;
					break;
				case 19200:
					packet.baud = BAUD_19200;
					break;
				case 38400:
					packet.baud = BAUD_38400;
					break;
				case 57600:
					packet.baud = BAUD_57600;
					break;
				case 115200:
					packet.baud = BAUD_115200;
					break;
				case 230400:
					packet.baud = BAUD_230400;
					break;
				}
			}

			if (config.inactivity) {
				packet.inactivity[0] = (*config.inactivity >> 8) & 0x00FF;
				packet.inactivity[1] = (*config.inactivity) & 0x00FF;
			}

			std::vector<uint8_t> sendBuffer;
			sendBuffer.push_back('S');
			sendBuffer.push_back('E');
			sendBuffer.push_back('T');
			sendBuffer.push_back('T');

			uint8_t *packetPtr = (uint8_t*) &packet;
			for (size_t i = 0; i < sizeof(WiznetPacket); ++i) {
				sendBuffer.push_back(packetPtr[i]);
			}

			boost::system::error_code ec;
			boost::asio::ip::udp::socket socket(ios);

			socket.open(boost::asio::ip::udp::v4(), ec);
			if (!ec) {
				socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
				socket.set_option(boost::asio::socket_base::broadcast(true));

				socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), LOCAL_PORT));

				socket.send_to(boost::asio::buffer(sendBuffer),
							   boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::broadcast(), REMOTE_PORT));

				boost::asio::deadline_timer timer(ios);
				timer.expires_from_now(boost::posix_time::seconds(5));
				timer.async_wait([&](const boost::system::error_code &ec)
								 {
									 boost::system::error_code ec2;
									 socket.cancel(ec2);
								 });

				boost::asio::ip::udp::endpoint ep;
				boost::array<unsigned char, 1024> recvBuffer;
				std::future<size_t> result = socket.async_receive_from(boost::asio::buffer(recvBuffer), ep, boost::asio::use_future);

				size_t len = 0;
				try {
					len = result.get();
					timer.cancel(ec);
				} catch (...) {
				}

				socket.close(ec);

				if ((len >= 4) &&
					(recvBuffer[0] == 'S') &&
					(recvBuffer[1] == 'E') &&
					(recvBuffer[2] == 'T') &&
					(recvBuffer[3] == 'C')) {

					return true;
				}
			}
		}
	}
	return false;
}

bool Wiznet::resetDevice(boost::asio::io_service &ios, WiznetConfig &config)
{
	return assignConfig(ios, config);
}

std::vector<WiznetPacket> Wiznet::findAllDevices(boost::asio::io_service &ios)
{
	std::vector<WiznetPacket> devices;

	boost::system::error_code ec;
	boost::asio::ip::udp::socket socket(ios);

	socket.open(boost::asio::ip::udp::v4(), ec);
	if (!ec) {
		socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		socket.set_option(boost::asio::socket_base::broadcast(true));

		socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), LOCAL_PORT));

		socket.send_to(boost::asio::buffer("FIND"), 
			boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::broadcast(), REMOTE_PORT));

		bool bCompletelyReceived = false;

		while (!bCompletelyReceived) {

			boost::asio::deadline_timer timer(ios);
			timer.expires_from_now(boost::posix_time::seconds(30));
			timer.async_wait([&](const boost::system::error_code &ec)
							 {
								 if (ec) return;
								 boost::system::error_code ec2;
								 socket.cancel(ec2);
								 bCompletelyReceived = true;
							 });

			boost::asio::ip::udp::endpoint ep;
			std::vector<unsigned char> buffer(1024);
			std::future<size_t> result = socket.async_receive_from(boost::asio::buffer(buffer), ep, boost::asio::use_future);

			size_t len = 0;
			try {
				len = result.get();
				timer.cancel(ec);
			} catch (...) {
			}

			if (len > 0) {

				size_t i = 0;
				bool parsed = false;
				while (!parsed && (i < len)) {
					if ((buffer[i++] == 'I') && (buffer[i++] == 'M') && (buffer[i++] == 'I') && (buffer[i++] == 'N')) {
						WiznetPacket packet;
						unsigned char *packetPtr = (unsigned char*) &packet;

						std::copy(buffer.begin() + i, buffer.begin() + i + sizeof(WiznetPacket), packetPtr);

						i += sizeof(WiznetPacket);

						devices.push_back(packet);

					} else {
						parsed = true;
					}
				}
			} else {
				bCompletelyReceived = true;
			}
		}

	} else {
		std::cerr << "Error: " << ec.message() << std::endl;
	}

	return devices;
}

WiznetDevice Wiznet::packetToObject(const WiznetPacket &packet)
{
	WiznetDevice device;

	std::stringstream macAddress;
	for (size_t j = 0; j < 6; ++j) {
		macAddress << boost::format("%02X") % ((int) packet.macAddress[j]);
	}
	device.macAddress = macAddress.str();

	switch (packet.opMode) {
	case CLIENT_MODE:
		device.operatingMode = (boost::format("CLIENT(%d)") % ((int) packet.opMode)).str();
		break;
	case MIXED_MODE:
		device.operatingMode = (boost::format("MIXED(%d)") % ((int) packet.opMode)).str();
		break;
	case SERVER_MODE:
		device.operatingMode = (boost::format("SERVER(%d)") % ((int) packet.opMode)).str();
		break;
	}

	std::stringstream ipAddress, netmask, gateway;
	for (size_t j = 0; j < 4; ++j) {
		if (j > 0) {
			ipAddress << ".";
			netmask << ".";
			gateway << ".";
		}
		ipAddress << (int) packet.ipAddress[j];
		netmask << (int) packet.subnetMask[j];
		gateway << (int) packet.gatewayAddress[j];
	}
	device.ipAddress = ipAddress.str();
	device.netmask = netmask.str();
	device.gateway = gateway.str();

	device.serverPort = (packet.serverPort[0] << 8) + packet.serverPort[1];

	switch (packet.baud) {
	case BAUD_4800:
		device.baudRate = 4800;
		break;
	case BAUD_9600:
		device.baudRate = 9600;
		break;
	case BAUD_19200:
		device.baudRate = 19200;
		break;
	case BAUD_38400:
		device.baudRate = 38400;
		break;
	case BAUD_57600:
		device.baudRate = 57600;
		break;
	case BAUD_115200:
		device.baudRate = 115200;
		break;
	case BAUD_230400:
		device.baudRate = 230400;
		break;
	}

	device.dataBits = packet.dataBits;
	device.stopBits = packet.stopBits;

	switch (packet.parity) {
	case 0x00:
		device.parity = "N";
		break;
	case 0x01:
		device.parity = "O";
		break;
	case 0x02:
		device.parity = "E";
		break;
	}

	device.inactivityTimeout = (packet.inactivity[0] << 8) + packet.inactivity[1];
	device.dhcpFlag = (packet.dhcp == 0x01);
	device.clientConnected = (packet.connStatus == 0x01);

	return device;
}
