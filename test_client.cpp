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

    std::string constructMessage(const test_spec::TestReply &reply) {
        const std::string msg = reply.message();
        const google::protobuf::RepeatedPtrField<test_spec::NameNum> &nns = reply.namenums();
        std::vector<NameNum> nameNums;
        nameNums.reserve(nns.size());
        for (auto &element : nns) {
            nameNums.emplace_back(std::string{element.name()}, element.number());
        }
        return fmt::format("{}\n{}\n", msg, fmt::join(nameNums.begin(), nameNums.end(), "\n"));
    }

    std::string test(int32_t multiplier) {
        test_spec::TestRequest request;
        request.set_multiplier(multiplier);

        test_spec::TestReply reply;
        grpc::ClientContext context;

        grpc::Status status = stub->TestCall(&context, request, &reply);

        if (status.ok()) {
            return constructMessage(reply);
        } else {
            fmt::print("{}: {}\n", status.error_code(), status.error_message());
            return "RPC failed\n";
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
    std::string response = client.test(factor);

    fmt::print(response);
    return 0;
}
