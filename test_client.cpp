#include <memory>
#include <string>

#include <fmt/format.h>

#include <grpcpp/grpcpp.h>

#include "test_spec.grpc.pb.h"

#include "name_num.h"

class TestClient {
public:
    explicit TestClient(std::shared_ptr<grpc::Channel> channel) :
            stub(test_spec::TestSpec::NewStub(channel)) {}

    std::string constructMessage(const test_spec::TestVectorReply &reply) {
        const std::string msg = reply.message();
        const google::protobuf::RepeatedPtrField<test_spec::NameNum> &nns = reply.namenums();
        std::vector<NameNum> nameNums;
        nameNums.reserve(nns.size());
        for (auto &element : nns) {
            nameNums.emplace_back(std::string{element.name()}, element.number());
        }
        return fmt::format("{}\n{}\n", msg, fmt::join(nameNums.begin(), nameNums.end(), "\n"));
    }

    std::string constructMessage(const test_spec::TestMapReply &reply) {
        const google::protobuf::Map<std::string, int32_t> &nns = reply.namenums();
        const std::vector<std::string> names = { "edna", "fred" };
        static constexpr std::string_view retFormat = "[{}, {}]\n";
        std::string ret;
        for (const auto& name : names) {
            if (nns.contains(name)) {
                ret += fmt::format(retFormat, name, nns.at(name));
            } else {
                ret += fmt::format(retFormat, name, "NOT FOUND");
            }
        }
        return ret;
    }

    std::string testVectors(int32_t multiplier) {
        test_spec::TestVectorRequest request;
        request.set_multiplier(multiplier);

        test_spec::TestVectorReply reply;
        grpc::ClientContext context;

        grpc::Status status = stub->TestVectorCall(&context, request, &reply);

        if (status.ok()) {
            return constructMessage(reply);
        } else {
            fmt::print("{}: {}\n", status.error_code(), status.error_message());
            return "RPC failed\n";
        }
    }

    std::string testMaps(int32_t multiplier) {
        test_spec::TestMapRequest request;
        request.set_multiplier(multiplier);

        test_spec::TestMapReply reply;
        grpc::ClientContext context;

        grpc::Status status = stub->TestMapCall(&context, request, &reply);

        if (status.ok()) {
            return constructMessage(reply);
        } else {
            fmt::print("{}: {}\n", status.error_code(), status.error_message());
            return "RPC failed";
        }
    }

private:
    std::unique_ptr<test_spec::TestSpec::Stub> stub;
};

int main() {
    std::string target = "localhost:50051";
    TestClient client(grpc::CreateChannel(
            target, grpc::InsecureChannelCredentials()
    ));
    int32_t factor = 3;

    std::string vectorTestResponse = client.testVectors(factor);
    fmt::print(vectorTestResponse);

    std::string mapTestResponse = client.testMaps(factor);
    fmt::print(mapTestResponse);

    return 0;
}
