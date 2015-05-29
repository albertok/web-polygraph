
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/LogComment.h"
#include "client/SessionWatchRegistry.h"
#include "client/Client.h"


class SessionAnnouncer: public SessionWatch<Client> {
	public:
		virtual String id() const;
		virtual void describe(ostream &os) const;

		virtual void noteStart(const Client *client);
		virtual void noteHeartbeat(const Client *client);
		virtual void noteEnd(const Client *client);

	protected:
		void announce(const Client *client, const String &state) const;
};


static bool registered = registered ||
	TheSessionWatchRegistry().add(new SessionAnnouncer);


String SessionAnnouncer::id() const {
	return "SessionAnnouncer-1.0";
}

void SessionAnnouncer::describe(ostream &os) const {
	os << "announces client-side sessions on the console";
}

void SessionAnnouncer::noteStart(const Client *client) {
	announce(client, "starts session");
	Comment(5) << "robot credentials: " << client->credentials() << endc;
}

void SessionAnnouncer::noteHeartbeat(const Client *client) {
	announce(client, "continues session");
}

void SessionAnnouncer::noteEnd(const Client *client) {
	announce(client, "ends session");
}

void SessionAnnouncer::announce(const Client *client, const String &action) const {
	Comment(5) << "robot " << client->id() << " @ " << client->host() << 
		": " << action << endc;
}

