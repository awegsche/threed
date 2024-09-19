#include <algorithm>
#include <assimp/vector3.h>
#include <format>
#include <vector>

#include <CLI/CLI.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <spdlog/spdlog.h>

#include <nbt.h>

using nbt::compound;

int main(int argc, char **argv)
{
    CLI::App    app("Three D");
    std::string outfile;
    std::string modelfile;
    bool        swap_yz = false;

    CLI::Option *model_opt   = app.add_option("-m,--model,model", modelfile, "3D model to load");
    CLI::Option *outfile_opt = app.add_option("-o,--outfile,outfile", outfile, "Path to output file");
    CLI::Option *swap_yz_opt = app.add_flag("--swap_yz", swap_yz, "Swaps Y and Z coordinates");

    CLI11_PARSE(app, argc, argv);


    if (model_opt->empty()) {
        spdlog::error("no input file given");
        return 1;
    }

    spdlog::info("Input model: {}", modelfile);

    Assimp::Importer importer;
    // unsigned int     pFlags = aiProcess_GenNormals | aiProcess_Triangulate; // | aiProcess_CalcTangentSpace;
    unsigned int pFlags = 0;

    const aiScene *scene = importer.ReadFile(modelfile.c_str(), pFlags);

    if (scene == nullptr) {
        spdlog::error("scene couldn't be loaded");
        return 1;
    }

    spdlog::info("{} meshes", scene->mNumMeshes);
    for (unsigned int imesh = 0; imesh < scene->mNumMeshes; ++imesh) {

        const auto *mesh = scene->mMeshes[imesh];
        spdlog::info("mesh {}:", imesh);
        spdlog::info("{} vertices", imesh, mesh->mNumVertices);
        if (mesh->HasNormals()) { spdlog::info("has normals"); }
    }


    if (!outfile_opt->empty()) {
        spdlog::info("output filename given, going to write nbt outpu");
        // nbt::nbt_node node{std::move(root)};
        // nbt::write_to_file(node, outfile);

        compound root{};

        const auto push_float = [](std::vector<uint8_t> &vec, float f) {
            uint32_t i = *reinterpret_cast<uint32_t *>(&f);
            vec.push_back(i & 0xFF);
            vec.push_back(i >> 8 & 0xFF);
            vec.push_back(i >> 16 & 0xFF);
            vec.push_back(i >> 24 & 0xFF);
        };

        const auto push_float3 = [push_float](std::vector<uint8_t> &vec, aiVector3D const& v, bool swap_yz_coords = false) {
            push_float(vec, v.x);
            if (swap_yz_coords) {
                push_float(vec, v.z);
                push_float(vec, v.y);
            } else {
                push_float(vec, v.y);
                push_float(vec, v.z);
            }
        };

        spdlog::info("{} meshes", scene->mNumMeshes);
        for (unsigned int imesh = 0; imesh < scene->mNumMeshes; ++imesh) {

            compound nbt_mesh{};

            const auto          *mesh = scene->mMeshes[imesh];
            std::vector<uint8_t> vertices{};
            std::vector<uint8_t> normals{};
            spdlog::info("mesh {}: {} vertices", imesh, mesh->mNumVertices);
            vertices.reserve(static_cast<size_t>(mesh->mNumVertices) * 3 * sizeof(float));

            for (unsigned int ivertex = 0; ivertex < mesh->mNumVertices; ++ivertex) {
                push_float3(vertices, mesh->mVertices[ivertex], swap_yz);
                push_float3(normals, mesh->mNormals[ivertex], swap_yz);
            }

            nbt_mesh.insert_node(vertices, "vertices");
            nbt_mesh.insert_node(normals, "normals");
            root.insert_node(nbt_mesh, std::format("mesh {}", imesh));
        }

        nbt::write_to_file(std::move(root), outfile);
    }
}
