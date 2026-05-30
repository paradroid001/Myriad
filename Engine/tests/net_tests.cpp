#include <catch2/catch_test_macros.hpp>

#include "myriad.h"

#include "io/MyrLogging.h"
#include "io/net/MyrNet.h"

using namespace Myriad::Net;

class TestTransport : public ITransport
{
  protected:
    virtual void SetState(NetTransportState_t new_state)
    {
        // nothing
    }

  public:
    // Pull in the ITransport Constructor
    using ITransport::ITransport;

    virtual void Tick(double dt)
    {
        // nothing/
    }
    virtual int Connect(net_id_t local_endpoint_id, ITransportAddress *args)
    {
        return -1;
    }
    virtual int Listen(net_id_t local_endpoint_id, ITransportAddress *args)
    {
        return -1;
    }

    virtual int Send(net_id_t dest_id, char *data, uint16_t size) { return -1; }
    virtual int Receive(net_id_t src_id, char *data) { return -1; }
};

TEST_CASE("Null Net Test")
{
    Network net;
    net_id_t local_id = net.CreateLocalEndpoint();
    net_id_t remote_id = net.CreateRemoteEndpoint();
    REQUIRE(local_id != remote_id);
}
