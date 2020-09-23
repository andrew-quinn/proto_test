#include <memory>
#include <string_view>
#include <vector>

#include <fmt/format.h>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "test_spec.grpc.pb.h"

#include "name_num.h"

class TestSpecImpl final : public test_spec::TestSpec::Service {
    const std::vector<int32_t> baseValues{1, 2, 3, 4, 5};
    const std::vector<std::string_view> baseNames{"anne", "bob", "carrie", "dave", "edna"};

    grpc::Status TestVectorCall(grpc::ServerContext *context,
                          const test_spec::TestVectorRequest *request,
                          test_spec::TestVectorReply *reply) override {
        const int32_t multiplier = request->multiplier();

        setVectorReplyMessage(reply, multiplier);
        setVectorReplyNameNums(reply, multiplier);

        // TODO: make error case out of empty nameNums
        // TODO: make error case out of overflow
        return grpc::Status::OK;
    }

    grpc::Status TestMapCall(grpc::ServerContext *context,
                             const test_spec::TestMapRequest *request,
                             test_spec::TestMapReply *reply) override {
        const int32_t multiplier = request->multiplier();

        setMapReplyNameNums(reply, multiplier);

        return grpc::Status::OK;
    }

    [[nodiscard]] std::vector<int32_t> getValues(int32_t multiplier) const {
        std::vector<int32_t> values(baseValues.size());
        std::transform(baseValues.begin(), baseValues.end(), values.begin(),
                       [&](int32_t v) -> int32_t { return v * multiplier; }
        );
        return values;
    }

    void setVectorReplyMessage(test_spec::TestVectorReply *reply, int32_t multiplier) const {
        static constexpr std::string_view messageFormat = "Multiplying values by {}...";
        reply->set_message(fmt::format(messageFormat, multiplier));
    }

    void setVectorReplyNameNums(test_spec::TestVectorReply *reply, int32_t multiplier) const {
        const auto nameNums = makeNameNums(baseNames, getValues(multiplier));
        google::protobuf::RepeatedPtrField<test_spec::NameNum> *nns = reply->mutable_namenums();
        for (const auto &element : nameNums) {
            test_spec::NameNum nn;
            nn.set_name(element.name);
            nn.set_number(element.num);
            nns->Add(std::move(nn));
        }
    }

    void setMapReplyNameNums(test_spec::TestMapReply *reply, int32_t multiplier) const {
        const std::vector<int32_t> values = getValues(multiplier);
        const size_t N = std::min(baseNames.size(), values.size());
        google::protobuf::Map<std::string, int32_t> &nameNums = *(reply->mutable_namenums());
        for (size_t i = 0; i < N; ++i) {
            nameNums[std::string{baseNames[i]}] = values[i];
        }
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
