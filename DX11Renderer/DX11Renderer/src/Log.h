#pragma once
#include <string>
#include <vector>

namespace gfx
{
	class Log
	{
		class LogMessage
		{
		public:
			enum LogMessageType
			{
				Error,
				Warning,
				Info
			};
		public:
			LogMessage(LogMessageType type, std::string message)
				: type(type), message(message)
			{}
			void DrawImGui();
		private:
			LogMessageType type;
			std::string message;
		};

	public:
		Log();
	public:
		void Info(const char* message);
		void Info(const std::string message);
		void Warning(const char* message);
		void Warning(const std::string message);
		void Error(const char* message);
		void Error(const std::string message);
		void DrawImguiControlWindow();
		void Clear();
	private:
		std::vector<LogMessage> messages;
	};
}