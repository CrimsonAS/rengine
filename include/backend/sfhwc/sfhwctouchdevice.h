/*
    Copyright (c) 2016, Gunnar Sletta <gunnar@sletta.org>
    Copyright (c) 2016, Jolla Ltd, author: <gunnar.sletta@jollamobile.com>
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <mtdev.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <mtdev.h>

#include <string>
#include <thread>
#include <mutex>

RENGINE_BEGIN_NAMESPACE

inline bool SfHwcTouchDevice::initialize(const char *deviceName)
{
    int fd = open(deviceName, O_RDONLY, 0);
    if (fd < 0) {
        printf("Failed to open input device '%s'\n", deviceName);
        return false;
    }
    m_fd = fd;

    m_dev = mtdev_new_open(m_fd);
    if (!m_dev) {
        printf("Failed to create/open mtdev...\n");
        close();
        return false;
    }

    m_minX = mtdev_get_abs_minimum(m_dev, ABS_MT_POSITION_X);
    m_maxX = mtdev_get_abs_maximum(m_dev, ABS_MT_POSITION_X);
    m_minY = mtdev_get_abs_minimum(m_dev, ABS_MT_POSITION_Y);
    m_maxY = mtdev_get_abs_maximum(m_dev, ABS_MT_POSITION_Y);
    m_minPressure = mtdev_get_abs_minimum(m_dev, ABS_MT_PRESSURE);
    m_maxPressure = mtdev_get_abs_maximum(m_dev, ABS_MT_PRESSURE);

    char name[1024];
    if (ioctl(m_fd, EVIOCGNAME(sizeof(name) - 1), name) < 0) {
        printf("Failed to read device name..\n");
        return false;
    }
    m_name = std::string(name);

    m_thread = std::thread(&SfHwcTouchDevice::run, this);

    return true;
}

inline void SfHwcTouchDevice::close()
{
    if (m_dev)
        mtdev_close(m_dev);
    m_dev = 0;

    if (m_fd >= 0)
        ::close(m_fd);
    m_fd = -1;
}

inline void SfHwcTouchDevice::lock()
{
    m_mutex.lock();
}

inline void SfHwcTouchDevice::unlock()
{
    m_mutex.unlock();
}

inline SfHwcTouchDevice::SfHwcTouchDevice()
{
    memset(&m_pending, 0, sizeof(State));
    memset(&m_state, 0, sizeof(State) * RENGINE_MAX_TOUCH_HISTORY);
}

inline void SfHwcTouchDevice::readEvent(const input_event &e)
{
    // double t = e.time.tv_sec * 1000 + e.time.tv_usec / 1000.0;
    // static double lastTime;
    // printf("event: %10.2f, delta=%.2f, (%x, %x)\n", t, lastTime - t, e.type, e.code);
    // lastTime = t;

    if (e.type == EV_SYN) {
        // printf("        -- got EV_SYN\n");
        if (e.code == SYN_REPORT) {
            // Make the pending state "actual"
            lock();
            m_stateIndex = (m_stateIndex + 1) % RENGINE_MAX_TOUCH_HISTORY;
            memcpy(&m_state[m_stateIndex], &m_pending, sizeof(State));
            m_state[m_stateIndex].time = e.time;
            unlock();
        }
    } else if (e.type == EV_ABS) {
        // printf("        -- got EV_ABS\n");
        if (e.code == ABS_MT_SLOT) {
            m_slot = e.value;
            // printf("        --- slot is: %d\n", m_slot);
        } else if (m_slot >= 0 && m_slot < RENGINE_MAX_TOUCH_POINTS) {
            Contact &c = m_pending.contacts[m_slot];
            if (e.code == ABS_MT_POSITION_X) {
                // printf("        --- x is: %d\n", e.value);
                c.x = e.value;
            } else if (e.code == ABS_MT_POSITION_Y) {
                // printf("        --- y is: %d\n", e.value);
                c.y = e.value;
            } else if (e.code == ABS_MT_TRACKING_ID) {
                // printf("        --- id is: %d\n", e.value);
                c.id = e.value;
                if (e.value >= 0)
                    ++m_pending.count;
                else
                    --m_pending.count;
            }
        }
    }
}

inline void SfHwcTouchDevice::run()
{
    while (m_fd >= 0) {
        input_event event;
        int read = mtdev_get(m_dev, m_fd, &event, 1);
        for (int i=0; i<read; ++i) {
            readEvent(event);
        }
    }

}

RENGINE_END_NAMESPACE