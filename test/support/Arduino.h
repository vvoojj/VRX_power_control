#pragma once

#include <stdint.h>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define F(value) value

class TestSerial {
public:
    std::string input;
    std::string output;
    size_t position = 0;
    unsigned flushCount = 0;

    int available() const { return (int)(input.size() - position); }
    int read() { return available() ? (unsigned char)input[position++] : -1; }
    void print(const char* value) { output += value; }
    void print(char value) { output += value; }
    void print(uint8_t value) { print((unsigned)value); }

    template <typename T>
    void print(T value) {
        std::ostringstream stream;
        stream << value;
        output += stream.str();
    }

    void println() { output += "\r\n"; }

    template <typename T>
    void println(T value) {
        print(value);
        println();
    }

    void flush() { ++flushCount; }
};

inline TestSerial& testSerial() {
    static TestSerial serial;
    return serial;
}

#define Serial testSerial()

inline std::vector<std::pair<uint8_t, uint8_t> >& testPinWrites() {
    static std::vector<std::pair<uint8_t, uint8_t> > writes;
    return writes;
}

inline void digitalWrite(uint8_t pin, uint8_t value) {
    testPinWrites().push_back(std::make_pair(pin, value));
}

inline void pinMode(uint8_t, uint8_t) {}
inline unsigned long millis() { return 0; }
