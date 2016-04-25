#pragma once

#include <cstdint>
#include <iostream>
#include <tuple>
#include <typeinfo>

#include <experimental/tuple>

#include <dbus/dbus.h>

using namespace std;

namespace dbus {
	template <typename T>
	constexpr auto type_to_constant() {
		static_assert(sizeof(T) == 0, "");
	}

	template <>
	constexpr auto type_to_constant<uint8_t>() {
		return DBUS_TYPE_BYTE;
	}

	template <>
	constexpr auto type_to_constant<bool>() {
		return DBUS_TYPE_BOOLEAN;
	}

	template <>
	constexpr auto type_to_constant<int16_t>() {
		return DBUS_TYPE_INT16;
	}

	template <>
	constexpr auto type_to_constant<uint16_t>() {
		return DBUS_TYPE_UINT16;
	}

	template <>
	constexpr auto type_to_constant<int32_t>() {
		return DBUS_TYPE_INT32;
	}

	template <>
	constexpr auto type_to_constant<uint32_t>() {
		return DBUS_TYPE_UINT32;
	}

	template <>
	constexpr auto type_to_constant<int64_t>() {
		return DBUS_TYPE_INT64;
	}

	template <>
	constexpr auto type_to_constant<uint64_t>() {
		return DBUS_TYPE_UINT64;
	}

	template <>
	constexpr auto type_to_constant<double>() {
		return DBUS_TYPE_DOUBLE;
	}

	template <>
	constexpr auto type_to_constant<char const*>() {
		return DBUS_TYPE_STRING;
	}

	template <typename... Args>
	auto prepare_arguments(Args&... args) {
		return tuple_cat(
			make_tuple(
				type_to_constant<Args>(),
				addressof(args)
			)...,

			make_tuple(DBUS_TYPE_INVALID)
		);
	}

	class Error {
	public:
		Error()	{
			dbus_error_init(&error);
		}

		~Error() {
			dbus_error_free(&error);
		}

		bool is_set() const {
			return dbus_error_is_set(&error);
		}

		char const* message() const {
			return error.message;
		}

		DBusError error;
	};

	class Message {
	public:
		Message(DBusMessage* msg)
			: message{msg}
		{
		}

        auto getInterface() const {
            return dbus_message_get_interface(message);
        }

        auto getMember() const {
            return dbus_message_get_member(message);
        }

		template <typename... Args>
		void appendArguments(Args&&... args) {
			experimental::apply(dbus_message_append_args, tuple_cat(
				make_tuple(message),
				prepare_arguments(forward<Args>(args)...)
			));
		}

		template <typename... Args>
		auto getArguments() const {
			auto result = make_tuple(Args()...);

			auto error = Error{};

			experimental::apply(dbus_message_get_args, tuple_cat(
				make_tuple(message, &error.error),
				experimental::apply(prepare_arguments<Args...>, result)
			));

            if (error.is_set()) {
                exit(1);
            }

			return result;
		}

		DBusMessage* message;
	};

	class MethodCall : public Message {
	public:
		MethodCall(char const* a, char const* b, char const* c, char const* d)
			: Message{dbus_message_new_method_call(a, b, c, d)}
		{
		}
	};

    class MethodReturn : public Message {
    public:
        MethodReturn(DBusMessage* request)
            : Message{dbus_message_new_method_return(request)}
        {
        }
    };

	class Connection {
	public:
		Connection() {
			auto error = Error{};

			connection = dbus_bus_get(DBUS_BUS_SESSION, &error.error);

            if (error.is_set()) {
                exit(1);
            }
		}

        Connection(DBusConnection* conn) {
            connection = conn;
        }

        void requestName(char const* name) {
            auto error = Error{};

            dbus_bus_request_name(connection, name, 0, &error.error);

            if (error.is_set()) {
                exit(1);
            }
        }

        void send(Message const& message) {
            dbus_connection_send(connection, message.message, nullptr);
        }

		Message send_with_reply_and_block(Message const& message, int timeout) {
			auto error = Error{};

			auto reply = Message{dbus_connection_send_with_reply_and_block(connection, message.message, timeout, &error.error)};

            if (error.is_set()) {
                exit(1);
            }

			return reply;
		}

		DBusConnection* connection;
	};
};
