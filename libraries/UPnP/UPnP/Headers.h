/*
 * UPnP/Headers.h
 *
 * Copyright (c) 2015 Danny Backx.
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


#ifndef _INCLUDE_Headers_H_
#define _INCLUDE_Headers_H_

enum UPnPHeader {
	//
	UPNP_METHOD_NONE,

	UPNP_METHOD_NOTIFY,
	UPNP_METHOD_HOST,
	UPNP_METHOD_CONTENT_TYPE,
	UPNP_METHOD_CALLBACK,
	UPNP_METHOD_NTS,
	UPNP_METHOD_NT,
	UPNP_METHOD_SID,
	UPNP_METHOD_UUID,
	UPNP_METHOD_SEQ,
	UPNP_METHOD_STATEVAR,
	UPNP_METHOD_TIMEOUT,
	UPNP_METHOD_CONTENTLENGTH,

	UPNP_END_METHODS
	// Don't add after this
};

static const char *upnp_header_strings[] = {
	"",
	"NOTIFY",
	"HOST",
	"CONTENT-TYPE",
	"CALLBACK",
	"NTS",
	"NT",
	"SID",
	"UUID",
	"SEQ",
	"STATEVAR",
	"TIMEOUT",
	"Content-Length",

	NULL
	// Don't add after this
};

extern char *upnp_headers[];

#endif // _INCLUDE_Headers_H_
