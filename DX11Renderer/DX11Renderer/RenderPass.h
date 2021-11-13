#pragma once
#include "Graphics.h"
#include "RenderJob.h"
#include <vector>

namespace Rgph
{
	class Sink;
	class Source;

	///
	/// Wrapper containing jobs
	///
	class RenderPass
	{
	public:
		RenderPass(std::string name);
		virtual ~RenderPass();
		void EnqueueJob(RenderJob job);
		virtual void Execute(Graphics& gfx) const = 0;
		virtual void Reset();
		const std::string& GetName() const;
		const std::vector<std::unique_ptr<Sink>>& GetSinks() const;
		Source& GetSource(const std::string& registeredName) const;
		Sink& GetSink(const std::string& registeredName) const;
		void SetSinkLinkage(const std::string& registeredName, const std::string& target);
		virtual void Finalize();
	protected:
		void RegisterSink(std::unique_ptr<Sink> sink);
		void RegisterSource(std::unique_ptr<Source> source);
	private:
		std::vector<RenderJob> jobs; // will be replaced by render graph
	private:
		std::vector<std::unique_ptr<Sink>> sinks;
		std::vector<std::unique_ptr<Source>> sources;
		std::string name;
	};
}