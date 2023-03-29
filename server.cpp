#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include "pt_generated.h"

using namespace property_tree;

void send_property(int socket_fd, const flatbuffers::FlatBufferBuilder &builder) {
    // Send the size of the buffer first
    uint32_t size = static_cast<uint32_t>(builder.GetSize());
    send(socket_fd, &size, sizeof(size), 0);

    // Send the actual FlatBuffer data
    send(socket_fd, builder.GetBufferPointer(), builder.GetSize(), 0);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(12345);

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    for (;;) {
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_addr_len);

        flatbuffers::FlatBufferBuilder builder;
        // Create the property object
        auto name = builder.CreateString("example");
        auto string_value = CreateStringValueDirect(builder, "hello world");

        std::vector<flatbuffers::Offset<Property>> sub_properties_vector; // Create an empty vector for sub-properties
        auto sub_properties = builder.CreateVector(sub_properties_vector);

        auto property = CreateProperty(
            builder,
            name,
            Value(Value_string_value),
            string_value.Union(),
            PropertyType_string,
            sub_properties
        );
        builder.Finish(property);

        send_property(client_fd, builder);
        close(client_fd);
    }

    close(server_fd);
    return 0;
}