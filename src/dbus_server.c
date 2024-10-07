// dbus_server.c
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "server.h"

void send_message(DBusConnection* conn, const char* msg_content) {
    DBusMessage* msg;
    DBusMessageIter args;
    dbus_uint32_t serial = 0;

    msg = dbus_message_new_signal(DBUS_OBJECT_PATH, DBUS_INTERFACE_NAME, DBUS_SIGNAL_NAME);

    if (!msg) {
        fprintf(stderr, "Message Null\n");
        exit(1);
    }

    dbus_message_iter_init_append(msg, &args);
    const char* client_id = CLIENT_ID;
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &client_id)) {
        fprintf(stderr, "Out Of Memory!\n");
        return;
    }
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &msg_content)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }

    if (!dbus_connection_send(conn, msg, &serial)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }

    dbus_connection_flush(conn);

    dbus_message_unref(msg);
    printf("Message sent: %s\n", msg_content);
}

void receive_reply(DBusConnection* conn) {
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
            fprintf(stderr, "First argument is not string!\n");
        else
            dbus_message_iter_get_basic(&args, &msg_id);

        dbus_message_iter_next(&args);
        if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
            fprintf(stderr, "Second argument is not string!\n");
        else
            dbus_message_iter_get_basic(&args, &msg_content);
            
        if (strcmp(msg_id, CLIENT_ID) != 0) {
            printf("Received Reply with value: %s\n", msg_content);
        }
    }

    dbus_message_unref(msg);
}

int main() {
    DBusConnection* conn;
    DBusError err;
    int ret;

    dbus_error_init(&err);

    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (!conn) exit(1);

    ret = dbus_bus_request_name(conn, DBUS_SERVICE_NAME, DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Name Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        fprintf(stderr, "Failed to request name on the bus.\n");
        exit(1);
    }

    dbus_bus_add_match(conn, "type='signal',interface='" DBUS_INTERFACE_NAME "'", &err);
    dbus_connection_flush(conn);
    
    while (1) {
        send_message(conn, "Hello from Server!");
        receive_reply(conn);
        usleep(500000);
    }

    return 0;
}