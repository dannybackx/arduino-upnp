/*
 * HTTP.h
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


#ifndef _INCLUDE_HTTP_H_
#define _INCLUDE_HTTP_H_

enum HTTPMethod {
	//
	HTTP_ANY,

	// Real HTTP
	HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_PATCH, HTTP_DELETE, HTTP_OPTIONS,

	// UPnP
	HTTP_SUBSCRIBE,
	HTTP_UNSUBSCRIBE,

	//
	HTTP_END_METHODS
	// Don't add after this
};

static const char *http_method_strings[] = {
	"",
	"GET",
	"POST",
	"PUT",
	"PATCH",
	"DELETE",
	"OPTIONS",
	"SUBSCRIBE",
	"UNSUBSCRIBE",


	NULL
	// Don't add after this
};

#endif // _INCLUDE_HTTP_H_
