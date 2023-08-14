#pragma once

#include "NetCommon.hpp"

namespace net
{

template<typename T>
struct MessageHeader
{
    T id{};
    uint32_t size = 0;
};

template <typename T>
struct Message;

template <typename T, typename DataType>
struct SendableTrait {
    static void push(Message<T>& msg, const DataType& data)
    {
        static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");
        
        size_t i = msg.body.size();
        
        msg.body.resize(i + sizeof(DataType));
        
        std::memcpy(msg.body.data() + i, &data, sizeof(DataType));
        
        msg.header.size = msg.size();
    }

    static void pull(Message<T>& msg, DataType& data)
    {
        static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");
        
        size_t i = msg.body.size() - sizeof(DataType);
        
        std::memcpy(&data, msg.body.data() + i, sizeof(DataType));
        
        msg.body.resize(i);
        
        msg.header.size = msg.size();
    }
};

template<typename T>
struct Message
{
    MessageHeader<T> header{};
    std::vector<uint8_t> body;

    Message() {}
    Message(const Message& other) : header(other.header), body(other.body) {}

    size_t size() const
    {
        return body.size();
    }

    friend std::ostream& operator<<(std::ostream& os, const Message<T>& msg)
    {
        os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
        return os;
    }

    template <typename DataType>
    friend Message<T>& operator<<(Message<T>& msg, const DataType& data)
    {
        SendableTrait<T, DataType>::push(msg, data);
        return msg;
    }

    template <typename DataType>
    friend Message<T>& operator>>(Message<T>& msg, DataType& data)
    {
        SendableTrait<T, DataType>::pull(msg, data);
        return msg;
    }
};

template<typename T>
class Connection;

template<typename T>
struct OwnedMessage
{
    std::shared_ptr<Connection<T>> remote = nullptr;
    Message<T> msg;

    friend std::ostream& operator<<(std::ostream& os, const OwnedMessage<T>& msg)
    {
        os << msg.msg;
        return os;
    }
};

// Sendable trait for std::string
template <typename T>
struct SendableTrait<T, std::string> {
    static void push(Message<T>& msg, const std::string& str)
    {
        size_t dataSize = str.size();

        size_t i = msg.body.size();

        msg.body.resize(i + dataSize);

        std::memcpy(msg.body.data() + i, str.data(), dataSize);

        msg.header.size = msg.size();

        msg << dataSize;
    }

    static void pull(Message<T>& msg, std::string& str)
    {
        size_t dataSize;
        msg >> dataSize;
        
        if (dataSize > msg.body.size())
            throw std::runtime_error("Not enough data in the message body to extract the string");
        
        str.clear();
        str.assign(msg.body.end() - dataSize, msg.body.end());
        
        msg.body.erase(msg.body.end() - dataSize, msg.body.end());
        
        msg.header.size = msg.size();
    }
};

template <typename T, typename Q>
struct SendableTrait<T, std::vector<Q>> {
    static void push(Message<T>& msg, const std::vector<Q>& data)
    {
        for (auto& obj : data)
            msg << obj;

        msg << data.size();
    }

    static void pull(Message<T>& msg, std::vector<Q>& data)
    {
        size_t dataSize;
        msg >> dataSize;
        
        if (dataSize > msg.body.size())
            throw std::runtime_error("Not enough data in the message body to extract the vector");
        
        data.clear();
        data.resize(dataSize);
        for (size_t i = 0; i < dataSize; i++)
            msg >> data[i];
    }
};

}