#pragma once
#include <string.h>

// The buffer object is a combination of a raw byte buffer with a LayoutElement
// tree structure which acts as an view/interpretation/overlay for those bytes
// operator [] indexes into the root Struct, returning a Ref shell that can be
// used to further index if struct/array, returning further Ref shells, or used
// to access the data stored in the buffer if a Leaf element type
class Buffer
{
public:
	// various resources can be used to construct a Buffer
	Buffer(RawLayout&& lay);
	Buffer(const CookedLayout& lay);
	Buffer(CookedLayout&& lay);
	Buffer(const Buffer&) noexcept;
	// have to be careful with this one...
	// the buffer that has once been pilfered must not be used :x
	Buffer(Buffer&&) noexcept;
	// how you begin indexing into buffer (root is always Struct)
	ElementRef operator[](const std::string& key);
	// if Buffer is const, you only get to index into the buffer with a read-only proxy
	ConstElementRef operator[](const std::string& key) const;
	// get the raw bytes
	const char* GetData() const noexcept;
	// size of the raw byte buffer
	size_t GetSizeInBytes() const noexcept;
	const LayoutElement& GetRootLayoutElement() const noexcept;
	// copy bytes from another buffer (layouts must match)
	void CopyFrom(const Buffer&);
	// return another sptr to the layout root
	std::shared_ptr<LayoutElement> ShareLayoutRoot() const noexcept;
private:
	std::shared_ptr<LayoutElement> pLayoutRoot;
	std::vector<char> bytes;
};