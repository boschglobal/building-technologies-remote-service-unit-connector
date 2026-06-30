// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef DEVICECONNECTOR_JOBSCHEDULER_JOBSCHEDULER_H_
#define DEVICECONNECTOR_JOBSCHEDULER_JOBSCHEDULER_H_
#include "IJobScheduler.h"

/// @brief This class provides a way to schedule tasks in regular intervals.
/// The tick interval can be specified by the user via the constructor.
/// The user is then expected to call the Tick() method once each interval, this class does not start any thread itself.
/// The class also does not call *::now() on a system provided timer to increase testability. If the user cares
/// that the runtime of the jobs will extend the tick interval, this has to be corrected for externally (by reducing the
/// time between ticks).
/// The added jobs will be called when their interval has been reached or exceeded. After a call, their interval counter is reset to zero,
/// so if a job has an interval of 150ms set and the tick interval is 100ms, the job will only be called every 200ms.
/// The user should not depend on the order of task execution.
/// If a tasks throws an exception, it is caught and a log message is generated.
class JobScheduler : public IJobScheduler
{
public:
    JobScheduler() = delete;

    /// @brief Construct a scheduler with the given tick interval.
    /// This interval is added to a counter implemented for every task when Tick() is called.
    /// When this counter overflows the interval of the task, the task will run and the counter will be reset to 0.
    /// @param tickInterval interval expected to have passed between each call to Tick().
    JobScheduler( std::chrono::milliseconds tickInterval );
    ~JobScheduler() = default;

    /// @brief Signal a passed tick interval to the scheduler, executing all tasks that are due.
    /// This method is expected to be called once every tickInterval.
    void Tick();

    /// @brief Add a task to the list of tasks that will be executed.
    /// @param jobName Name to refer to the task. A task will replace an existing one if the same name is reused.
    /// @param interval Interval that the task should be triggered in once.
    /// @param trigger trigger functional to call to run the task.
    /// @return Reference to this instance so calls can be chained.
    IJobScheduler& Add( const std::string& jobName, std::chrono::milliseconds interval, std::function<void()> trigger );

    /// @brief Change the scheduling interval of a task.
    /// @param jobName Name of the job to change, will throw an invalid argument exception if no such task is registered.
    /// @param newInterval New interval for this task.
    void ChangeInterval( const std::string& jobName, std::chrono::milliseconds newInterval );

    /// @brief Suspend the execution of the task.
    /// Will not do anything if the task is already suspended.
    /// @param jobName Name of the job to suspend, will throw an invalid argument exception if no such task is registered.
    void Suspend( const std::string& jobName );

    /// @brief Resume the execution of the task.
    /// Will not do anything if the task is not suspended.
    /// @param jobName Name of the job to resume, will throw an invalid argument exception if no such task is registered.
    void Resume( const std::string& jobName );

#if !defined( UNITTEST_PUBLIC_PIMPL )
private:
#endif

    struct JobSchedulerImpl;
    std::shared_ptr<JobSchedulerImpl> _impl;
};

#endif // DEVICECONNECTOR_JOBSCHEDULER_JOBSCHEDULER_H_
