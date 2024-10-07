// dbus_client.c
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "server.h"

void send_reply(DBusConnection* conn, const char* msg_content) {
    DBusMessageIter args;
    DBusError err;
    dbus_error_init(&err);

    // Create a new signal
    DBusMessage* msg = dbus_message_new_signal(DBUS_OBJECT_PATH, DBUS_INTERFACE_NAME, DBUS_SIGNAL_NAME);
    if (!msg) {
        fprintf(stderr, "Message Null\n");
        return;
    }

    // Append arguments
    dbus_message_iter_init_append(msg, &args);
    const char* server_id = SERVER_ID;
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &server_id)) {
        fprintf(stderr, "Out Of Memory!\n");
        return;
    }
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &msg_content)) {
        fprintf(stderr, "Out Of Memory!\n");
        return;
    }

    printf("Message sent: ACK\n");
    // Send the message and flush the connection
    if (!dbus_connection_send(conn, msg, NULL)) {
        fprintf(stderr, "Out Of Memory!\n");
        return;
    }
    dbus_connection_flush(conn);

    // Free the message
    dbus_message_unref(msg);
}


void receive_message(DBusConnection* conn) {
    DBusMessage* msg;
    DBusMessageIter args;
    char* msg_id;
    char* msg_content;

    dbus_connection_read_write(conn, 0);
    msg = dbus_connection_pop_message(conn);

    if (!msg) return;

    if (dbus_message_is_signal(msg, DBUS_INTERFACE_NAME, DBUS_SIGNAL_NAME)) {
        if (!dbus_message_iter_init(msg, &args))
            fprintf(stderr, "Message has no arguments!\n");
        else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
            fprintf(stderr, "Argument is not string!\n");
        else
            dbus_message_iter_get_basic(&args, &msg_id);

        dbus_message_iter_next(&args);
        if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
            fprintf(stderr, "Second argument is not string!\n");
        else
            dbus_message_iter_get_basic(&args, &msg_content);

        if (strcmp(msg_id, SERVER_ID) != 0) {
            printf("Received Msg: %s\n", msg_content);
        }
        // Send reply message
        send_reply(conn, "ACK");
    }

    dbus_message_unref(msg);
}

int main() {
    DBusConnection* conn;
    DBusError err;

    dbus_error_init(&err);

    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (!conn) {
        exit(1);
    }

    dbus_bus_add_match(conn, "type='signal',interface='" DBUS_INTERFACE_NAME "'", &err);
    dbus_connection_flush(conn);

    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Match Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    while (1) {
        receive_message(conn);
        usleep(500000);
    }

    return 0;
}
