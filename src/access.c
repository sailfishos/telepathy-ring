/*
 * access.c - Check for dbus permissions
 *
 * Copyright (C) 2019 Jolla Ltd
 *
 * This work is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define POLICY "1;group(privileged) | group(sailfish-radio) = allow;"

#include "access.h"

#include <dbus/dbus.h>
#include <dbusaccess_policy.h>
#include <dbusaccess_peer.h>
#include <glib.h>

void *
access_user_data_new(void)
{
  return da_policy_new(POLICY);
}

void
access_user_data_free(void *user_data)
{
  da_policy_unref((void *)user_data);
}

static inline gboolean
is_allowed(DBusMessage *message, DAPolicy *policy)
{
  const char *sender = dbus_message_get_sender(message);
  DAPeer *peer = da_peer_get(DA_BUS_SESSION, sender);

  if (!peer)
    return FALSE;

  if (da_policy_check(policy, &peer->cred, 0, NULL, DA_ACCESS_DENY) == DA_ACCESS_ALLOW)
    return TRUE;

  return FALSE;
}

DBusHandlerResult
access_filter_non_privileged(
  DBusConnection *connection,
  DBusMessage *message,
  void *user_data)
{
  DBusMessage *error_reply;

  if (dbus_message_is_method_call(message,
          "org.freedesktop.DBus.Introspectable", "Introspect"))
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

  if (dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_METHOD_CALL &&
      !is_allowed(message, (DAPolicy *)user_data)) {
    // Reply with error org.freedesktop.DBus.Error.AccessDenied
    error_reply = dbus_message_new_error(message, DBUS_ERROR_ACCESS_DENIED,
        "Caller must be privileged");
    if (error_reply) {
      dbus_message_set_reply_serial(error_reply, dbus_message_get_serial(message));
      dbus_connection_send(connection, error_reply, NULL);
      dbus_message_unref(error_reply);
    }

    return DBUS_HANDLER_RESULT_HANDLED;
  }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
