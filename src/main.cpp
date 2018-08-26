#include <stdio.h>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/bind.hpp>
#include <boost/program_options.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread/thread.hpp>

#include "wiznet.hpp"

boost::asio::io_service ios;

void shutdownHandler(const boost::system::error_code &ec, int signal)
{
	ios.stop();
}

int main(int argc, char* argv[])
{
	try {
		boost::asio::signal_set signals(ios, SIGINT, SIGTERM, SIGSEGV);
		signals.async_wait(shutdownHandler);

		namespace po = boost::program_options;

		po::options_description options_desc("wiznet-config-cli");
		options_desc.add_options()
			("discovery,d", "Network discovery of WIZnet devices")
			("ipconfig", po::value<std::string>(), "MAC address of device to configure")
			("static-ip", po::value<std::string>(), "Assign IP address for static configuration")
			("static-netmask", po::value<std::string>()->default_value("255.255.255.0"), "Assign netmask for static configuration")
			("static-gateway", po::value<std::string>(), "Assign gateway for static configuration")
			("dhcp", "Assign DHCP configuration")
			("port", po::value<int>(), "Assign WIZnet server port")
			("baud,b", po::value<int>(), "Assign baud rate (4800,9600,19200,38400,57600,115200,230400)")
			("mode-client", "Set the operation mode to CLIENT")
			("mode-server", "Set the operation mode to SERVER")
			("mode-mixed", "Set the operation mode to MIXED")
			("inactivity", po::value<int>(), "Set the inactivity timeout in seconds (0-65535, 0 to disable)")
			("reset", "Reset the WIZNet device")
			("help,h", "Display this help message");

		po::variables_map vm;

		try {
			po::store(po::parse_command_line(argc, argv, options_desc), vm);
			po::notify(vm);
		} catch (std::exception &ex) {
			std::cerr << ex.what() << std::endl;
			return 1;
		}

		if (vm.count("help")) {
			std::cout << options_desc << std::endl;
			return 0;
		}

		boost::thread_group tg;
		for (size_t i = 0; i < 2; ++i) {
			tg.create_thread(boost::bind(&boost::asio::io_service::run, &ios));
		}

		if (vm.count("discovery")) {
			std::cout << "Scanning network for devices ...." << std::endl;

			std::vector<WiznetDevice> devices = Wiznet::discoveryAll(ios);

			if (devices.size() == 0) {
				std::cout << "No devices found" << std::endl;
			}

			for (size_t i = 0; i < devices.size(); ++i) {
				if (i == 0) {
					printf("    %-15s %-15s %-15s %-15s %-10s %-5s %-15s %-12s %-12s %-9s\n",
						   "MAC Address",
						   "IP Address",
						   "Subnet",
						   "Gateway",
						   "Port",
						   "DHCP",
						   "Serial Config.",
						   "Oper. Mode",
						   "Inactivity",
						   "HasClient");
				}

				printf("%2d. %-15s %-15s %-15s %-15s %-10d %-5s %-6d %d%s%-6d %-12s %-12d %s\n",
					   (i+1),
					   devices[i].macAddress.c_str(),
					   devices[i].ipAddress.c_str(),
					   devices[i].netmask.c_str(),
					   devices[i].gateway.c_str(),
					   devices[i].serverPort,
					   (devices[i].dhcpFlag ? "Y" : "N"),
					   devices[i].baudRate,
					   devices[i].dataBits,
					   devices[i].parity.c_str(),
					   devices[i].stopBits,
					   devices[i].operatingMode.c_str(),
					   devices[i].inactivityTimeout,
					   (devices[i].clientConnected ? "Y" : "N"));
			}

			goto finished;
		}

		if (vm.count("ipconfig")) {

			std::string macAddress = vm["ipconfig"].as<std::string>();
			if (macAddress.length() != 12) {
				std::cerr << "Error: MAC address needs 12 characters" << std::endl;
				goto finished;
			}

			WiznetConfig config;
			config.macAddress = macAddress;

			if (vm.count("reset")) {

				if (Wiznet::resetDevice(ios, config)) {
					std::cout << "Device reset was successful" << std::endl;
				} else {
					std::cout << "Device reset was NOT successful" << std::endl;
				}

				goto finished;
			}


			if (vm.count("port")) {
				config.serverPort = vm["port"].as<int>();
			}

			if (vm.count("baud")) {
				config.baudRate = vm["baud"].as<int>();
			}

			if (vm.count("mode-server")) {
				config.operatingMode = Wiznet::SERVER_MODE;
			}

			if (vm.count("mode-client")) {
				config.operatingMode = Wiznet::CLIENT_MODE;
			}

			if (vm.count("mode-mixed")) {
				config.operatingMode = Wiznet::MIXED_MODE;
			}

			if (vm.count("dhcp")) {
				config.dhcpFlag = true;
			} else {
				config.dhcpFlag = false;
			}

			if (vm.count("static-ip")) {
				config.ipAddress = vm["static-ip"].as<std::string>();
				config.dhcpFlag = false;
			}

			if (vm.count("static-netmask")) {
				config.netmask = vm["static-netmask"].as<std::string>();
			}

			if (vm.count("static-gateway")) {
				config.gateway = vm["static-gateway"].as<std::string>();
			}

			if (vm.count("inactivity")) {
				config.inactivity = vm["inactivity"].as<int>();
			}

			if (Wiznet::assignConfig(ios, config)) {
				std::cout << "Device configuration was successful" << std::endl;
			} else {
				std::cout << "Device configuration was NOT successful" << std::endl;
			}
		}

	finished:

		ios.stop();
		tg.join_all();

	} catch (std::exception ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
