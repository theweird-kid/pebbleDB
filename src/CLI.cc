#include "pebble/app/CLI.h"

#include <iostream>
#include <sstream>

using namespace pebble::app;

CLI::CLI(StorageEngine& engine)
	: m_Engine(engine)
{ }

void CLI::printHelp() const
{
	std::cout << "Avaliable commands:\n"
		<< "\tcreate <collection>\n"
		<< "\tinsert <collection> <key> <value>\n"
		<< "\tget <collection> <key>\n"
		<< "\tremove <collection> <key>\n"
		<< "\thelp\n"
		<< "\texit\n";
}

void CLI::handleCommand(const std::string& line)
{
	std::istringstream iss(line);
	std::string cmd;
	iss >> cmd;

	if (cmd == "help")
	{
		printHelp();
	}
	else if (cmd == "create")
	{
		std::string collection;
		iss >> collection;
		if (collection.empty()) {
			std::cout << "Usage: create <collection>\n";
			return;
		}
		if (!m_Engine.createCollection(collection)) {
			std::cout << "Collection already exists.\n";
		}
		else {
			std::cout << "Created collection: " << collection << "\n";
		}
	}
	else if (cmd == "insert")
	{
		std::string collection, value;
		int key;
		iss >> collection >> key >> value;
		if (collection.empty() || value.empty()) {
			std::cout << "Usage: insert <collection> <key> <value>\n";
			return;
		}
		m_Engine.insert(collection, key, value);
		std::cout << "Inserted (" << key << ", " << value << ") into " << collection << std::endl;
	}
	else if (cmd == "get")
	{
		std::string collection;
		int key;
		iss >> collection >> key;
		if (collection.empty()) {
			std::cout << "Usage: get <collection> <key>\n";
			return;
		}
		
		auto val = m_Engine.get(collection, key);
		if (val) {
			std::cout << key << " => " << *val << "\n";
		}
		else {
			std::cout << key << " => NOT FOUND\n";
		}
	}
	else if (cmd == "remove") 
	{
		std::string collection;
		int key;
		iss >> collection >> key;
		if (collection.empty()) {
			std::cout << "Usage: remove <collection> <key>\n";
			return;
		}
		if (m_Engine.remove(collection, key)) {
			std::cout << "Removed key " << key << " from " << collection << "\n";
		}
		else {
			std::cout << "Key " << key << " not found.\n";
		}
	}
	else if (!cmd.empty()) 
	{
		std::cout << "Unknown command: " << cmd << "\n";
		printHelp();
	}
}

void CLI::run() {
	std::cout << "Pebble KVStore CLI\n";
	printHelp();

	std::string line;
	while (true) {
		std::cout << ">> ";
		if (!std::getline(std::cin, line)) break;
		if (line == "exit") break;
		handleCommand(line);
	}

	std::cout << "Exiting Pebble CLI.\n";
}