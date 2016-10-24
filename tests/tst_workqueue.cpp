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

#include "test.h"

static int GLOBAL_COUNTER = 0;

class OneJob : public WorkQueue::Job
{
public:
    OneJob(int id) : jobId(id) { };

    void onExecute() override {
        this_thread::sleep_for(std::chrono::milliseconds(50));
        runId = GLOBAL_COUNTER++;
    }

    int jobId;
    int runId = -1;
};


// The purpose of this test is to verify that the WorkQueue starts up and
// performs the job on its own. This test is slightly simpler than the
// scheduleBatchAndWait one so we can verify that the thread is running
// and executing jobs.

void tst_runOneJob()
{
    GLOBAL_COUNTER = 1;

    WorkQueue queue;
    shared_ptr<WorkQueue::Job> job(new OneJob(1));

    check_true(!job->hasCompleted());
    check_equal(static_cast<OneJob *>(job.get())->runId, -1);

    queue.schedule(job);

    // Spin a while loop, waiting 1 ms at a time, to see if the job completes
    // on its own. If we're still spinning after 10000 iterations, emit a warning
    // that the thing is probably locked up
    int counter = 0;
    while (!job->hasCompleted()) {
        this_thread::sleep_for(std::chrono::milliseconds(1));
        ++counter;
        if (counter > 10000) {
            cout << __FUNCTION__ << ": has with all likelyhood timed out, assuming failure!" << endl;
            check_true(false);
        }
    }

    if (counter == 0) {
        cout << " - job seems to have completed without any delay.. This is suspicious.." << endl;
        return;
    }

    // The runId should be the one we set up above, namely 1, as this is the
    // only job executing at present.
    check_equal(static_cast<OneJob *>(job.get())->runId, 1);

    cout << __FUNCTION__ << ": ok" << endl;
}


// Fire off COUNT jobs and verify that they are executed in the scheduled
// order and that if we wait for the last one, then all the ones preceding it
// have also completed...

void tst_schedulBatchAndWait()
{
    GLOBAL_COUNTER = 0;
    WorkQueue queue;

    const int COUNT = 5;

    std::vector<shared_ptr<WorkQueue::Job> > jobs;

    // Schedule a couple of jobs..
    for (int i=0; i<COUNT; ++i) {
        jobs.push_back(shared_ptr<WorkQueue::Job>(new OneJob(i)));
        queue.schedule(jobs.back());
    }

    // Wait for the very last one...
    jobs.back()->waitForCompletion();

    for (int i=0; i<COUNT; ++i) {
        OneJob *j = static_cast<OneJob *>(jobs.at(i).get());

        // Job should have completed.
        check_true(j->hasCompleted());

        // This will guarantee that the jobs have been completed in the right
        // order..
        check_equal(j->runId, j->jobId);
    }

    cout << __FUNCTION__ << ": ok" << endl;
}



int main(int argc, char **argv)
{
    tst_runOneJob();
    tst_schedulBatchAndWait();

    return 0;
}
