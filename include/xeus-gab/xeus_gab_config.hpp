/***************************************************************************
* Copyright (c) 2025, Teddy Randby                                  
*                                                                          
* Distributed under the terms of the MIT license.                 
*                                                                          
* The full license is in the file LICENSE, distributed with this software. 
****************************************************************************/

#ifndef XEUS_GAB_CONFIG_HPP
#define XEUS_GAB_CONFIG_HPP

// Project version
#define XEUS_GAB_VERSION_MAJOR 0
#define XEUS_GAB_VERSION_MINOR 1
#define XEUS_GAB_VERSION_PATCH 0

// Composing the version string from major, minor and patch
#define XEUS_GAB_CONCATENATE(A, B) XEUS_GAB_CONCATENATE_IMPL(A, B)
#define XEUS_GAB_CONCATENATE_IMPL(A, B) A##B
#define XEUS_GAB_STRINGIFY(a) XEUS_GAB_STRINGIFY_IMPL(a)
#define XEUS_GAB_STRINGIFY_IMPL(a) #a

#define XEUS_GAB_VERSION XEUS_GAB_STRINGIFY(XEUS_GAB_CONCATENATE(XEUS_GAB_VERSION_MAJOR,   \
                 XEUS_GAB_CONCATENATE(.,XEUS_GAB_CONCATENATE(XEUS_GAB_VERSION_MINOR,   \
                                  XEUS_GAB_CONCATENATE(.,XEUS_GAB_VERSION_PATCH)))))

#ifdef _WIN32
    #ifdef XEUS_GAB_EXPORTS
        #define XEUS_GAB_API __declspec(dllexport)
    #else
        #define XEUS_GAB_API __declspec(dllimport)
    #endif
#else
    #define XEUS_GAB_API
#endif

#endif