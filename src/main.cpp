#include <algorithm>
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
    bool        gui         = false;

    CLI::Option *model_opt   = app.add_option("-m,--model,model", modelfile, "3D model to load");
    CLI::Option *outfile_opt = app.add_option("-o,--outfile,outfile", outfile, "Path to output file");

    CLI11_PARSE(app, argc, argv);


    if (model_opt->empty()) {
        spdlog::error("no input file given");
        return 1;
    }

    spdlog::info("Input model: {}", modelfile);

    Assimp::Importer importer;
    unsigned int     pFlags = aiProcess_GenNormals | aiProcess_Triangulate; // | aiProcess_CalcTangentSpace;

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

        spdlog::info("{} meshes", scene->mNumMeshes);
        for (unsigned int imesh = 0; imesh < scene->mNumMeshes; ++imesh) {

            compound nbt_mesh{};

            const auto          *mesh = scene->mMeshes[imesh];
            std::vector<uint8_t> vertices{};
            std::vector<uint8_t> normals{};
            spdlog::info("mesh {}: {} vertices", imesh, mesh->mNumVertices);
            vertices.reserve(static_cast<size_t>(mesh->mNumVertices) * 3 * sizeof(float));

            for (unsigned int ivertex = 0; ivertex < mesh->mNumVertices; ++ivertex) {
                push_float(vertices, mesh->mVertices[ivertex].x);
                push_float(vertices, mesh->mVertices[ivertex].y);
                push_float(vertices, mesh->mVertices[ivertex].z);

                push_float(normals, mesh->mNormals[ivertex].x);
                push_float(normals, mesh->mNormals[ivertex].y);
                push_float(normals, mesh->mNormals[ivertex].z);
            }
            nbt_mesh.insert_node(vertices, "vertices");
            nbt_mesh.insert_node(normals, "normals");
            root.insert_node(nbt_mesh, std::format("mesh {}", imesh));
        }

        nbt::write_to_file(std::move(root), outfile);
    }
}
