#ifndef _MYRIAD_NET_H_
#define _MYRIAD_NET_H_
#include <cstdint>
#include <list>
#include <memory> //shared_ptr

#include "core/MyrEvent.h"
#include "core/core.h"
#include "io/MyrLogging.h"
#include "util/MyrContainers.h" //MyrIDMap

using namespace Myriad;

namespace Myriad::Net
{
// 0 is the invalid net id, it means 'unassigned'
#define NET_ID_INVALID 0

    typedef enum NetTransportState
    {
        STATE_TRANSPORT_CLOSED = 20,
        STATE_TRANSPORT_LISTENING = 21,
        STATE_TRANSPORT_ESTABLISHED = 22,
        STATE_TRANSPORT_CLOSING = 23
    } NetTransportState_t;

    typedef enum NetConnectionState
    {
        STATE_CONNECTION_CLOSED = 30,
        STATE_CONNECTION_ESTABLISHED = 31,
        STATE_CONNECTION_CLOSING = 32
    } NetConnectionState_t;

    // This needs to fit in 8 bits, see 'config.h'
    // MyrEvent defines the type and subtype together as a uint16_t
    typedef enum MyrNetEventSubtypes
    {
        NET_EVENT_TRANSPORT_STATE_CHANGE = 1,
        NET_EVENT_CONNECTION_STATE_CHANGE = 2
    } MyrNetEventSubtypes_t;

    // representing net_id as uint32
    typedef uint32_t net_id_t;

    //'Network' level Events

    class NetEvent : public MyrEvent
    {
      public:
        NetEvent(MyrNetEventSubtypes_t subtype)
            : MyrEvent(MYR_EVENT_NET, subtype)
        {
        }
        virtual ~NetEvent()
        {
            MYR_TRACE(
                "Destructor for NetEvent {0}:{1}. Pay attention: it is good if "
                "this is being called",
                GetType(), GetSubType());
        }
        net_id_t endpoint_id; // the originating endpoint.
    };

    class TransportStateChanged : public NetEvent
    {
      public:
        TransportStateChanged() : NetEvent(NET_EVENT_TRANSPORT_STATE_CHANGE) {}
        net_id_t transport_id;
        NetTransportState_t newstate;
    };

    class ConnectionStateChanged : public NetEvent
    {
      public:
        ConnectionStateChanged() : NetEvent(NET_EVENT_CONNECTION_STATE_CHANGE)
        {
        }
        net_id_t transport_id;
        NetConnectionState_t newstate;
    };

    class MYR_API Message
    {
      private:
        uint32_t msg_id;
        uint32_t conversation_id;
        uint16_t length;
        // Control flags.
        bool requires_ack;
        bool is_ack;
        bool result;
        bool is_last_message;
    };

    // fwd declare Network so Endpoint can have a pointer
    class Network;
    class Connection;
    class Endpoint;
    class IProtocol;

    class NetworkObject
    {
      protected:
        net_id_t id; // locally assigned id in this network.
        std::string name;

      public:
        NetworkObject(net_id_t id, std::string name) : id(id), name(name) {}
        virtual ~NetworkObject() {}

        inline net_id_t GetID() const { return id; }
        // Use with caution, this should only be done when the existing ID was
        // MYR_INVALID_ID
        inline void SetID(net_id_t newid) { id = newid; }
        inline std::string GetName() const { return name; }
    };

    // Represent a transport address however you like.
    // Transports should know how to consume these.
    class ITransportAddress
    {
      public:
        virtual ~ITransportAddress() {}
        virtual char *AsString() const = 0;
    };

    // Virtual class for transports to override
    class MYR_API ITransport : public NetworkObject
    {
      protected:
        // this is where we send our events
        std::shared_ptr<MyrEventService> events;
        // transport lives on an endpoint
        std::shared_ptr<Endpoint> endpoint;
        // transport can be bound to a connection.
        std::shared_ptr<Connection> connection;
        // transport can carry one prototol
        std::shared_ptr<IProtocol> protocol;

        NetTransportState_t state;
        // Should be a state machine for things that need
        // transitions within this class
        virtual void SetState(NetTransportState_t newstate) = 0;

      public:
        ITransport(net_id_t id, std::string name,
                   std::shared_ptr<Endpoint> endpoint,
                   std::shared_ptr<MyrEventService> event_service)
            : NetworkObject(id, name), events(event_service),
              endpoint(endpoint), state(STATE_TRANSPORT_CLOSED)
        {
        }
        virtual ~ITransport() {}
        // Gets the current state.
        inline const NetTransportState State() const { return state; }

        // binds a protocol and returns the id
        // You create it and pass me a unique ptr
        // I'll hold it, assign an id, and it will die when the transport dies.
        // Since the protocol is on the transport, we give it the same id as the
        // transport. NOTE: we have to use shared_ptr here. See .cpp for
        // explanation
        net_id_t BindProtocol(std::shared_ptr<IProtocol> protocol);

        // The update loop for this: call it with how much time elapsed.
        virtual void Tick(double dt) = 0; // TODO we may not need this?
        // Implemented inside the transport will be how to
        // address the endpoint and serialise/deserialise the message
        virtual int Connect(net_id_t local_endpoint_id,
                            ITransportAddress *args) = 0;
        virtual int Listen(net_id_t local_endpoint_id,
                           ITransportAddress *args) = 0; // flags?

        virtual int Send(net_id_t dest_id, char *data, uint16_t size) = 0;
        virtual int Receive(net_id_t src_id, char *data) = 0;
    };

    // A protocol is bound to one transport.
    class IProtocol : public NetworkObject
    {
      protected:
        std::shared_ptr<ITransport> transport; // the transport we are bound to.

      public:
        IProtocol(net_id_t id, std::string name,
                  std::shared_ptr<ITransport> transport)
            : NetworkObject(id, name), transport(transport)
        {
        }
        virtual ~IProtocol() {};

        virtual net_id_t SetTransport(std::shared_ptr<ITransport> t)
        {
            if (transport != nullptr)
            {
                MYR_CORE_WARN("Protocol {0}:{1} already had a transport "
                              "assigned ({2}:{3}). Ignoring.",
                              id, name, transport->GetID(),
                              transport->GetName());
                return NET_ID_INVALID;
            }
            else
            {
                transport = t;
            }
            // Either way return the id of the currently assigned transport
            return transport->GetID();
        }

        virtual int ProcessData(char *data) = 0;
    };

    /**
     * @brief Represents a network node as opposed to an edge (connection)
     * @details An endpoint can
     *          : only belong to one network.
     *          : have multiple physical connections
     *          : be part of multiple logical connection groups
     */
    class MYR_API Endpoint : public NetworkObject
    {
      private:
        // This should really be a shared pointer, but it's hard to get the
        // network to store
        //  a shared pointer to itself unless it is originally created as a
        //  shared ptr. Lifetime of the network should be longer than the
        //  endpoint,
        // so we should be fine.
        Network *network;

        // Endpoint can have multiple transports (which have
        //  their own protocol(s)).
        // Endpoints manages and own the ids of transports,
        // but transports are created outside this class.
        // you would pass them in with id == MYR_INVALID_ID
        MyrIDMap<std::shared_ptr<ITransport>> transports;

        // if true this is a representation of a remote endpoint,
        // not a local one
        //(so it doesn't make sense to ask it to connect, listen, etc.)
        bool is_remote;

        // serialiser?
        // time?
        // events?
      public:
        Endpoint(net_id_t id, std::string name, Network *network, bool remote);
        virtual ~Endpoint();

        // Add a transport to the endpoint, transport gets a new id.
        net_id_t virtual AddTransport(std::shared_ptr<ITransport> transport);
        // Get a transport from an endpoint
        std::shared_ptr<ITransport> virtual GetTransport(net_id_t transport_id);
    };

    /**
     * @brief Represents a network edge as opposed to a node (endpoint)
     * @note this class should be abstract, which means
     *       Network::Connect will take <Connection, Transport>
     *
     * A connection is actually just two sets of endpoints which have
     * one or more established transports. For a point to point connection,
     * it's just a source and a destination.
     */
    class MYR_API Connection : public NetworkObject
    {
      protected:
        std::shared_ptr<Endpoint> endpoint1;
        net_id_t transport1;
        std::shared_ptr<Endpoint> endpoint2;
        net_id_t transport2;
        NetConnectionState_t state;

      public:
        Connection(net_id_t id, std::shared_ptr<Endpoint> ep1,
                   net_id_t transport_ep1, std::shared_ptr<Endpoint> ep2,
                   net_id_t transport_ep2);
        virtual ~Connection();

        // virtual int SendMessage() { return -1; }
        // virtual int RecvMessage() { return -1; }

        bool IsEstablished() { return state == STATE_CONNECTION_ESTABLISHED; }
        // Get transport details for the other endpoint in the connection
        // What gets returned will be specific to the transport.
        virtual void *GetOtherTransportAddress(net_id_t my_endpoint_id);
    };

    /**
     * @brief Represents and manages the set of endpoints and connections
     * between them.
     */
    class MYR_API Network
    {
      private:
        double time; // a representation of time.
        std::shared_ptr<MyrEventService> event_service;

        // All endpoints - we manage the ids for these.
        MyrIDMap<std::shared_ptr<Endpoint>> endpoints;
        // All connections - we create and manage these.
        MyrIDMap<std::shared_ptr<Connection>> connections;

        // Transports sit on endpoints, and protocols sit on transports
        // Endpoints manage the transports, transports manage the protocols
        std::list<std::shared_ptr<ITransport>> transports;

        net_id_t CreateEndpoint(bool remote);

      public:
        Network();
        virtual ~Network();
        virtual void Tick(double time); // caller calls this at tickrate.

        inline std::shared_ptr<MyrEventService> GetEventService()
        {
            // TODO: how many things are really calling this?
            //       perhaps this shouldn't be a shared ptr return?
            return event_service;
        }

        // Ask an endpoint to connect, given a transport type to
        // instance
        // TODO: it wouldn't make sense to ask an endpoint which
        // wasn't "you" to connect to something else. That's not
        // reflected in this model.
        template <typename T>
        int EndpointConnect(net_id_t local_endpoint_id, ITransportAddress *args)
        {
            // Get the endpoint
            std::shared_ptr<Endpoint> endpoint =
                *endpoints.Get(local_endpoint_id);
            if (endpoint == nullptr)
            {
                MYR_CORE_ERROR("Attempted to make transport for endpoint which "
                               "did not exist");
                return -2;
            }

            // Make a new transport, pass it our event service
            // This means an id, an endpoint, and an event service.
            T *t = new T(MYRIAD_INVALID_ID, endpoint, event_service);
            std::shared_ptr<ITransport> shared_t(t);
            // TODO - what if this fails / gives MYRIAD_INVALID_ID?
            net_id_t new_id = endpoint->AddTransport(shared_t);
            // Add it to the list, no error checking.
            transports.push_back(shared_t);

            // TODO: We don't want to do anything with connections until we get
            // the event that this has connected - we would handle that
            // elsewhere: the question is - at that time how will we know
            // which transport it was? Should the transport live on the
            // endpoint?

            // Connection *connection = new Connection(GetNextConnectionID());
            // connection->AttachTransport(transport);
            // pending_connections.push_back(connection);

            // If this connects or not it will put an
            // event back on the network with the local endpoint id.
            return shared_t->Connect(local_endpoint_id, args);
        }

        // Ask an endpoint to listen, given a transport type
        // endpoints can't listen multiple times simultaneously with the same
        // transport?
        // See above comment re: transports/connections and who owns them.
        template <typename T>
        int EndpointListen(net_id_t local_endpoint_id, ITransportAddress *args)
        {
            // Get the endpoint
            std::shared_ptr<Endpoint> endpoint =
                *endpoints.Get(local_endpoint_id);
            if (endpoint == nullptr)
            {
                MYR_CORE_ERROR("Attempted to make transport for endpoint which "
                               "did not exist");
                return -2;
            }
            // TODO - this is the second time we are creating
            //  a transport: feels like we need a createtransport method.
            T *t = new T(MYRIAD_INVALID_ID, "Unknown Transport", endpoint,
                         event_service);
            std::shared_ptr<ITransport> shared_t(t);
            // TODO - what if this fails / gives MYRIAD_INVALID_ID?
            net_id_t new_id = endpoint->AddTransport(shared_t);
            // Add it to the list, no error checking.
            transports.push_back(shared_t);
            return shared_t->Listen(local_endpoint_id, args);
        }

        // Return a raw pointer because the caller shouldn't be storing it,
        // they just use it.
        template <typename T>
        T *EndpointAddTransport(net_id_t endpoint_id,
                                std::string transport_name)
        {
            // Get the endpoint
            std::shared_ptr<Endpoint> *e = endpoints.Get(endpoint_id);
            if (e == nullptr)
            {
                MYR_CORE_ERROR(
                    "Attempt to add transport {0} to non-existant endpoint",
                    transport_name);
                return nullptr;
            }

            // Create the transport
            T *t = new T(MYRIAD_INVALID_ID, transport_name, *e, event_service);
            std::shared_ptr<ITransport> shared_t(t);

            // Add to endpoint
            // TODO - what if this fails / gives MYRIAD_INVALID_ID?
            (*e)->AddTransport(shared_t);
            // Add it to the list, no error checking!
            transports.push_back(shared_t);
            return t;
        }

        net_id_t CreateLocalEndpoint();
        net_id_t CreateRemoteEndpoint();
        Endpoint *GetEndpointForID(net_id_t id);
        // This one isn't going to work: endpoint can have multiple connections/
        Connection *GetConnectionForEndpointID(net_id_t id);
        // Probably methods to create subsets of endpoints in groups?
    };

} // namespace Myriad::Net

#endif
