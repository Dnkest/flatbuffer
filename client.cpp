#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include "pt_generated.h"
#include "flatbuffers/reflection.h"
#include "flatbuffers/util.h"
#include "flatbuffers/verifier.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/reflection_generated.h"

using namespace property_tree;

void receive_property(int socket_fd, flatbuffers::FlatBufferBuilder &builder) {
    // Receive the size of the buffer first
    uint32_t size;
    recv(socket_fd, &size, sizeof(size), 0);

    // Receive the actual FlatBuffer data
    std::vector<uint8_t> buf(size);
    recv(socket_fd, buf.data(), size, 0);

    // Deserialize the property object
    auto property = GetProperty(buf.data());

    // Access the value of the old property
    auto string_value = property->value_as_string_value()->value()->str();
    std::cout << "property value: " << string_value << std::endl;

    // Load the binary schema file
    std::string schemafile;
    bool loaded = flatbuffers::LoadFile("pt.bfbs", false, &schemafile);
    if (!loaded) {
        std::cerr << "Failed to load schema file." << std::endl;
        return;
    }

    // Parse the binary schema
    flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t *>(schemafile.data()), schemafile.size());
    if (!reflection::VerifySchemaBuffer(verifier)) {
        std::cerr << "Failed to verify schema buffer." << std::endl;
        return;
    }

    // Initialize the schema
    auto schema = reflection::GetSchema(schemafile.data());

    // Get the root table
    auto root_table = schema->root_table();

    auto fields = root_table->fields();
    auto &root = *flatbuffers::GetAnyRoot(buf.data());

    for (auto field : *fields) {
        auto field_name = field->name()->str();
        // TODO
    }

}

int main() {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(12345);

    connect(client_fd, (sockaddr*)&addr, sizeof(addr));

    flatbuffers::FlatBufferBuilder builder;
    receive_property(client_fd, builder);

    close(client_fd);
    return 0;
}
