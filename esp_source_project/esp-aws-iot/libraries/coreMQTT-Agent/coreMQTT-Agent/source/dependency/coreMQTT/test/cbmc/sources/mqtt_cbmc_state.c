/*
 * coreMQTT v1.2.0
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file mqtt_cbmc_state.c
 * @brief Implements the functions defined in mqtt_cbmc_state.h.
 */
#include <stdint.h>
#include <stdlib.h>
#include "core_mqtt.h"
#include "mqtt_cbmc_state.h"
#include "network_interface_stubs.h"
#include "get_time_stub.h"
#include "event_callback_stub.h"

/* An exclusive default bound on the subscription count. Iterating over possibly
 * SIZE_MAX number of subscriptions does not add any value to the proofs. An
 * application can allocate memory for as many subscriptions as their system can
 * handle. The proofs verify that the code can handle the maximum
 * topicFilterLength in each subscription. */
#ifndef SUBSCRIPTION_COUNT_MAX
    #define SUBSCRIPTION_COUNT_MAX    2U
#endif

/* An exclusive default bound on the remainingLength in an incoming packet. This
 * bound is used for the MQTT_DeserializeAck() proof to limit the number of
 * iterations on a SUBACK packet's payload bytes. We do not need to iterate an
 * unbounded remaining length amount of bytes to verify memory safety in the
 * dereferencing the SUBACK payload's bytes. */
#ifndef REMAINING_LENGTH_MAX
    #define REMAINING_LENGTH_MAX    CBMC_MAX_OBJECT_SIZE
#endif

MQTTPacketInfo_t * allocateMqttPacketInfo( MQTTPacketInfo_t * pPacketInfo )
{
    if( pPacketInfo == NULL )
    {
        pPacketInfo = malloc( sizeof( MQTTPacketInfo_t ) );
    }

    if( pPacketInfo != NULL )
    {
        pPacketInfo->pRemainingData = malloc( pPacketInfo->remainingLength );
    }

    return pPacketInfo;
}

bool isValidMqttPacketInfo( const MQTTPacketInfo_t * pPacketInfo )
{
    bool isValid = true;

    if( pPacketInfo != NULL )
    {
        isValid = isValid && pPacketInfo->remainingLength < REMAINING_LENGTH_MAX;
    }

    return isValid;
}

MQTTPublishInfo_t * allocateMqttPublishInfo( MQTTPublishInfo_t * pPublishInfo )
{
    if( pPublishInfo == NULL )
    {
        pPublishInfo = malloc( sizeof( MQTTPublishInfo_t ) );
    }

    if( pPublishInfo != NULL )
    {
        pPublishInfo->pTopicName = malloc( pPublishInfo->topicNameLength );
        pPublishInfo->pPayload = malloc( pPublishInfo->payloadLength );
    }

    return pPublishInfo;
}

bool isValidMqttPublishInfo( const MQTTPublishInfo_t * pPublishInfo )
{
    bool isValid = true;

    return isValid;
}

MQTTConnectInfo_t * allocateMqttConnectInfo( MQTTConnectInfo_t * pConnectInfo )
{
    if( pConnectInfo == NULL )
    {
        pConnectInfo = malloc( sizeof( MQTTConnectInfo_t ) );
    }

    if( pConnectInfo != NULL )
    {
        pConnectInfo->pClientIdentifier = malloc( pConnectInfo->clientIdentifierLength );
        pConnectInfo->pUserName = malloc( pConnectInfo->userNameLength );
        pConnectInfo->pPassword = malloc( pConnectInfo->passwordLength );
    }

    return pConnectInfo;
}

bool isValidMqttConnectInfo( const MQTTConnectInfo_t * pConnectInfo )
{
    bool isValid = true;

    return isValid;
}

MQTTFixedBuffer_t * allocateMqttFixedBuffer( MQTTFixedBuffer_t * pFixedBuffer )
{
    if( pFixedBuffer == NULL )
    {
        pFixedBuffer = malloc( sizeof( MQTTFixedBuffer_t ) );
    }

    if( pFixedBuffer != NULL )
    {
        pFixedBuffer->pBuffer = malloc( pFixedBuffer->size );
    }

    return pFixedBuffer;
}

bool isValidMqttFixedBuffer( const MQTTFixedBuffer_t * pFixedBuffer )
{
    bool isValid = true;

    return isValid;
}

MQTTSubscribeInfo_t * allocateMqttSubscriptionList( MQTTSubscribeInfo_t * pSubscriptionList,
                                                    size_t subscriptionCount )
{
    if( pSubscriptionList == NULL )
    {
        pSubscriptionList = malloc( sizeof( MQTTSubscribeInfo_t ) * subscriptionCount );
    }

    if( pSubscriptionList != NULL )
    {
        for( int i = 0; i < subscriptionCount; i++ )
        {
            pSubscriptionList[ i ].pTopicFilter = malloc( pSubscriptionList[ i ].topicFilterLength );
        }
    }

    return pSubscriptionList;
}

bool isValidMqttSubscriptionList( MQTTSubscribeInfo_t * pSubscriptionList,
                                  size_t subscriptionCount )
{
    bool isValid = true;

    return isValid;
}

MQTTContext_t * allocateMqttContext( MQTTContext_t * pContext )
{
    TransportInterface_t * pTransportInterface;
    MQTTFixedBuffer_t * pNetworkBuffer;
    MQTTStatus_t status = MQTTSuccess;

    if( pContext == NULL )
    {
        pContext = malloc( sizeof( MQTTContext_t ) );
    }

    pTransportInterface = malloc( sizeof( TransportInterface_t ) );

    if( pTransportInterface != NULL )
    {
        /* The possibility that recv and send callbacks are NULL is tested in the
         * MQTT_Init proof. MQTT_Init is required to be called before any other
         * function in core_mqtt.h. */
        pTransportInterface->recv = NetworkInterfaceReceiveStub;
        pTransportInterface->send = NetworkInterfaceSendStub;
    }

    pNetworkBuffer = allocateMqttFixedBuffer( NULL );

    /* It is part of the API contract to call MQTT_Init() with the MQTTContext_t
     * before any other function in core_mqtt.h. */
    if( pContext != NULL )
    {
        status = MQTT_Init( pContext,
                            pTransportInterface,
                            GetCurrentTimeStub,
                            EventCallbackStub,
                            pNetworkBuffer );
    }

    /* If the MQTTContext_t initialization failed, then set the context to NULL
     * so that function under harness will return immediately upon a NULL
     * parameter check. */
    if( status != MQTTSuccess )
    {
        pContext = NULL;
    }

    return pContext;
}

bool isValidMqttContext( const MQTTContext_t * pContext )
{
    bool isValid = true;

    if( pContext != NULL )
    {
        isValid = isValidMqttFixedBuffer( &( pContext->networkBuffer ) );
    }

    return isValid;
}
