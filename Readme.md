# Debug Utility for Windows Applications

A lightweight, stream-style debug logging utility designed for Windows applications, providing functionality similar to Qt's qDebug(). This utility enables easy debug output to the Visual Studio Output window with zero overhead in release builds.

## Features

- Stream-style interface similar to std::cout
- UTF-8 and UTF-16 string support
- All standard stream manipulators support
- Automatic type conversion
- Zero overhead in release builds
- Thread-safe output
- RAII-compliant resource management

## Installation

Simply include the `DebugUtil.h` header in your project:

```cpp
#include "DebugUtil.h"
```

## Basic Usage

```cpp
// Basic string output
Debug() << L"Hello World";

// Multiple values
Debug() << L"Count: " << 42 << L" Active: " << true;

// Using manipulators
Debug() << std::hex << L"Hex value: " << 255;  // Outputs: Hex value: ff
Debug() << std::setw(4) << std::setfill('0') << 42;  // Outputs: 0042

// Narrow string support
Debug() << "Narrow string" << "UTF-8 string";
```

## ⚠️ Important: String Usage in Windows

### Prefer Wide Strings
Windows internally uses UCS-2/UTF-16 encoding. To avoid potential character encoding issues and conversion overhead, it's **strongly recommended** to use wide strings whenever possible:

```cpp
// Preferred: Use wide strings
Debug() << L"Wide string is preferred";  // Best practice
Debug() << std::wstring(L"Also good");   // Best practice

// Only use narrow strings for ASCII content
Debug() << "ASCII string";  // OK for ASCII only
Debug() << std::string("ASCII only");  // OK for ASCII only
```

### Character Encoding for Narrow Strings

When narrow strings must be used, this utility uses UTF-8 encoding by default. If you notice incorrect character display in the debug output (especially with non-ASCII characters), you need to define `DEBUG_CODE_PAGE` before including the header. The simplest solution is usually:

```cpp
#define DEBUG_CODE_PAGE CP_ACP  // Use system default Windows ANSI code page
#include "DebugUtil.h"
```

Common code page options:
- `CP_UTF8` (65001) - UTF-8 encoding (default)
- `CP_ACP` (0) - System default Windows ANSI code page
- `CP_THREAD_ACP` - Current thread's ANSI code page
- Other code pages can be found in the [Windows Code Page Identifiers documentation](https://docs.microsoft.com/en-us/windows/win32/intl/code-page-identifiers)

## Features
- Stream-style interface similar to std::cout
- UTF-8 and UTF-16 string support
- All standard stream manipulators support
- Automatic type conversion
- Zero overhead in release builds
- Thread-safe output
- RAII-compliant resource management


## API Reference

### Class: DebugStream

The main class providing debug stream functionality.

#### Constructors

```cpp
explicit DebugStream(bool autoFlushEnabled = true)
```
- Creates a new DebugStream instance
- Parameters:
  - `autoFlushEnabled`: Controls whether output is automatically flushed (default: true)
- Example:
```cpp
DebugStream debug(false);  // Create with auto-flush disabled
```

#### Public Methods

##### `void Flush()`
- Manually flushes the current contents of the buffer to the debug output
- Usage:
```cpp
DebugStream debug(false);  // Auto-flush disabled
debug << "This won't be visible yet";
debug.Flush();  // Now it's visible
```

#### Stream Operators

##### `template<typename T> DebugStream& operator<<(const T& value)`
- Handles output of any type that can be streamed to std::wostream
- Parameters:
  - `value`: The value to output
- Returns: Reference to the DebugStream for chaining
- Example:
```cpp
Debug() << 42 << 3.14 << true;
```

##### `DebugStream& operator<<(const char* value)`
- Specialized handling for C-style strings (UTF-8)
- Parameters:
  - `value`: UTF-8 encoded string
- Returns: Reference to the DebugStream for chaining
- Example:
```cpp
Debug() << "UTF-8 string";
```

##### `DebugStream& operator<<(const std::string& value)`
- Specialized handling for std::string (UTF-8)
- Parameters:
  - `value`: UTF-8 encoded string
- Returns: Reference to the DebugStream for chaining
- Example:
```cpp
std::string str = "Hello";
Debug() << str;
```

##### `DebugStream& operator<<(const wchar_t* value)`
- Direct handling of wide character strings
- Parameters:
  - `value`: Wide character string
- Returns: Reference to the DebugStream for chaining
- Example:
```cpp
Debug() << L"Wide string";
```

##### `DebugStream& operator<<(const std::wstring& value)`
- Direct handling of std::wstring
- Parameters:
  - `value`: Wide string
- Returns: Reference to the DebugStream for chaining
- Example:
```cpp
std::wstring wstr = L"Hello";
Debug() << wstr;
```

### Global Functions

##### `DebugStream Debug()`
- Factory function to create a DebugStream instance
- Returns: A new DebugStream object
- Example:
```cpp
Debug() << "Hello World";
```

## Advanced Usage

### Character Encoding and Troubleshooting

If you're experiencing issues with character display in the debug output, especially with non-ASCII characters, you can change the code page used for string conversion:

```cpp
// Before including DebugUtil.h, define DEBUG_CODE_PAGE:

// Use system default Windows ANSI code page (recommended for most Windows applications)
#define DEBUG_CODE_PAGE CP_ACP

// Or use specific code pages:
#define DEBUG_CODE_PAGE 932    // Japanese Shift-JIS
#define DEBUG_CODE_PAGE 949    // Korean
#define DEBUG_CODE_PAGE 936    // Simplified Chinese (GB2312)
#define DEBUG_CODE_PAGE 1252   // Western European (Latin 1)
```

Example with different code pages:
```cpp
// In a Japanese Windows system:
#define DEBUG_CODE_PAGE CP_ACP  // or 936 for GB2312
#include "DebugUtil.h"

Debug() << "中文测试";  // Will display correctly
```

### Using Stream Manipulators

```cpp
// Numerical formatting
Debug() << std::hex << "Hex: " << 255 << std::dec << " Dec: " << 255;

// Width and fill
Debug() << std::setw(5) << std::setfill('*') << 42;

// Floating point precision
Debug() << std::fixed << std::setprecision(2) << 3.14159;
```

### Custom Types

To use custom types with DebugStream, simply provide an appropriate stream operator:

```cpp
struct Point {
    int x, y;
    friend std::wostream& operator<<(std::wostream& os, const Point& p) {
        return os << "(" << p.x << "," << p.y << ")";
    }
};

// Now you can use Point with Debug()
Point p{1, 2};
Debug() << "Point: " << p;  // Outputs: Point: (1,2)
```

### Release Build Behavior

In release builds (when `_DEBUG` is not defined), all debug output code is completely removed by the compiler, resulting in zero runtime overhead. This means you can freely use Debug() throughout your code without worrying about performance in release builds.

## Thread Safety

The DebugStream class is designed to be thread-safe for individual stream operations. However, if you need to output multiple values as a single atomic operation, you should use a single stream operation:

```cpp
// This is thread-safe (single operation):
Debug() << "Value1: " << value1 << " Value2: " << value2 << std::endl;

// This might interleave with other thread output:
Debug() << "Value1: " << value1 << std::endl;
Debug() << "Value2: " << value2 << std::endl;
```

## Best Practices

1. **Always prefer wide strings (L"..." or std::wstring) over narrow strings**
   - Windows internally uses UCS-2/UTF-16
   - Avoids character encoding issues
   - Better performance (no conversion needed)
   ```cpp
   // Good
   Debug() << L"Wide string message";
   Debug() << std::wstring(L"Status: ") << L"OK";

   // Avoid for non-ASCII content
   Debug() << "Narrow string";  // Only use for ASCII
   ```

2. Single Stream Operations
   - Prefer single stream operations for related output to ensure atomicity
   ```cpp
   // Good - atomic output
   Debug() << L"Value1: " << value1 << L" Value2: " << value2 << std::endl;

   // Avoid - may interleave with other thread output
   Debug() << L"Value1: " << value1 << std::endl;
   Debug() << L"Value2: " << value2 << std::endl;
   ```

3. String Encoding
   - Use wide strings for any non-ASCII content
   - Only use narrow strings for ASCII content
   - If narrow strings must be used, consider setting appropriate DEBUG_CODE_PAGE

4. Performance Considerations
   - Consider disabling auto-flush for performance-critical debug sections
   - Remember that all Debug() calls are removed in release builds
   - Use wide strings to avoid conversion overhead

5. Formatting
   - Use stream manipulators for formatting when needed
   - Maintain consistent string type usage within a single stream operation

## License

MIT License
