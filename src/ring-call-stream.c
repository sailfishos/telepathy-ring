/*
 * ring-call-stream.c - a Stream object owned by a RingCallContent
 * Copyright ©2010 Collabora Ltd.
 * Copyright ©2010 Nokia Corporation
 *   @author Will Thompson <will.thompson@collabora.co.uk>
 * @author Tom Swindell <t.swindell@rubyx.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "ring-call-stream.h"

#define DEBUG_FLAG RING_DEBUG_MEDIA
#include "ring-debug.h"

static void implement_call_stream (gpointer klass, gpointer unused);

G_DEFINE_TYPE(RingCallStream, ring_call_stream,
    TP_TYPE_BASE_MEDIA_CALL_STREAM);

static void
ring_call_stream_init (RingCallStream *self)
{
}

static void
ring_call_stream_class_init (RingCallStreamClass *klass)
{
}

RingCallStream *
ring_call_stream_new (RingConnection *connection,
    const gchar *object_path)
{
  return g_object_new (RING_TYPE_CALL_STREAM,
      "connection", connection,
      "object-path", object_path,
      NULL);
}

static void
ring_call_stream_set_sending (
    RingCallStream *self,
    gboolean send,
    DBusGMethodInvocation *context)
{
  GError error = { TP_ERRORS, TP_ERROR_NOT_IMPLEMENTED,
      "SetSending is not supported for cellular calls." };

 /* Maybe we should put the call on hold/resume? */

  dbus_g_method_return_error (context, &error);
}

static void
ring_call_stream_request_receiving (
    RingCallStream *self,
    TpHandle contact,
    gboolean receive,
    DBusGMethodInvocation *context)
{
  GError error = { TP_ERRORS, TP_ERROR_NOT_IMPLEMENTED,
      "RequestReceiving is not supported for cellular calls." };

  dbus_g_method_return_error (context, &error);
}
