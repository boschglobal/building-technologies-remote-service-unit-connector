// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef DEVICECONNECTOR_AZURESDKWRAPPER_IMESSAGELIFETIMETRACKER_H_
#define DEVICECONNECTOR_AZURESDKWRAPPER_IMESSAGELIFETIMETRACKER_H_
#include "common/NonCopyable.h"

/// @brief For one message that requires reliable delivery, this interface is used to track delivery status.
struct IMessageLifeTimeTracker : private NonCopyable<IMessageLifeTimeTracker>
{
    enum class DeliveryState
    {
        Sending,
        SentSuccessful,
        ErrorTimeout,
        ErrorDestroyed,
        ErrorNoSink,
        UnspecifiedError
    };

    virtual ~IMessageLifeTimeTracker() = default;

    /// @brief Used by the IotHub client to store a reference to the message sent.
    virtual void SetReference( void* message ) = 0;

    /// @brief Used by the IotHub client to query the reference associated to this tracker.
    virtual void* Reference() = 0;

    /// @brief The IotHub client will set the message delivery state using this call.
    virtual void SetState( DeliveryState status ) = 0;

    /// @brief Getter for the message delivery state.
    virtual DeliveryState State() = 0;
};

#endif // DEVICECONNECTOR_AZURESDKWRAPPER_IMESSAGELIFETIMETRACKER_H_
