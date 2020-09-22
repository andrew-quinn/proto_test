#ifndef PROTO_TEST_NAME_NUM_H
#define PROTO_TEST_NAME_NUM_H

#include <string>
#include <vector>

#include <fmt/format.h>

struct NameNum {
    std::string name;
    int32_t num;

    NameNum(std::string &&name, int32_t num) : name(name), num(num) {}
};

std::vector<NameNum> makeNameNums(const std::vector<std::string_view> &names, const std::vector<int32_t> &nums) {
    size_t N = std::min(names.size(), nums.size());
    std::vector<NameNum> nameNums;
    nameNums.reserve(N);
    for (size_t i = 0; i < N; ++i) {
        nameNums.emplace_back(std::string{names[i]}, nums[i]);
    }
    return nameNums;
}

template<>
struct fmt::formatter<NameNum> {
    // leave the format invariant
    [[maybe_unused]] constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    [[maybe_unused]] auto format(const NameNum &n, FormatContext &ctx) {
        return format_to(
                ctx.out(),
                "[name: {}, num: {}]",
                n.name,
                n.num
        );
    }
};

#endif //PROTO_TEST_NAME_NUM_H
