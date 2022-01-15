#pragma once

// Macros for calling exceptions
#define CWND_EXCEPT( hr ) Window::HresultException( __LINE__,__FILE__,(hr) )
#define CWND_LAST_EXCEPT() Window::HresultException( __LINE__,__FILE__,GetLastError() )
#define CWND_NOGFX_EXCEPT() Window::NoGfxException( __LINE__,__FILE__ )