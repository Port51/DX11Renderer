#pragma once
#include <type_traits>
#include <memory>
#include <unordered_map>
#include "CodexElement.h"

class Graphics;
class CodexElement;

namespace Bind
{
	class Codex
	{
	public:
		///
		/// Return codex type, creating it if needed
		///
		template<class T,typename...Params>
		static std::shared_ptr<T> Resolve( Graphics& gfx, std::string id, Params&&...p )
		{
			static_assert( std::is_base_of<CodexElement,T>::value, "Can only resolve classes derived from CodexElement" );
			return GetInstance().InternalResolve<T>( gfx, id, std::forward<Params>( p )... );
		}
	private:
		template<class T,typename...Params>
		std::shared_ptr<T> InternalResolve( Graphics& gfx, std::string id, Params&&...p )
		{
			const auto i = binds.find( id );
			if( i == binds.end() )
			{
				// Create CodexElement
				auto bind = std::make_shared<T>( gfx,std::forward<Params>( p )... );
				binds[id] = bind;
				return bind;
			}
			else
			{
				return std::static_pointer_cast<T>( i->second );
			}
		}
		static Codex& GetInstance()
		{
			static Codex codex;
			return codex;
		}
	private:
		std::unordered_map<std::string,std::shared_ptr<class CodexElement>> binds;
	};
}