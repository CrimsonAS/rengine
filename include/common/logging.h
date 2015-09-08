/*
    Copyright (c) 2015, Gunnar Sletta <gunnar@sletta.org>
    Copyright (c) 2015, Jolla Ltd, author: <gunnar.sletta@jollamobile.com>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <iostream>
#include <chrono>

RENGINE_BEGIN_NAMESPACE

inline std::string log_timestring()
{
	auto time = std::chrono::system_clock::now().time_since_epoch();
	unsigned s = std::chrono::duration_cast<std::chrono::seconds>(time).count() % 10000;
	unsigned us = std::chrono::duration_cast<std::chrono::microseconds>(time).count() % 1000000;
	// ssss.uuuuu
	char buffer[16];
	snprintf(buffer, sizeof(buffer), "%4d.%05d", s, us);
	return buffer;
}

#ifdef RENGINE_LOG_INFO
#  define logi std::cerr << "I" << log_timestring() << ": "
#else
#  define logi if (0) std::cerr
#endif

#ifdef RENGINE_LOG_DEBUG
#  define logd std::cerr << "D" << log_timestring() << " " << __PRETTY_FUNCTION__ << ": "
#else
#  define logd if (0) std::cerr
#endif

#ifdef RENGINE_LOG_WARNING
#  define logw std::cerr << "W" << log_timestring() << " " << __PRETTY_FUNCTION__ << ": "
#else
#  define logw if (0) std::cerr
#endif

#ifdef RENGINE_LOG_ERROR
#  define loge std::cerr << "E" << log_timestring() << " " << __PRETTY_FUNCTION__ << ": "
#else
#  define loge if (0) std::cerr
#endif

RENGINE_END_NAMESPACE

