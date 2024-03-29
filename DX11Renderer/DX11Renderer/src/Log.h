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
			enum class LogMessageType : u8
			{
				Error,
				Warning,
				Info
			};
		public:
			LogMessage(const LogMessageType type, const std::string message)
				: type(type), message(message)
			{}
			void DrawImGui();
		private:
			const LogMessageType type;
			const std::string message;
		};

	public:
		Log();
		virtual ~Log();
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
		std::vector<LogMessage> m_messages;
	};
}