/*
    Copyright (c) 2016, Gunnar Sletta <gunnar@sletta.org>
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

#include <thread>
#include <mutex>
#include <condition_variable>

RENGINE_BEGIN_NAMESPACE

/*!
    The WorkQueue class encapsulates a queue of jobs that will be run on a
    background thread.

    Jobs are schedule for execution by calling WorkQueue::schedule().

    Jobs are implemented by reimplementing WorkQueue::Job and the
    Job::onExecute() virtual function. Jobs will be completed in their own
    time, but the job can be checked for completion on the calling thread
    through Job::hasCompleted() and it is possible to wait for a job to
    complete with Job::waitForCompletion().
 */

class WorkQueue
{
public:

    class Job
    {
    public:
        virtual ~Job() { }

        /*!
            Returns if the job has completed or not. This function is thread-
            safe.
         */
        bool hasCompleted() const;

        /*!
            Blocks until the job has completed. The function returns
            immediately, if the job has already been completed. This function
            is thread-safe.
         */
        void waitForCompletion();

        /*!
            Custom jobs should override this function to perform jobs on the
            work queue. The job will be marked as completed immediately after
            the onExecute() function completes.
         */
        virtual void onExecute() { }

    private:
        friend class WorkQueue;

        mutable std::mutex m_mutex;
        std::condition_variable m_condition;
        bool m_completed = false;
    };



    WorkQueue();
    ~WorkQueue();

    /*!
        Place \a job into the work queue.

        The function will return right away and the job's onExecute() function
        will be called on another thread at a later time.

        The execution order of jobs is first-in, first-out, so one can
        schedule multiple jobs to be run, and then wait for the final one to
        complete as means of performing batch processing.

        If the job was already completed, its completed state will be reset
        before being entered into the queue, allowing it to be checked or
        waited for again.
     */
    void schedule(const std::shared_ptr<Job> &job);

private:

    /*!
        The method for the running thread. This spins a while loop, waiting
        for jobs to appear and then immediately executes them.
     */
    void run();

    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::list<std::shared_ptr<Job>> m_jobs;

    bool m_running = true;
};

inline WorkQueue::WorkQueue()
    : m_thread(&WorkQueue::run, this)
{
}

inline WorkQueue::~WorkQueue()
{
    // Tell thread to exit..
    m_mutex.lock();
    m_running = false;
    m_condition.notify_one();
    m_mutex.unlock();

    // Wait for it to finish..
    m_thread.join();
}

inline void WorkQueue::run()
{
    bool running = m_running;
    while (running) {
        std::unique_lock<std::mutex> locker(m_mutex);
        if (m_jobs.empty()) {
            m_condition.wait(locker);
        }
        std::shared_ptr<Job> job;
        if (!m_jobs.empty()) {
            job = m_jobs.front();
            m_jobs.pop_front();
        }

        // While still holding the mutex, check if we should abort..
        running = m_running;
        locker.unlock();

        // Then, if we're still running, execute the job, mark it completed
        // and then the signal potential waiter.
        if (running && job.get() != nullptr) {
            assert(!job->m_completed);
            job->onExecute();
            job->m_mutex.lock();
            job->m_completed = true;
            job->m_condition.notify_one();
            job->m_mutex.unlock();
        }
    }
}

inline void WorkQueue::schedule(const std::shared_ptr<Job> &job)
{
    std::lock_guard<std::mutex> locker(m_mutex);

    // Make sure the thing is not scheduled again
    assert(std::find(m_jobs.begin(), m_jobs.end(), job) == m_jobs.end());

    // Clear the flag.. This is done to allow us to reuse the same instance
    // for multiple calls.
    job->m_completed = false;

    // Add the job and wake up the work thread in case it was dosing off.
    m_jobs.push_back(job);
    m_condition.notify_one();
}



inline bool WorkQueue::Job::hasCompleted() const
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_completed;
}

inline void WorkQueue::Job::waitForCompletion()
{
    std::unique_lock<std::mutex> locker(m_mutex);
    if (!m_completed)
        m_condition.wait(locker);
}


RENGINE_END_NAMESPACE
