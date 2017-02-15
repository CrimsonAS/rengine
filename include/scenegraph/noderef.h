/*
    Copyright (c) 2017, Gunnar Sletta <gunnar@crimson.no>
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

template <typename NodeClass>
class NodeRef
{
public:
    NodeRef(NodeClass *n);
    NodeRef(const NodeRef &other);
    NodeRef();

    ~NodeRef();

    NodeClass *get() const;

    void operator=(const NodeRef &other);

    NodeClass *operator->();
    const NodeClass *operator->() const;

    bool expired() const { return !m_data || !m_data->node; } 

private:
    struct Data : public SignalHandler<> 
    {
        NodeClass *node;
        int ref;
        void onSignal() override { node = 0; }
    };

    void deref();

    Data *m_data;
};

template <typename NodeClass>
NodeRef<NodeClass>::NodeRef(NodeClass *n)
    : m_data(new Data())
{
    assert(n != nullptr);
    m_data->ref = 1;
    m_data->node = n;
    NodeClass::onDestruction.connect(n, m_data);
}

template <typename NodeClass>
NodeRef<NodeClass>::NodeRef(const NodeRef &other)
{
    m_data = other->m_data;
    ++m_data->ref;
}

template <typename NodeClass>
NodeRef<NodeClass>::NodeRef()
    : m_data(nullptr)
{
}

template <typename NodeClass>
NodeRef<NodeClass>::~NodeRef()
{
    deref();
}

template <typename NodeClass>
NodeClass *NodeRef<NodeClass>::get() const 
{
    return m_data ? m_data->node : nullptr;
}

template <typename NodeClass>
void NodeRef<NodeClass>::operator=(const NodeRef &other)
{
    Data *d = other.m_data;
    if (d) 
        ++d->ref;
    deref();
    m_data = d;
}

template <typename NodeClass>
void NodeRef<NodeClass>::deref()
{
    if (m_data) {
        --m_data->ref;
        assert(m_data->ref >= 0);
        if (m_data->ref == 0) {
            delete m_data;
            m_data = nullptr; // strictly speaking superflous, but 
        }
    }
}

template <typename NodeClass>
NodeClass *NodeRef<NodeClass>::operator->()
{
    return m_data ? m_data->node : nullptr;
}

template <typename NodeClass>
const NodeClass *NodeRef<NodeClass>::operator->() const
{
    return m_data ? m_data->node : nullptr;
}


RENGINE_END_NAMESPACE
