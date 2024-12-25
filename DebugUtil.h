//------------------------------------------------------------------------------
// DebugUtil.h
//------------------------------------------------------------------------------
/**
 * @file DebugUtil.h
 * @brief Provides a stream-style debug logging utility for Windows applications
 * 
 * This header provides a Qt-style debug logging utility that writes to the Visual
 * Studio Output window. It's designed to be easy to use with a familiar stream
 * syntax while having zero overhead in release builds.
 * 
 * Usage Examples:
 * @code
 *     // Basic usage
 *     Debug() << L"Hello World";  // Outputs: Hello World
 * 
 *     // Mixed types
 *     int value = 42;
 *     Debug() << L"The answer is " << value;  // Outputs: The answer is 42
 * 
 *     // Using manipulators
 *     Debug() << std::hex << 255;  // Outputs: ff
 *     Debug() << std::setw(4) << std::setfill('0') << 42;  // Outputs: 0042
 * 
 *     // Narrow string support
 *     Debug() << "Ascii string" << "Ascii string";
 *     
 *     // Multiple values
 *     Debug() << L"Count: " << 1 << L" State: " << true << std::endl;
 * @endcode
 * 
 * Best Practice:
 * It's strongly recommended to use wide strings (wchar_t* i.e. LPWSTR or std::wstring) whenever 
 * possible, especially when working with Windows APIs. Using wide strings avoids 
 * potential character encoding issues as Windows internally uses UCS-2/UTF-16:
 * @code
 *     Debug() << L"Wide string is preferred";  // Recommended
 *     Debug() << "ASCII string";               // OK for ASCII only
 * @endcode
 * 
 * Important Note:
 * When narrow strings must be used, this utility uses UTF-8 encoding (CP_UTF8) by 
 * default for string conversion. If debug output is not displaying correctly 
 * (especially for non-ASCII characters), you should define DEBUG_CODE_PAGE before 
 * including this header. The simplest solution is usually:
 * @code
 * #include <Windows.h>
 * #define DEBUG_CODE_PAGE CP_ACP  // Use system default Windows ANSI code page
 * #include "DebugUtil.h"
 * @endcode
 * 
 * For other code pages, see: https://docs.microsoft.com/en-us/windows/win32/intl/code-page-identifiers
 * 
 * Features:
 * - Stream-style interface similar to std::cout
 * - Supports both narrow (UTF-8) and wide strings
 * - Handles all standard stream manipulators
 * - Automatic type conversion
 * - Zero overhead in release builds (all calls are removed)
 * - Thread-safe output
 * 
 * Note:
 * In release builds (when _DEBUG is not defined), all Debug() calls are compiled
 * out completely, resulting in zero overhead. This means you can freely use Debug()
 * throughout your code without worrying about performance in release builds.
 */

#pragma once
#include <Windows.h>
#include <string>
#include <sstream>
#include <stdexcept>
#include <iomanip>

// Define the default code page for string conversion if not already defined
#ifndef DEBUG_CODE_PAGE
#define DEBUG_CODE_PAGE CP_UTF8
#endif

#ifdef _DEBUG
/**
 * @class DebugStream
 * @brief A class that provides a stream-like interface for writing debug output to the Visual Studio Output window.
 * 
 * DebugStream provides functionality similar to std::cout or std::cerr but outputs
 * to the Windows debug output stream using OutputDebugStringW. It supports both
 * narrow and wide strings, various manipulators, and automatic type conversion.
 */
class DebugStream {
private:
    std::wostringstream buffer;         // Internal buffer for collecting output
#if __cplusplus >= 201103L
    const
#endif
    bool autoFlush;                     // Flag controlling automatic flushing behavior

    /**
     * @brief Converts a narrow string to a wide string and outputs it to a buffer.
     *
     * This function takes a narrow string (default UTF-8 encoding), converts it to a wide string (USC-2),
     * and then outputs the wide string to a buffer. If the input string is null, it throws
     * an invalid_argument exception. If the conversion fails or the string is too long,
     * it throws a runtime_error exception.
     *
     * @param str The narrow C style string to be converted and output.
     * @throws std::invalid_argument If the input string is null.
     * @throws std::runtime_error If the conversion fails or the string is too long.
     * 
     * @remark This function uses the DEBUG_CODE_PAGE macro to determine the code page for conversion,
     * which defaults to CP_UTF8. You can define DEBUG_CODE_PAGE to a different code page if the debug
     * output is not displayed correctly. See @link{https://docs.microsoft.com/en-us/windows/win32/intl/code-page-identifiers}
     */
    inline void ConvertAndOutput(const char* str) {
        if (!str) {
            throw std::invalid_argument("Null string pointer");
        }

        int length = MultiByteToWideChar(DEBUG_CODE_PAGE, 0, str, -1, nullptr, 0);
        if (length <= 0) {
            throw std::runtime_error("Failed to convert string to wide string.");
        }

        if (static_cast<SIZE_T>(length) > (SIZE_MAX / sizeof(WCHAR))) {
            throw std::runtime_error("String too long");
        }

        std::wstring wstr(length, L'\0');
        if (MultiByteToWideChar(DEBUG_CODE_PAGE, 0, str, -1, &wstr[0], length) == 0) {
            throw std::runtime_error("Failed to convert string to wide string.");
        }

        buffer << wstr;
        if (autoFlush) Flush();
    }

public:
    explicit DebugStream(bool autoFlushEnabled = true) : autoFlush(autoFlushEnabled) {}

#if __cplusplus < 201103L
    // Copy constructor and assignment operator for C++98
    DebugStream(const DebugStream& other) : buffer(other.buffer.str()), autoFlush(other.autoFlush) {}
    DebugStream& operator=(const DebugStream& other) {
        if (this != &other) {
            buffer.str(other.buffer.str());
            buffer.clear();
            autoFlush = other.autoFlush;
        }
        return *this;
    }
#else
    // Delete copy constructor and assignment operator for C++11 and later
    DebugStream(const DebugStream& other) = delete;
    DebugStream& operator=(const DebugStream& other) = delete;
#endif
    
    ~DebugStream() {
        Flush();
    }

    /**
     * @brief Flushes the current contents of the buffer to the debug output.
     *
     * This function sends the current contents of the buffer to the debug output
     * using OutputDebugStringW. After flushing, it clears the buffer for future use.
     */
    inline void Flush() {
        OutputDebugStringW(buffer.str().c_str());
        buffer.str(L"");
        buffer.clear();
    }

    /**
     * @brief Overloaded insertion operator for DebugStream.
     * 
     * This template function allows any type T to be inserted into the DebugStream.
     * The value is first inserted into the internal buffer. If autoFlush is enabled,
     * the buffer is flushed immediately after the insertion.
     * 
     * @tparam T The type of the value to be inserted into the DebugStream.
     * @param value The value to be inserted into the DebugStream.
     * @return DebugStream& A reference to the current DebugStream object.
     */
    template<typename T>
    inline DebugStream& operator<<(const T& value) {
        buffer << value;
        if (autoFlush) Flush();
        return *this;
    }

    // For endl and other basic_ostream manipulators
    /**
     * Overloaded operator<< for DebugStream to handle wide character stream manipulators.
     *
     * This function allows the use of standard wide character stream manipulators (such as std::endl)
     * with the DebugStream class. It applies the manipulator to the internal buffer and flushes the
     * buffer if autoFlush is enabled.
     *
     * @param manip A function pointer to a wide character stream manipulator.
     * @return A reference to the current DebugStream object.
     */
    inline DebugStream& operator<<(std::basic_ostream<wchar_t>& (*manip)(std::basic_ostream<wchar_t>&)) {
        manip(buffer);
        if (autoFlush) Flush();
        return *this;
    }

    // For ios_base manipulators (hex, dec, etc)
    /**
     * Overloads the insertion operator to handle stream manipulators.
     *
     * This function allows the use of standard stream manipulators (such as std::endl)
     * with the DebugStream class. It applies the manipulator to the internal buffer
     * and flushes the buffer if autoFlush is enabled.
     *
     * @param manip A function pointer to a stream manipulator.
     * @return A reference to the current DebugStream object.
     */
    inline DebugStream& operator<<(std::ios_base& (*manip)(std::ios_base&)) {
        manip(buffer);
        if (autoFlush) Flush();
        return *this;
    }

    // For basic_ios manipulators
    /**
     * Overloaded insertion operator for handling stream manipulators.
     *
     * This operator allows the use of stream manipulators (such as std::endl) with the DebugStream class.
     *
     * @param manip A function pointer to a stream manipulator that takes and returns a reference to a std::basic_ios<wchar_t> object.
     * @return A reference to the current DebugStream object.
     */
    inline DebugStream& operator<<(std::basic_ios<wchar_t>& (*manip)(std::basic_ios<wchar_t>&)) {
        manip(buffer);
        if (autoFlush) Flush();
        return *this;
    }

    // Specialized handling for C-style strings
    /**
     * @brief Overloaded insertion operator for DebugStream to handle C-style strings.
     * 
     * This operator allows you to use the insertion operator (<<) with C-style strings
     * (const char*) to output them to the DebugStream. It converts the input string
     * and outputs it using the ConvertAndOutput function.
     * 
     * @param value The C-style string to be output to the DebugStream.
     * @return A reference to the current DebugStream object.
     */
    inline DebugStream& operator<<(const char* value) {
        ConvertAndOutput(value);
        return *this;
    }

    // Specialized handling for std::string
    /**
     * @brief Overloaded insertion operator for DebugStream to handle std::string.
     * 
     * This operator allows a DebugStream object to accept a std::string and 
     * output its content by converting it to a C-style string and passing it 
     * to the ConvertAndOutput function.
     * 
     * @param value The std::string to be output.
     * @return A reference to the current DebugStream object.
     */
    inline DebugStream& operator<<(const std::string& value) {
        ConvertAndOutput(value.c_str());
        return *this;
    }

    // Direct wide string handling
    /**
     * @brief Overloaded insertion operator for wide character strings.
     *
     * This operator allows wide character strings (const wchar_t*) to be inserted
     * into the DebugStream. If the value is not null, it appends the string to the
     * internal buffer. If autoFlush is enabled, it will automatically flush the buffer.
     *
     * @param value The wide character string to be inserted into the DebugStream.
     * @return A reference to the current DebugStream object.
     */
    inline DebugStream& operator<<(const wchar_t* value) {
        if (value) {
            buffer << value;
            if (autoFlush) Flush();
        }
        return *this;
    }

    /**
     * @brief Overloaded insertion operator for std::wstring.
     *
     * This operator allows you to insert a std::wstring into the DebugStream.
     * The value is appended to the internal buffer. If autoFlush is enabled,
     * the buffer is flushed immediately after the value is inserted.
     *
     * @param value The std::wstring to be inserted into the DebugStream.
     * @return A reference to the current DebugStream object.
     */
    inline DebugStream& operator<<(const std::wstring& value) {
        buffer << value;
        if (autoFlush) Flush();
        return *this;
    }
};

/**
 * @brief Creates a DebugStream object for logging debug messages.
 *
 * This function mimics the behavior of qDebug() in Qt which
 * returns a DebugStream object that can be used to log debug messages.
 *
 * @return A DebugStream object for logging debug messages.
 */
inline DebugStream Debug() {
    return DebugStream();
}

#else

/**
 * @class DebugStream
 * @brief A dummy class that provides a no-op stream-like interface in release builds.
 * 
 * DebugStream is a dummy class that provides a no-op stream-like interface in release builds.
 */
class DebugStream {
public:
    template<typename T>
    DebugStream& operator<<(const T&) { return *this; }

    // Support manipulators in release build too
    DebugStream& operator<<(std::basic_ostream<wchar_t>& (*)(std::basic_ostream<wchar_t>&)) { return *this; }
    DebugStream& operator<<(std::ios_base& (*)(std::ios_base&)) { return *this; }
    DebugStream& operator<<(std::basic_ios<wchar_t>& (*)(std::basic_ios<wchar_t>&)) { return *this; }

    void Flush() {}
};

/**
 * @brief Creates a dummy DebugStream object for logging debug messages in release builds.
 *
 * This function mimics the behavior of qDebug() in Qt which
 * returns a DebugStream object that can be used to log debug messages.
 *
 * @return A dummy DebugStream object for logging debug messages in release builds.
 */
inline DebugStream Debug() {
    return DebugStream();
}

#endif