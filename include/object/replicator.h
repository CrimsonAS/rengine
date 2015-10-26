/*
    Copyright (c) 2015, Gunnar Sletta <gunnar@sletta.org>
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

RENGINE_BEGIN_NAMESPACE

template <typename T, typename Context>
class Replicator
{
public:

    int count() const { return m_instances.size(); }

    /*
        Sets the number of objects this replicator should instantiate.

        The instances are synchronously created and onInstanceCreated()
        will be called for each instance.

        The function assumes that the class to instantiate has a ::create()
        and ::destroy function.
     */
    void setCount(unsigned count) {
        // add missing elements
        while (count > m_instances.size()) {
            T *t = onCreateInstance(m_instances.size(), count);
            m_instances.push_back(t);
        }

        // remove overflowing elements
        while (count < m_instances.size()) {
            T *t = m_instances.back();
            m_instances.pop_back();
            onDestroyInstance(t);
        }
    }

    /*
        Returns the replicated instance for \a index.
     */
    T *at(unsigned index) const {
        assert(index < count());
        return m_instances.at(index);
    }

    /*
        Called to replicate a new instance, the instance being number \a index
        out of the total \a count to replicate.
     */

    virtual T *onCreateInstance(unsigned index, unsigned count) = 0;

    /*
        This function is called to destroy a given instance when the
        replicator's count is reduced.
     */

    virtual void onDestroyInstance(T *instance) = 0;

private:
    std::vector<T *> m_instances;
};

RENGINE_END_NAMESPACE