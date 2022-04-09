#pragma once
#include "CommonHeader.h"
#include <type_traits>
#include <unordered_map>
#include "CodexElement.h"

namespace gfx
{
	class GraphicsDevice;
	class CodexElement;

	class Codex
	{
	public:
		///
		/// Return codex type, creating it if needed
		///
		template<class T,typename...Params>
		static std::shared_ptr<T> Resolve(const GraphicsDevice& gfx, std::string id, Params&&...p )
		{
			static_assert( std::is_base_of<CodexElement,T>::value, "Can only resolve classes derived from CodexElement" );
			return std::move(GetInstance().InternalResolve<T>( gfx, id, std::forward<Params>( p )... ));
		}

		static void ReleaseAll()
		{
			GetInstance().InternalReleaseAll();
		}

	private:
		template<class T,typename...Params>
		std::shared_ptr<T> InternalResolve(const GraphicsDevice& gfx, std::string id, Params&&...p )
		{
			const auto i = m_binds.find( id );
			if( i == m_binds.end() )
			{
				// Create CodexElement
				auto bind = std::make_shared<T>( gfx,std::forward<Params>( p )... );
				m_binds[id] = bind;
				return bind;
			}
			else
			{
				return std::static_pointer_cast<T>( i->second );
			}
		}

		void InternalReleaseAll()
		{
			for (std::pair<std::string, std::shared_ptr<CodexElement>> bind : m_binds)
			{
				bind.second->Release();
			}
		}

		static Codex& GetInstance()
		{
			static Codex codex;
			return codex;
		}

	private:
		std::unordered_map<std::string, std::shared_ptr<CodexElement>> m_binds;
	};
}