#pragma once

#include "pebble/app/StorageEngine.h"

#include <string>

using namespace pebble::app;


		class CLI
		{
		public:
			CLI(StorageEngine& engine);
			void run();

		private:
			StorageEngine& m_Engine;

			void printHelp() const;
			void handleCommand(const std::string& line);
		};
	
