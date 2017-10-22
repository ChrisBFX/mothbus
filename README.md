# mothbus, a c++11 library for the modbus protocol

[![Build Status](https://travis-ci.org/ChrisBFX/mothbus.svg?branch=master)](https://travis-ci.org/ChrisBFX/mothbus)

## Introduction

The goal of the mothbus library is to provide a complete and simple implementation of the 
modbus protocol as defined on modbus.org. It provides role agnostic intefaces for modbus-tcp 
and modbus-rtu, which means you can use the same code for server (called slaves in modbus context)
and clients (called master), as well a simple client interface.
The design goals (which set it apart of other modbus implementations) are:

* **Scalability.** Mothbus scales from tiny microcontrollers to the cloud.

* **Network layer agnostic.**  Mothbus does not implement the network layer, which allows it to 
be ported to almost everything. As it is heavily influenced by boost::asio it works best with it.

* **Async api.** A simple, based on boost::asio, asynchronous api (if your network layer support it).

* **Low memory footprint** Dynamic memory allocations are kept to a minimum.

## Status

The library is currently in a proof-of-concept state and while it maybe already works, there are many
unaddressed issues and internal inconsistencies.

## Requirements

* C++11, propably some C++14 features required
* gsl (tested with https://github.com/Microsoft/GSL)
* boost (asio, system (program_options for examples))
* google-test for unit tests


## To Build

Mothbus is a header only library, so simple add the include path to your project.
For tests and examples you can use cmake.

## Example

Reading a register from a modbus tcp device:
```C++
#include <iostream>
#include <boost/asio.hpp>
#include <mothbus/adu/tcp.h>

int main(int argc, char** argv)
{	
	using boost::asio::ip::tcp;
	std::string host = "localhost";	
	boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);
	tcp::socket socket(io_service);
	boost::asio::connect(socket, esolver.resolve(tcp::resolver::query{host, "502"}));

	mothbus::tcp_slave<tcp::socket> client(socket);
	std::array<mothbus::byte, 2> singleRegister;
	client.read_registers(slave, register_address, singleRegister);
	
	uint16_t value = (gsl::to_integer<uin16_t>(singleRegister[0]) << 8) + gsl::to_integer<uin16_t>(singleRegister[0]);
	std::cout << value;	
	return 0;
}
```