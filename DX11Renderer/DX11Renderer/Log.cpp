#include "Log.h"
#include "imgui/imgui.h"

Log::Log()
{
}

void Log::Info(const char* message)
{
	messages.push_back(LogMessage(LogMessage::LogMessageType::Info, std::string(message)));
}

void Log::Info(const std::string message)
{
	messages.push_back(LogMessage(LogMessage::LogMessageType::Info, message));
}

void Log::Warning(const char * message)
{
	messages.push_back(LogMessage(LogMessage::LogMessageType::Warning, std::string(message)));
}

void Log::Warning(const std::string message)
{
	messages.push_back(LogMessage(LogMessage::LogMessageType::Warning, message));
}

void Log::Error(const char * message)
{
	messages.push_back(LogMessage(LogMessage::LogMessageType::Error, std::string(message)));
}

void Log::Error(const std::string message)
{
	messages.push_back(LogMessage(LogMessage::LogMessageType::Error, message));
}

void Log::DrawImguiControlWindow()
{
	if (ImGui::Begin("Log"))
	{
		ImGui::Text("Messages:");

		if (ImGui::Button("Clear"))
		{
			Clear();
		}

		ImGui::BeginChild("Scrolling");
		for (size_t i = 0; i < messages.size(); ++i)
		{
			messages[i].DrawImGui();
		}
		
		ImGui::EndChild();
	}
	ImGui::End();
}

void Log::Clear()
{
}

void Log::LogMessage::DrawImGui()
{
	ImVec4 textColor;
	switch (type)
	{
	case LogMessage::LogMessageType::Info:
		textColor = ImVec4(0.5, 0.5, 0.9, 0.9);
		break;
	case LogMessage::LogMessageType::Warning:
		textColor = ImVec4(1.0, 1.0, 0.3, 0.9);
		break;
	case LogMessage::LogMessageType::Error:
		textColor = ImVec4(1.0, 0.2, 0.2, 0.9);
		break;
	default:
		textColor = ImVec4(0.9, 0.9, 0.9, 0.9);
		break;
	}
	ImGui::TextColored(textColor, message.c_str());
}
