#include <array>
#include <memory>
#include <string>
#include <vector>

#include <fmt/format.h>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "test_spec.grpc.pb.h"

class TestSpecImpl final : public test_spec::TestSpec::Service {
    static constexpr std::array<std::int32_t, 5> baseValues{1, 2, 3, 4, 5};

    grpc::Status TestCall(grpc::ServerContext *context,
                          const test_spec::TestRequest *request,
                          test_spec::TestReply *reply) override {
        const int32_t multiplier = request->multiplier();
        reply->set_message(makeMessage(multiplier));
        std::vector<int32_t> values = getValues(multiplier);
        reply->mutable_numbers()->Add(values.begin(), values.end());

        // TODO: make error case out of overflow
        return grpc::Status::OK;
    }

    std::vector<int32_t> getValues(int32_t multiplier) const {
        std::vector<int32_t> values(baseValues.size());
        std::transform(baseValues.begin(), baseValues.end(), values.begin(),
                       [&](int32_t v) -> int32_t { return v * multiplier; }
        );
        return values;
    }

    std::string makeMessage(const int32_t multiplier) const {
        return fmt::format("Multiplying values by {}...", multiplier);
    }
};

void RunServer() {
    std::string server_address = "0.0.0.0:50051";
    TestSpecImpl service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    grpc::ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    fmt::print("Server listening on {}\n", server_address);

    server->Wait();
}

int main() {
    RunServer();

    return 0;
}
