#include <iostream>
#include <map>
#include <string>

#include <dbus/dbus.h>

#include "common.h"

using namespace std;

void respond_to_introspect(dbus::Connection& connection, dbus::Message const& request) {
    const char* introspection_data_xml =
        "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\""
        "    \"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">"
        ""
        "<node>"
        "    <interface name=\"org.freedesktop.DBus.Introspectable\">"
        "        <method name=\"Introspect\">"
        "            <arg name=\"data\" direction=\"out\" type=\"s\"/>"
        "        </method>"
        "    </interface>"
        ""
        "    <interface name=\"de.engelmarkus.TheServer\">"
        "        <method name=\"getData\">"
        "            <arg name=\"input\" direction=\"in\" type=\"i\"/>"
        "            <arg name=\"output\" direction=\"out\" type=\"i\"/>"
        "        </method>"
        "    </interface>"
        "</node>"
        "";

    auto reply = dbus::MethodReturn{request.message};

    reply.appendArguments(introspection_data_xml);

    connection.send(reply);
}

void respond_to_getData(dbus::Connection& connection, dbus::Message const& request) {
    int32_t input;

    DBusError error;
    dbus_error_init(&error);

    tie(input) = request.getArguments<int32_t>();

    int32_t output = input;

    auto reply = dbus::MethodReturn{request.message};
    reply.appendArguments(output);

    connection.send(reply);
}

DBusHandlerResult handle_messages(DBusConnection* conn, DBusMessage* msg, void* user_data) {
    auto connection = dbus::Connection{conn};
    auto message = dbus::Message{msg};

    auto interface_name = message.getInterface();
    auto member_name = message.getMember();

    /*if (string(interface_name) == "org.freedesktop.DBus.Introspectable" &&
        string(member_name) == "Introspect")
    {
        respond_to_introspect(connection, message);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else*//* if (string(interface_name) == "de.engelmarkus.TheServer" &&
        string(member_name) == "getData")
    {*/
        respond_to_getData(connection, message);
        return DBUS_HANDLER_RESULT_HANDLED;
    /*}
    else {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }*/
}

int main() {
    DBusError error;
    dbus_error_init(&error);

    auto connection = dbus::Connection{};
    connection.requestName("de.engelmarkus.TheServer");

    DBusObjectPathVTable vtable;
    vtable.message_function = handle_messages;
    vtable.unregister_function = nullptr;

    dbus_connection_try_register_object_path(connection.connection, "/de/engelmarkus/TheServer", &vtable, nullptr, &error);

    if (dbus_error_is_set(&error)) {
        std::cerr << error.message << std::endl;
        exit(1);
    }

    while (true) {
        dbus_connection_read_write_dispatch(connection.connection, 1000);
    }
}
