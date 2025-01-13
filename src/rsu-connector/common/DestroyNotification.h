// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef CONNECTOR_COMMON_DESTROYNOTIFICATIONE_H_
#define CONNECTOR_COMMON_DESTROYNOTIFICATIONE_H_
#include <functional>

/// @brief Derive from this class to provide a callback when the class is destroyed.
class DestroyNotification
{
public:
    ~DestroyNotification()
    {
        if ( destroyNotification )
        {
            destroyNotification();
        }
    }
    /// @brief Set callback to be called when the class is destroyed.
    /// @param notification function to call when the class is destroyed.
    void SetDestroyNotification( std::function<void()> notification ) { destroyNotification = notification; }

protected:
    std::function<void()> destroyNotification{ []() {} };
};

#endif /* CONNECTOR_COMMON_DESTROYNOTIFICATIONE_H_ */
