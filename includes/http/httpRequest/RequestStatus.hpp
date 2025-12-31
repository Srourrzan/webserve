/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestStatus.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/30 16:44:42 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/31 15:53:31 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

enum RequestStatus
{
    REQ_OK = 200,
    REQ_CREATED = 201,
    REQ_NO_CONTENT = 204,
    REQ_MULTIPLE_CHOICES = 300,
    REQ_MOVED_PERMANENTLY = 301,
    REQ_FOUND = 302,
    REQ_BAD_REQUEST = 400,
    REQ_FORBIDDEN = 403, 
    REQ_NOT_FOUND = 404,
    REQ_METHOD_NOT_ALLOWED = 405,
    REQ_CONFLICT = 409,
    REQ_PAYLOAD_TOO_LARGE = 413,
    REQ_URI_TOO_LONG = 414,
    REQ_INTERNAL_SERVER_ERROR = 500, 
    REQ_NOT_IMPLEMENTED = 501,
    REQ_VERSION_NOT_SUPPORTED = 505
};