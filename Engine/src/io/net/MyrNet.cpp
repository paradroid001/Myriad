#include <memory> //shared_ptr

#include "core/config.h" //MYR_ID_INVALID
#include "io/MyrLogging.h"
#include "io/net/MyrNet.h"

namespace Myriad::Net
{

    // Transport

    // Instead of a unique_ptr (which would make sense) we need to
    // use a shared ptr. This is because the single-ownership of a unique_ptr
    // is enforced through deletion of the copy constructor. The only way to
    // pass it around is with move semantics. We could do this in BindProtocol:
    // net_id_t ITransport::BindProtocol(std::unique_ptr<IProtocol> &&proto)
    // {
    //    ...
    //    net_id_t retval = protocols.Add(std::move(proto));
    // }
    // and this works.
    // BUT the container MyrIDMap has not been designed with move semantics
    // and thus can't store unique_ptrs.
    net_id_t ITransport::BindProtocol(std::shared_ptr<IProtocol> protocol)
    {
        net_id_t old_id = protocol->GetID();
        if (old_id != MYRIAD_INVALID_ID)
        {
            MYR_CORE_WARN(
                "Attempting to bind protocol with an existing id. Ignoring");
            return old_id;
        }
        // Assign the protocol ID the same as the transport ID.
        net_id_t retval = GetID();
        protocol->SetID(retval);
        return retval;
    }

    // Endpoint

    // Constructor should only be called by Network, i.e. AddEndpoint
    Endpoint::Endpoint(net_id_t id, std::string name, Network *network,
                       bool remote)
        : NetworkObject(id, name), network(network), is_remote(remote)
    {
        MYR_CORE_TRACE("Endpoint constructor");
    }

    Endpoint::~Endpoint() { MYR_CORE_TRACE("Endpoint {0} destructor", id); }

    net_id_t Endpoint::AddTransport(std::shared_ptr<ITransport> transport)
    {
        net_id_t retval = MYRIAD_INVALID_ID;

        if (nullptr != transport)
        {
            net_id_t old_id = transport->GetID();
            if (old_id != MYRIAD_INVALID_ID)
            {
                MYR_CORE_WARN("Attempt to add transport to protocol with "
                              "existing id. Ignoring");
            }
            else
            {
                retval = transports.Add(transport);
                transport->SetID(retval);
            }
        }
        return retval;
    }

    std::shared_ptr<ITransport> Endpoint::GetTransport(net_id_t transport_id)
    {
        for (auto it = transports.begin(); it != transports.end(); it++)
        {
            if (it->first == transport_id)
                return it->second;
        }
        return nullptr;
    }

    // Connection

    Connection::Connection(net_id_t id, std::shared_ptr<Endpoint> ep1,
                           net_id_t transport1, std::shared_ptr<Endpoint> ep2,
                           net_id_t transport2)
        : NetworkObject(id, "Unknown Connection"), endpoint1(ep1),
          transport1(transport1), endpoint2(ep2), transport2(transport2)
    {
        MYR_CORE_TRACE("Connection {0} constructor", GetID());
    }

    Connection::~Connection()
    {
        MYR_CORE_TRACE("Connection {0} destructor", GetID());
    }

    void *Connection::GetOtherTransportAddress(net_id_t my_endpoint_id)
    {
        // TODO
        return nullptr;
    }

    // Network
    Network::Network()
    {
        event_service = std::make_shared<MyrEventService>();
        time = 0;
        MYR_CORE_TRACE("Network Constructor");
    }
    Network::~Network() { MYR_CORE_TRACE("Network Destructor"); }

    void Network::Tick(double dt)
    {
        time += dt;
        event_service->ProcessEvents();
        event_service->ClearEvents(); // todo - memleak.?
    }

    // Creates an endpoint, assigns a local id
    net_id_t Network::CreateEndpoint(bool remote)
    {
        std::shared_ptr<Endpoint> endpoint = std::make_shared<Endpoint>(
            MYRIAD_INVALID_ID, "Unknown Local", this, remote);
        net_id_t new_id = endpoints.Add(endpoint);
        endpoint->SetID(new_id);
        return new_id;
    }

    net_id_t Network::CreateLocalEndpoint() { return CreateEndpoint(false); }

    // Creates an remote endpoint
    // TODO really feels like you should have a connection by this point.
    net_id_t Network::CreateRemoteEndpoint() { return CreateEndpoint(true); }

    Endpoint *Network::GetEndpointForID(net_id_t id)
    {
        return endpoints.Get(id)->get(); // return the raw ptr
    }

    Connection *Network::GetConnectionForEndpointID(net_id_t id)
    {
        return nullptr;
    }

} // namespace Myriad::Net
