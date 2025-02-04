/*
 * ring-call-member.c - Source for CallMember
 * Copyright (C) 2010 Collabora Ltd.
 * @author Sjoerd Simons <sjoerd.simons@collabora.co.uk>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "util.h"

#include "base-call-channel.h"

#include "ring-connection.h"
#include "ring-call-member.h"

G_DEFINE_TYPE(RingCallMember, ring_call_member, G_TYPE_OBJECT)

/* signal enum */
enum
{
    FLAGS_CHANGED,
    CONTENT_ADDED,
    CONTENT_REMOVED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* properties */
enum
{
  PROP_CALL = 1,
  PROP_TARGET
};

/* private structure */
struct _RingCallMemberPrivate
{
  TpHandle target;

  RingBaseCallChannel *call;
  TpCallMemberFlags flags;

  GList *contents;
  gchar *transport_ns;
  gboolean accepted;

  gboolean dispose_has_run;
};

#define RING_CALL_MEMBER_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), RING_TYPE_CALL_MEMBER, \
    RingCallMemberPrivate))

static void
ring_call_member_init (RingCallMember *self)
{
  RingCallMemberPrivate *priv =
    RING_CALL_MEMBER_GET_PRIVATE (self);

  self->priv = priv;
  priv->accepted = FALSE;
}

static void
ring_call_member_get_property (GObject    *object,
    guint       property_id,
    GValue     *value,
    GParamSpec *pspec)
{
  RingCallMember *self = RING_CALL_MEMBER (object);
  RingCallMemberPrivate *priv = self->priv;

  switch (property_id)
    {
      case PROP_CALL:
        g_value_set_object (value, ring_call_member_get_connection (self));
        break;
      case PROP_TARGET:
        g_value_set_uint (value, priv->target);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
ring_call_member_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  RingCallMember *self = RING_CALL_MEMBER (object);
  RingCallMemberPrivate *priv = self->priv;

  switch (property_id)
    {
      case PROP_CALL:
        priv->call = g_value_get_object (value);
        g_assert (priv->call != NULL);
        break;
      case PROP_TARGET:
        priv->target = g_value_get_uint (value);
        g_assert (priv->target != 0);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
  }
}

static void ring_call_member_dispose (GObject *object);
static void ring_call_member_finalize (GObject *object);

static void
ring_call_member_class_init (
    RingCallMemberClass *ring_call_member_class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (ring_call_member_class);
  GParamSpec *param_spec;

  g_type_class_add_private (ring_call_member_class,
      sizeof (RingCallMemberPrivate));

  object_class->dispose = ring_call_member_dispose;
  object_class->finalize = ring_call_member_finalize;

  object_class->get_property = ring_call_member_get_property;
  object_class->set_property = ring_call_member_set_property;

  param_spec = g_param_spec_object ("call", "Call",
      "The base call object that contains this member",
      RING_TYPE_BASE_CALL_CHANNEL,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_CALL, param_spec);

  param_spec = g_param_spec_uint ("target", "Target",
      "the target handle of member",
      0,
      G_MAXUINT,
      0,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_TARGET, param_spec);

  signals[FLAGS_CHANGED] =
    g_signal_new ("flags-changed",
                  G_OBJECT_CLASS_TYPE (ring_call_member_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__UINT,
                  G_TYPE_NONE, 1, G_TYPE_UINT);

  signals[CONTENT_ADDED] =
    g_signal_new ("content-added",
                  G_OBJECT_CLASS_TYPE (ring_call_member_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, G_TYPE_OBJECT);

  signals[CONTENT_REMOVED] =
    g_signal_new ("content-removed",
                  G_OBJECT_CLASS_TYPE (ring_call_member_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, G_TYPE_OBJECT);
}

void
ring_call_member_dispose (GObject *object)
{
  RingCallMember *self = RING_CALL_MEMBER (object);
  RingCallMemberPrivate *priv = self->priv;
  GList *l;

  if (priv->dispose_has_run)
    return;

  priv->dispose_has_run = TRUE;

  for (l = priv->contents ; l != NULL; l = g_list_next (l))
    g_object_unref (l->data);

  tp_clear_pointer (&priv->contents, g_list_free);

  /* release any references held by the object here */

  if (G_OBJECT_CLASS (ring_call_member_parent_class)->dispose)
    G_OBJECT_CLASS (ring_call_member_parent_class)->dispose (object);
}

void
ring_call_member_finalize (GObject *object)
{
  RingCallMember *self = RING_CALL_MEMBER (object);
  RingCallMemberPrivate *priv = self->priv;

  g_free (priv->transport_ns);
  priv->transport_ns = NULL;

  G_OBJECT_CLASS (ring_call_member_parent_class)->finalize (object);
}

/*
static void
remote_state_changed_cb (WockyJingleSession *session, gpointer user_data)
{
  RingCallMember *self = RING_CALL_MEMBER (user_data);
  RingCallMemberPrivate *priv = self->priv;
  TpCallMemberFlags newflags = 0;

  if (wocky_jingle_session_get_remote_ringing (session))
    newflags |= TP_CALL_MEMBER_FLAG_RINGING;

  if (wocky_jingle_session_get_remote_hold (session))
    newflags |= TP_CALL_MEMBER_FLAG_HELD;

  if (priv->flags == newflags)
    return;

  priv->flags = newflags;

  DEBUG ("Call members flags changed to: %d", priv->flags);

  g_signal_emit (self, signals[FLAGS_CHANGED], 0, priv->flags);
}
*/

/*
static void
member_content_removed_cb (RingCallMemberContent *mcontent,
    gpointer user_data)
{
  RingCallMember *self = RING_CALL_MEMBER (user_data);
  RingCallMemberPrivate *priv = self->priv;

  priv->contents = g_list_remove (priv->contents, mcontent);
  g_signal_emit (self, signals[CONTENT_REMOVED], 0, mcontent);
  g_object_unref (mcontent);
}

static void
ring_call_member_add_member_content (RingCallMember *self,
    RingCallMemberContent *content)
{
  RingCallMemberPrivate *priv = self->priv;

  priv->contents = g_list_prepend (priv->contents, content);

  ring_signal_connect_weak (content, "removed",
      G_CALLBACK (member_content_removed_cb), G_OBJECT (self));

  g_signal_emit (self, signals[CONTENT_ADDED], 0, content);
}
*/

/* This function handles additional contents added by the remote side */
/*
static void
new_content_cb (WockyJingleSession *session,
    WockyJingleContent *c,
    gpointer user_data)
{
  RingCallMember *self = RING_CALL_MEMBER (user_data);
  RingCallMemberContent *content = NULL;

  if (wocky_jingle_content_is_created_by_us (c))
    return;

  content = ring_call_member_content_from_jingle_content (c, self);

  ring_call_member_add_member_content (self, content);
}

static gboolean
call_member_update_existing_content (RingCallMember *self,
    WockyJingleContent *content)
{
  GList *l;

  for (l = self->priv->contents; l != NULL ; l = g_list_next (l))
    {
      RingCallMemberContent *mcontent = RING_CALL_MEMBER_CONTENT (l->data);

      if (ring_call_member_content_has_jingle_content (mcontent))
        continue;

      if (!tp_strdiff (ring_call_member_content_get_name (mcontent),
          wocky_jingle_content_get_name (content)))
        {
          ring_call_member_content_set_jingle_content (mcontent, content);
          return TRUE;
        }
    }

  return FALSE;
}

void
ring_call_member_set_session (RingCallMember *self,
    WockyJingleSession *session)
{
  RingCallMemberPrivate *priv = self->priv;
  GList *c, *contents;

  g_assert (priv->session == NULL);
  g_assert (session != NULL);

  DEBUG ("Setting session: %p -> %p\n", self, session);
  priv->session = g_object_ref (session);

  contents = wocky_jingle_session_get_contents (session);
  for (c = contents ; c != NULL; c = g_list_next (c))
    {
      WockyJingleContent *content = WOCKY_JINGLE_CONTENT (c->data);

      if (priv->transport_ns == NULL)
        {
          g_object_get (content, "transport-ns",
            &priv->transport_ns,
            NULL);
        }

      if (!call_member_update_existing_content (self, content))
        {
          RingCallMemberContent *mcontent =
              ring_call_member_content_from_jingle_content (content,
                self);

          ring_call_member_add_member_content (self, mcontent);
        }
    }

  g_object_notify (G_OBJECT (self), "session");

  ring_signal_connect_weak (priv->session, "remote-state-changed",
    G_CALLBACK (remote_state_changed_cb), G_OBJECT (self));
  ring_signal_connect_weak (priv->session, "new-content",
    G_CALLBACK (new_content_cb), G_OBJECT (self));

  if (priv->accepted)
    ring_call_member_accept (self);

  g_list_free (contents);
}

WockyJingleSession *
ring_call_member_get_session (RingCallMember *self)
{
  return self->priv->session;
}
*/

TpCallMemberFlags
ring_call_member_get_flags (RingCallMember *self)
{
  return self->priv->flags;
}

TpHandle ring_call_member_get_handle (
    RingCallMember *self)
{
  return self->priv->target;
}

GList *
ring_call_member_get_contents (RingCallMember *self)
{
  RingCallMemberPrivate *priv = self->priv;

  return priv->contents;
}

/*
RingCallMemberContent *
ring_call_member_ensure_content (RingCallMember *self,
  const gchar *name,
  WockyJingleMediaType mtype)
{
  RingCallMemberPrivate *priv = self->priv;
  GList *l;
  RingCallMemberContent *content = NULL;

  for (l = priv->contents ; l != NULL; l = g_list_next (l))
    {
      RingCallMemberContent *c = RING_CALL_MEMBER_CONTENT (l->data);

      if (ring_call_member_content_get_media_type (c) == mtype &&
          !tp_strdiff (ring_call_member_content_get_name (c), name))
        {
          content = c;
          break;
        }
    }

  if (content == NULL)
    {
      content = ring_call_member_content_new (name, mtype, self);
      ring_call_member_add_member_content (self, content);
    }

  return content;
}

RingCallMemberContent *
ring_call_member_create_content (RingCallMember *self,
  const gchar *name,
  WockyJingleMediaType mtype,
  WockyJingleContentSenders senders,
  GError **error)
{
  RingCallMemberPrivate *priv = self->priv;
  const gchar *content_ns;
  WockyJingleContent *c;
  RingCallMemberContent *content;
  const gchar *peer_resource;

  g_assert (priv->session != NULL);

  peer_resource = wocky_jingle_session_get_peer_resource (priv->session);

  DEBUG ("Creating new content %s, type %d", name, mtype);

  if (peer_resource != NULL)
    DEBUG ("existing call, using peer resource %s", peer_resource);
  else
    DEBUG ("existing call, using bare JID");

  content_ns = jingle_pick_best_content_type (ring_call_member_get_connection (self),
    priv->target,
    peer_resource, mtype);

  if (content_ns == NULL)
    {
      g_set_error (error, TP_ERROR, TP_ERROR_NOT_AVAILABLE,
        "Content type %d not available for this resource", mtype);
      return NULL;
    }

  DEBUG ("Creating new jingle content with ns %s : %s",
    content_ns, priv->transport_ns);

  c = wocky_jingle_session_add_content (priv->session,
      mtype, senders, name, content_ns, priv->transport_ns);

  g_assert (c != NULL);

  content = ring_call_member_content_from_jingle_content (c, self);

  ring_call_member_add_member_content (self, content);

  return content;
}

void
ring_call_member_accept (RingCallMember *self)
{
  self->priv->accepted = TRUE;

  if (self->priv->session != NULL)
    wocky_jingle_session_accept (self->priv->session);
}
*/

/**
 * Start a new session using the existing contents for this member. For now
 * assumes we're using the latest jingle dialect and ice-udp
 * FIXME: make dialect and transport selection more dynamic?
 */
/*
gboolean
ring_call_member_open_session (RingCallMember *self,
    GError **error)
{
  RingCallMemberPrivate *priv = self->priv;
  RingConnection *conn = ring_call_member_get_connection (self);
  WockyJingleFactory *jf;
  WockyJingleSession *session;
  gchar *jid;

  jid = ring_peer_to_jid (conn, priv->target, NULL);

  jf = ring_jingle_mint_get_factory (conn->jingle_mint);
  g_return_val_if_fail (jf != NULL, FALSE);

  session = wocky_jingle_factory_create_session (jf, jid, WOCKY_JINGLE_DIALECT_V032,
      FALSE);
  DEBUG ("Created a jingle session: %p", session);

  priv->transport_ns = g_strdup (NS_JINGLE_TRANSPORT_ICEUDP);

  ring_call_member_set_session (self, session);

  g_free (jid);

  return TRUE;
}

gboolean
ring_call_member_start_session (RingCallMember *self,
    const gchar *audio_name,
    const gchar *video_name,
    GError **error)
{
  RingCallMemberPrivate *priv = self->priv;
  TpBaseChannel *base_channel = TP_BASE_CHANNEL (priv->call);
  TpHandle target = tp_base_channel_get_target_handle (base_channel);
  const gchar *resource;
  WockyJingleDialect dialect;
  gchar *jid;
  const gchar *transport;
  WockyJingleFactory *jf;
  WockyJingleSession *session;

  // FIXME might need to wait on capabilities, also don't need transport
  // and dialect already
  if (!jingle_pick_best_resource (ring_call_member_get_connection (self),
          target, audio_name != NULL, video_name != NULL,
          &transport, &dialect, &resource))
    {
      g_set_error (error, TP_ERROR, TP_ERROR_NOT_CAPABLE,
        "member does not have the desired audio/video capabilities");
      return FALSE;
    }

  jid = ring_peer_to_jid (ring_call_member_get_connection (self), target, resource);

  jf = ring_jingle_mint_get_factory (
        ring_call_member_get_connection (self)->jingle_mint);
  g_return_val_if_fail (jf != NULL, FALSE);

  session = wocky_jingle_factory_create_session (jf, jid, dialect, FALSE);
  g_free (jid);

  ring_call_member_set_session (self, session);

  priv->transport_ns = g_strdup (transport);

  if (audio_name != NULL)
    ring_call_member_create_content (self, audio_name,
      WOCKY_JINGLE_MEDIA_TYPE_AUDIO, WOCKY_JINGLE_CONTENT_SENDERS_BOTH, NULL);

  if (video_name != NULL)
    ring_call_member_create_content (self, video_name,
      WOCKY_JINGLE_MEDIA_TYPE_VIDEO, WOCKY_JINGLE_CONTENT_SENDERS_BOTH, NULL);

  return TRUE;
}
*/

RingConnection *
ring_call_member_get_connection (RingCallMember *self)
{
  TpBaseChannel *base_chan = TP_BASE_CHANNEL (self->priv->call);

  return RING_CONNECTION (tp_base_channel_get_connection (base_chan));
}

const gchar *
ring_call_member_get_transport_ns (RingCallMember *self)
{
  return self->priv->transport_ns;
}

void
ring_call_member_shutdown (RingCallMember *self)
{
/*
  RingCallMemberPrivate *priv = self->priv;
*/

  /* removing the content will remove it from our list */
/*
  while (priv->contents != NULL)
    ring_call_member_content_remove (
      RING_CALL_MEMBER_CONTENT (priv->contents->data));
*/
}
